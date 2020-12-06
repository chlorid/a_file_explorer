/*
* Copyright (C) 2008,2010,2012,2018,2019,2020 Klaus Michael Indlekofer. All rights reserved.
*
* m.indlekofer@gmx.de
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/



#include "commoninclude.h"
#include "akaiutil_io.h"
#include "akaiutil.h"
#include "akaiutil_file.h"
#include "akaiutil_take.h"
#include "akaiutil_tar.h"



u_int
tar_checksum(u_char *hp)
{
	u_int sum;
	u_int i;
	int zeroflag;

	if (hp==NULL){
		return 0;
	}

	sum=0;
	zeroflag=1; /* still all zeroes */
	for (i=0;i<sizeof(struct tar_head_s);i++){
		if (hp[i]!=0){
			zeroflag=0;
		}
		if ((i>=148)&&(i<156)){	/* treat chksum field */
			sum+=(u_int)' '; /* space */
		}else{
			sum+=(u_int)hp[i];
		}
	}
	/* Note: sum is always >0 !!! */

	if (zeroflag){
		sum=0; /* unique value, see comment above */
	}
	return sum;
}

int
tar_export(int fd,struct part_s *pp,struct vol_s *vp,struct file_s *fp,u_int ti,u_int flags,int verbose,u_char *filtertagp)
{
	struct tar_head_s tarhd;
	u_int chksum;
	u_int size;
	u_int n;
	u_char buf[TAR_BLOCKSIZE];
	int wavret;
	char *wavname;
	int wavconvflag;

	/* DD take */
	u_int ddcstarts,ddcstarte;
	u_int ddcsizes,ddcsizee;
	struct akai_ddtake_s ddt;

	if (fd<0){
		return -1;
	}
	if ((flags&(TAR_EXPORT_PART|TAR_EXPORT_VOL|TAR_EXPORT_ANYFILE))==0){
		return -1;
	}

	ddcstarts=0;
	ddcstarte=0;
	ddcsizes=0;
	ddcsizee=0;

	wavconvflag=0;

	/* create tar header */
	bzero(&tarhd,sizeof(struct tar_head_s));
	
	/* name */
	/* XXX should always fit */
	if (flags&TAR_EXPORT_PART){
		/* partition */
		if (pp==NULL){
			return -1;
		}
		if (pp->type==PART_TYPE_DD){
			/* harddisk DD partition */
			if (pp->letter==0){
				sprintf(tarhd.name,"DD/");
			}else{
				sprintf(tarhd.name,"DD%u/",(u_int)pp->letter);
			}
		}else{
			/* sampler partition */
			sprintf(tarhd.name,"%c/",pp->letter);
		}
		size=0;
	}
	if (flags&TAR_EXPORT_VOL){
		/* sampler volume */
		if ((vp==NULL)||(vp->type==AKAI_VOL_TYPE_INACT)){
			return -1;
		}
		sprintf(tarhd.name+strlen(tarhd.name),"%s/",vp->name);
		size=0;
		if ((flags&TAR_EXPORT_ANYFILE)==0){ /* not a file? */
			/* XXX use devmajor field for type */
			sprintf(tarhd.devmajor,"%07o",vp->type);
			/* XXX use devminor field for load number */
			sprintf(tarhd.devminor,"%07o",vp->lnum);
#ifndef TAR_NOVOLPARAMTARHD
			if (vp->param!=NULL){
				/* XXX use linkname for volume parameters */
				tarhd.linkname[0]='\0'; /* XXX magic, also empty linkname string */
				tarhd.linkname[1]='P'; /* XXX magic */
				bcopy(vp->param,
					&tarhd.linkname[0]+2, /* XXX +2: linkname starts with magic */
					sizeof(struct akai_volparam_s)); /* XXX should fit */
			}
#endif
		}
	}
	if (flags&TAR_EXPORT_DDFILE){
		/* DD take */
		if (flags&TAR_EXPORT_WAV){
			wavret=akai_take2wav(pp,ti,fd,&size,&wavname,TAKE2WAV_CHECK);
			if (wavret<0){ /* error? */
#if 1
				return 0; /* skip this file w/o error */
#else
				return -1;
#endif
			}
			strcpy(tarhd.name+strlen(tarhd.name),wavname);
			wavconvflag=1;
		}
		if (!wavconvflag){
			char fnamebuf[AKAI_NAME_LEN+4+1]; /* name (ASCII), +4 for ".<type>", +1 for '\0' */

			/* index */
			if (ti>=AKAI_DDTAKE_MAXNUM){
				return 0; /* skip this file w/o error */
			}
			if (pp->head.dd.take[ti].stat==AKAI_DDTAKESTAT_FREE){ /* free? */
				return 0; /* skip this file w/o error */
			}

			/* clusters */
			ddcstarts=(pp->head.dd.take[ti].cstarts[1]<<8)
					  +pp->head.dd.take[ti].cstarts[0];
			ddcstarte=(pp->head.dd.take[ti].cstarte[1]<<8)
					  +pp->head.dd.take[ti].cstarte[0];

			/* determine csizes (sample clusters) and csizee (envelope clusters) */
			if (ddcstarts!=0){ /* sample not empty? */
				ddcsizes=akai_count_ddfatchain(pp,ddcstarts);
			}else{
				ddcsizes=0;
			}
			if (ddcstarte!=0){ /* envelope not empty? */
				ddcsizee=akai_count_ddfatchain(pp,ddcstarte);
			}else{
				ddcsizee=0;
			}

			/* name */
			akai2ascii_name(pp->head.dd.take[ti].name,fnamebuf,0); /* 0: not S900 */
			strcpy(fnamebuf+strlen(fnamebuf),AKAI_DDTAKE_FNAMEEND);
			strcpy(tarhd.name+strlen(tarhd.name),fnamebuf);

			/* get DD take header (DD directory entry) */
			bcopy(&pp->head.dd.take[ti],&ddt,sizeof(struct akai_ddtake_s));
			/* XXX use cstarts/cstarte fields in DD take header for csizes/csizee */
			ddt.cstarts[1]=0xff&(ddcsizes>>8);
			ddt.cstarts[0]=0xff&ddcsizes;
			ddt.cstarte[1]=0xff&(ddcsizee>>8);
			ddt.cstarte[0]=0xff&ddcsizee;
			/* size in bytes */
			size=sizeof(struct akai_ddtake_s)+(ddcsizes+ddcsizee)*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE;
		}
	}
	if (flags&TAR_EXPORT_FILE){
		/* sampler file */
		if ((fp==NULL)||(fp->type==AKAI_FTYPE_FREE)){
			return -1;
		}
		/* test if tag-filter matches */
		if (akai_match_filetags(filtertagp,fp->tag)<0){ /* no match? */
			return 0; /* skip this file w/o error */
		}
		if (flags&TAR_EXPORT_WAV){
			wavret=akai_sample2wav(fp,fd,&size,&wavname,SAMPLE2WAV_CHECK);
			if (wavret<0){ /* error? */
#if 1
				return 0; /* skip this file w/o error */
#else
				return -1;
#endif
			}
			if (wavret==0){
				/* file is a valid sample file */
				strcpy(tarhd.name+strlen(tarhd.name),wavname);
				wavconvflag=1;
			} /* else: file is not a sample file, export without WAV conversion */
		}
		if (!wavconvflag){
			strcpy(tarhd.name+strlen(tarhd.name),fp->name);
			size=fp->size;
		}
		/* XXX use devmajor field for osver */
		if (wavconvflag&&(fp->type==AKAI_SAMPLE900_FTYPE)){ /* WAV conversion of S900 sample file? */
			/* WAV is non-compressed, must use osver for S900 non-compressed sample format */
			sprintf(tarhd.devmajor,"%07o",AKAI_OSVER_S900VOL); /* XXX */
		}else{
			sprintf(tarhd.devmajor,"%07o",fp->osver); /* XXX */
		}
		if ((fp->volp!=NULL)
			&&((fp->volp->type==AKAI_VOL_TYPE_S3000)||(fp->volp->type==AKAI_VOL_TYPE_CD3000))){
			/* XXX use linkname for file tags */
			tarhd.linkname[0]='\0'; /* XXX magic, also empty linkname string */
			tarhd.linkname[1]='T'; /* XXX magic */
			bcopy(fp->tag,
				&tarhd.linkname[0]+2, /* XXX +2: linkname starts with magic */
				AKAI_FILE_TAGNUM); /* XXX should fit */
		}
	}
#ifndef TAR_NOTAGSFILE
#ifndef TAR_TAGSFILENAME
#define TAR_TAGSFILENAME	"TAGS.DAT"
#endif
	if (flags&TAR_EXPORT_TAGSFILE){
		/* tags-file */
		if ((pp==NULL)||(pp->type!=PART_TYPE_HD) /* not in S1000/S3000 harddisk partition on partition level? */
			||(strncmp(AKAI_PARTHEAD_TAGSMAGIC,(char *)pp->head.hd.tagsmagic,4)!=0)){
#if 1
			return 0; /* skip this file w/o error */
#else
			return -1;
#endif
		}
		strcpy(tarhd.name+strlen(tarhd.name),TAR_TAGSFILENAME);
		size=4+AKAI_PARTHEAD_TAGNUM*AKAI_NAME_LEN; /* tags magic and tag names */;
	}
#endif
#ifndef TAR_NOVOLPARAMFILE
#ifndef TAR_VOLPARAMFILENAME
#define TAR_VOLPARAMFILENAME	"VOLPARAM.DAT"
#endif
	if (flags&TAR_EXPORT_VOLPARAMFILE){
		/* volparam-file */
		if ((vp==NULL)||(vp->type==AKAI_VOL_TYPE_INACT)||(vp->param==NULL)){
#if 1
			return 0; /* skip this file w/o error */
#else
			return -1;
#endif
		}
		strcpy(tarhd.name+strlen(tarhd.name),TAR_VOLPARAMFILENAME);
		size=sizeof(struct akai_volparam_s);
	}
#endif

	if (verbose){
		PRINTF_OUT("%s\n",tarhd.name);
		FLUSH_ALL;
	}

	/* mode, uid, gid */
	if (flags&TAR_EXPORT_ANYFILE){ /* file? */
		sprintf(tarhd.mode,"%07o",0600); /* XXX -rw------- */
	}else{
		/* directory */
		sprintf(tarhd.mode,"%07o",0700); /* XXX -rwx------ */
	}
	sprintf(tarhd.uid,"%07o",0); /* XXX */
	sprintf(tarhd.gid,"%07o",0); /* XXX */

	/* size */
	sprintf(tarhd.size,"%011o",size);

	/* mtime */
	sprintf(tarhd.mtime,"%011o",(u_int)time(NULL)); /* XXX */

	/* space */
	tarhd.space=' ';

	/* type */
	if (flags&TAR_EXPORT_ANYFILE){ /* file? */
		tarhd.type=TAR_TYPE_REG;
	}else{
		/* directory */
		tarhd.type=TAR_TYPE_DIR;
	}

	/* magic */
	bcopy(TAR_USTAR,tarhd.ustar,8);

	/* user name */
#ifndef TAR_UNAME
#define TAR_UNAME	"AKAI"
#endif
	strcpy(tarhd.uname,TAR_UNAME);
	/* group name */
#ifndef TAR_GNAME
#define TAR_GNAME	"AKAI"
#endif
	strcpy(tarhd.gname,TAR_GNAME);
	
	/* checksum */
	chksum=tar_checksum((u_char *)&tarhd);
	sprintf(tarhd.chksum,"%06o",chksum);
	
	/* write tar header */
	if (WRITE(fd,(void *)&tarhd,sizeof(struct tar_head_s))!=sizeof(struct tar_head_s)){
		PRINTF_ERR("cannot write tar header\n");
		return -1;
	}

	if ((flags&TAR_EXPORT_ANYFILE)==0){ /* not a file? */
		return 0; /* done */
	}

	if (flags&TAR_EXPORT_DDFILE){
		/* export DD take */
		if (wavconvflag){
			if (akai_take2wav(pp,ti,fd,NULL,NULL,SAMPLE2WAV_EXPORT)<0){
				return -1;
			}
		}else{
			if (akai_export_take(fd,pp,&ddt,ddcsizes,ddcsizee,ddcstarts,ddcstarte)<0){
				PRINTF_ERR("cannot export take\n");
				return -1;
			}
		}
	}

	if (flags&TAR_EXPORT_FILE){
		/* export sampler file */
		if (wavconvflag){
			if (akai_sample2wav(fp,fd,NULL,NULL,SAMPLE2WAV_EXPORT)<0){
				return -1;
			}
		}else{
			if (akai_read_file(fd,NULL,fp,0,fp->size)<0){
				return -1;
			}
		}
	}

#ifndef TAR_NOTAGSFILE
	if (flags&TAR_EXPORT_TAGSFILE){
		/* export tags-file */
		if (WRITE(fd,(void *)pp->head.hd.tagsmagic,size)!=(int)size){
			PRINTF_ERR("cannot export tags-file\n");
			return -1;
		}
	}
#endif
#ifndef TAR_NOVOLPARAMFILE
	if (flags&TAR_EXPORT_VOLPARAMFILE){
		/* export volparam-file */
		if (WRITE(fd,(void *)vp->param,size)!=(int)size){
			PRINTF_ERR("cannot export volparam-file\n");
			return -1;
		}
	}
#endif

	/* fill rest of TAR_BLOCKSIZE bytes */
	n=size%TAR_BLOCKSIZE;
	n=(TAR_BLOCKSIZE-n)%TAR_BLOCKSIZE;
	if (n>0){
		bzero(buf,n);
		/* write */
		if (WRITE(fd,(void *)buf,n)!=(int)n){
			PRINTF_ERR("cannot write rest of block\n");
			return -1;
		}
	}

	return 0;
}

int
tar_export_vol(int fd,struct vol_s *vp,u_int flags,int verbose,u_char *filtertagp)
{
	u_int fi;
	struct file_s tmpfile; /* current file */

	if ((fd<0)||(vp==NULL)||(vp->type==AKAI_VOL_TYPE_INACT)||(vp->partp==NULL)||(!vp->partp->valid)){
		return -1;
	}

#ifndef TAR_NOVOLPARAMFILE
	if (vp->param!=NULL){
		/* export volparam-file */
		if (tar_export(fd,vp->partp,vp,NULL,0,flags|TAR_EXPORT_VOLPARAMFILE,verbose,NULL)){
			return -1;
		}
	}
#endif

	/* files in volume directory */
	for (fi=0;fi<vp->fimax;fi++){
		/* get file */
		if (akai_get_file(vp,&tmpfile,fi)<0){
			continue; /* XXX ignore error, next file */
		}
#if 1
		if (akai_check_fatblk(tmpfile.bstart,vp->partp->bsize,vp->partp->bsyssize)<0){ /* invalid? */
			if (verbose){
				PRINTF_OUT("file %u has invalid start block, skipping file\n",tmpfile.index+1);
				FLUSH_ALL;
			}
			continue; /* XXX ignore error, next file */
		}
#endif

		/* export [part/volume/]file */
		if (tar_export(fd,vp->partp,vp,&tmpfile,0,flags|TAR_EXPORT_FILE,verbose,filtertagp)){
			return -1;
		}
	}

	return 0;
}

int
tar_export_part(int fd,struct part_s *pp,u_int flags,int verbose,u_char *filtertagp)
{
	u_int vi;
	struct vol_s tmpvol; /* current volume */

	if ((fd<0)||(pp==NULL)||(!pp->valid)){
		return -1;
	}

	if (pp->type==PART_TYPE_DD){
		/* harddisk DD partition */
		/* export all DD takes */
		u_int ti;

		for (ti=0;ti<AKAI_DDTAKE_MAXNUM;ti++){
			if (pp->head.dd.take[ti].stat==AKAI_DDTAKESTAT_FREE){ /* free? */
				continue; /* next */
			}
			if (tar_export(fd,pp,NULL,NULL,ti,flags|TAR_EXPORT_DDFILE,verbose,filtertagp)){
				return -1;
			}
		}
	}

	/* now, sampler partition */

#ifndef TAR_NOTAGSFILE
	if (pp->type==PART_TYPE_HD){ /* in S1000/S3000 harddisk partition? */
		/* export tags-file */
		/* Note: tar_export() will check for valid tags magic */
		if (tar_export(fd,pp,NULL,NULL,0,flags|TAR_EXPORT_TAGSFILE,verbose,NULL)){
			return -1;
		}
	}
#endif

	/* volumes in root directory of partition */
	for (vi=0;vi<pp->volnummax;vi++){
		/* get volume */
		if (akai_get_vol(pp,&tmpvol,vi)<0){
			continue; /* XXX ignore error, next volume */
		}
		if ((tmpvol.type!=AKAI_VOL_TYPE_S900)
			&&(tmpvol.type!=AKAI_VOL_TYPE_S1000)
			&&(tmpvol.type!=AKAI_VOL_TYPE_S3000)
			&&(tmpvol.type!=AKAI_VOL_TYPE_CD3000)){
			continue; /* next volume */
		}
		/* create vol/ */
		if (tar_export(fd,pp,&tmpvol,NULL,0,flags|TAR_EXPORT_VOL,verbose,NULL)){
			return -1;
		}
		/* export volume */
		if (tar_export_vol(fd,&tmpvol,flags|TAR_EXPORT_VOL,verbose,filtertagp)<0){
			return -1;
		}
	}

	return 0;
}

int
tar_export_disk(int fd,struct disk_s *dp,u_int flags,int verbose,u_char *filtertagp)
{
	u_int pi;

	if ((fd<0)||(dp==NULL)){
		return -1;
	}

	/* partitions on disk */
	for (pi=0;pi<part_num;pi++){
		/* partition not on disk */
		if (part[pi].diskp!=dp){
			continue; /* next partition */
		}
		/* create part/ */
		if (tar_export(fd,&part[pi],NULL,NULL,0,flags|TAR_EXPORT_PART,verbose,NULL)){
			return -1;
		}
		/* export partition (sampler or DD) */
		if (tar_export_part(fd,&part[pi],flags|TAR_EXPORT_PART,verbose,filtertagp)<0){
			return -1;
		}
	}

	return 0;
}

int
tar_export_curdir(int fd,int verbose,u_int flags)
{

	if (curdiskp==NULL){ /* no disk? */
		PRINTF_ERR("must be on a disk\n");
		return -1;
	}
	if (curpartp==NULL){ /* no partition? */
		/* no, on disk */
		return tar_export_disk(fd,curdiskp,flags,verbose,curfiltertag);
	}
	if (curvolp==NULL){ /* no sampler volume? */
		/* now, in partition (sampler or DD) */
		return tar_export_part(fd,curpartp,flags,verbose,curfiltertag);
	}
	/* now, in sampler volume */
	return tar_export_vol(fd,curvolp,flags,verbose,curfiltertag);
}

int
tar_export_tailzero(int fd)
{
	u_char buf[TAR_TAILZERO_BLOCKS*TAR_BLOCKSIZE];

	/* zero blocks */
	bzero(buf,TAR_TAILZERO_BLOCKS*TAR_BLOCKSIZE);

	/* write blocks */
	if (WRITE(fd,buf,TAR_TAILZERO_BLOCKS*TAR_BLOCKSIZE)!=TAR_TAILZERO_BLOCKS*TAR_BLOCKSIZE){
		PRINTF_ERR("cannot write zero blocks\n");
		return -1;
	}

	return 0;
}



int
tar_import_curdir(int fd,u_int vtype0,int verbose,u_int flags)
{
	struct tar_head_s tarhd;
	u_int byteend,skip;
	u_int chksum,chksum0;
	u_int nlen;
	u_int l;
	int ret;
	struct vol_s tmpvol;
	struct file_s tmpfile;
	u_int vtype,lnum;
	u_int ftype;
	u_int osver;
	u_char *tagp;
	struct akai_volparam_s *parp;
	u_int wavbcount;
	int wavret;
	u_int ti;
	u_int csizes,csizee;
	u_int tsize;
	struct akai_ddtake_s t;

	if (curdiskp==NULL){ /* no disk? */
		PRINTF_ERR("must be on a disk\n");
		return -1;
	}

	save_curdir(1); /* 1: could be modifications */

	/* interpret tar file */
	skip=0;
	ret=-1; /* no success so far */
	for (;;){
		if (verbose){
			FLUSH_ALL;
		}
#ifdef DEBUG
		print_blk_cache();
#endif
		restore_curdir();

		/* skip if necessary */
		if (skip>0){
			/* skip file */
			if (LSEEK64(fd,(OFF64_T)skip,SEEK_CUR)<0){
				PERROR("lseek");
				ret=-1;
				goto tar_import_done;
			}
		}
		skip=0;

		/* read header */
		/* XXX size must not exceed SSIZE_MAX! */
		ret=(int)READ(fd,(void *)&tarhd,sizeof(struct tar_head_s));
		if (ret==0){
			/* end of file */
			ret=0; /* success */
			goto tar_import_done;
		}else if (ret!=sizeof(struct tar_head_s)){
			PRINTF_ERR("cannot read tar header\n");
			ret=-1;
			goto tar_import_done;
		}

		/* check header */
		chksum0=tar_checksum((u_char *)&tarhd);
		if (chksum0==0){ /* all zero block? */
			/* XXX ignore */
			continue; /* next */
		}
		/* checksum */
		if ((sscanf(tarhd.chksum,"%o",&chksum)!=1)||(chksum!=chksum0)){
			PRINTF_OUT("checksum error in next header\n");
			FLUSH_ALL;
			ret=-1;
			goto tar_import_done;
		}

		/* get size */
		byteend=0; /* default */
		sscanf(tarhd.size,"%o",&byteend);
		/* Note: if sscanf finds nothing, vtype remains! */
		/* skip rest of full TAR_BLOCKSIZE */
		skip=byteend%TAR_BLOCKSIZE;
		skip=(TAR_BLOCKSIZE-skip)%TAR_BLOCKSIZE;

		/* check type */
		if ((tarhd.type!=TAR_TYPE_DIR)
			&&(tarhd.type!=TAR_TYPE_REG)
			&&(tarhd.type!=TAR_TYPE_REG0)){
			skip+=byteend; /* skip file */
			continue; /* next */
		}
		/* now, directory or regular file */

		/* get name */
		if (verbose){
			PRINTF_OUT("%s\n",tarhd.name);
			FLUSH_ALL;
		}
		l=(u_int)strlen(tarhd.name);
		/* correct name if directory */
		if ((tarhd.type==TAR_TYPE_DIR)&&(l>0)&&(tarhd.name[l-1]=='/')){
			tarhd.name[l-1]='\0'; /* terminate */
			l--;
		}
		if (l==0){
			if (verbose){
				PRINTF_OUT("empty name, skipping file\n");
				FLUSH_ALL;
			}
			skip+=byteend; /* skip file */
			continue; /* next */
		}

		/* XXX ignore other tarhd elements */

		/* change to upper level directory */
		/* Note: will be undone upon exit or next for-loop iteration */
		save_curdir(1); /* 1: could be modifications */
		if (change_curdir(tarhd.name,0,dirnamebuf,0)<0){
			PRINTF_ERR("directory not found\n");
			ret=-1;
			goto tar_import_done;
		}

		/* parse type further */
		if (tarhd.type==TAR_TYPE_DIR){
			/* directory */

			/* check if on disk level */
			if (curpartp==NULL){
				/* on disk level */
				/* Note: no need to create partition, but must check if exists!!! */
				if (akai_find_part(curdiskp,dirnamebuf)==NULL){
					PRINTF_ERR("partition does not exist\n");
					ret=-1;
					goto tar_import_done;
				}	
			}else{
				if (curpartp->type==PART_TYPE_DD){
					/* harddisk DD partition */
					PRINTF_ERR("no directory allowed in DD-partition\n");
					ret=-1;
					goto tar_import_done;
				}
				/* now, sampler partition */
				if (curvolp!=NULL){ /* already in volume? */
					PRINTF_ERR("no directory allowed in volume\n");
					ret=-1;
					goto tar_import_done;
				}
				/* load number */
				lnum=AKAI_VOL_LNUM_OFF; /* default */
				/* XXX use devminor field for load number */
				sscanf(tarhd.devminor,"%o",&lnum);
				/* Note: if sscanf finds nothing, lnum remains! */
				/* correct lnum */
				if ((lnum!=AKAI_VOL_LNUM_OFF)
					&&((lnum<AKAI_VOL_LNUM_MIN)||(lnum>AKAI_VOL_LNUM_MAX))){
						lnum=AKAI_VOL_LNUM_OFF; /* XXX default */
				}
				/* volume parameters */
				parp=NULL; /* default volume parameters or keep old ones */
#ifndef TAR_NOVOLPARAMTARHD
				/* XXX use linkname for volume parameters */
				if ((tarhd.linkname[0]=='\0')&&(tarhd.linkname[1]=='P')){ /* XXX correct magic? */
					parp=(struct akai_volparam_s *)(&tarhd.linkname[0]+2); /* XXX +2: linkname starts with magic */
				}
#endif
				/* on partition level */
				/* check if destination volume already exists */
				if (akai_find_vol(curpartp,&tmpvol,dirnamebuf)<0){
					/* does not exist (or error in finding it, don't care) */
					/* user-supplied volume type, Note: will be corrected below if bad or inactive */
					vtype=vtype0;
					if (vtype0==AKAI_VOL_TYPE_INACT){ /* no user-supplied type? */
						/* XXX use devmajor field for type */
						sscanf(tarhd.devmajor,"%o",&vtype);
						/* Note: if sscanf finds nothing, vtype remains! */
						/* correct vtype */
						if (curpartp->type==PART_TYPE_HD9){
							vtype=AKAI_VOL_TYPE_S900;
						}else if ((vtype!=AKAI_VOL_TYPE_S1000)
							&&(vtype!=AKAI_VOL_TYPE_S3000)
							&&(vtype!=AKAI_VOL_TYPE_CD3000)){
								vtype=AKAI_VOL_TYPE_S1000; /* XXX default */
						}
					}

					/* create volume */
					if (akai_create_vol(curpartp,&tmpvol,vtype,AKAI_CREATE_VOL_NOINDEX,dirnamebuf,lnum,parp)<0){
						PRINTF_ERR("cannot create volume\n");
						ret=-1;
						goto tar_import_done;
					}
				}else{
					/* volume already exists */
					/* Note: keep volume type (even if user-supplied type) */
					/* XXX use devminor field for load number */
					if (tarhd.devminor[0]==0x00){ /* no load number? */
						lnum=tmpvol.lnum; /* keep old value */
					}
					osver=tmpvol.osver; /* keep old value */
					/* update load number and optional volume parameters */
					if (akai_rename_vol(&tmpvol,NULL,lnum,osver,parp)<0){
						PRINTF_ERR("cannot update volume\n");
						ret=-1;
						goto tar_import_done;
					}
				}
				if ((tmpvol.param!=NULL)&&(parp!=NULL)&&verbose){
					PRINTF_OUT("volume parameters imported\n");
					FLUSH_ALL;
				}
			}

			skip+=byteend; /* skip file */
			continue; /* next */
		}
		/* now, regular file */
		nlen=(u_int)strlen(dirnamebuf);

#ifndef TAR_NOTAGSFILE
		/* handle tags-file */
		if (strcmp(dirnamebuf,TAR_TAGSFILENAME)==0){
			if ((curpartp!=NULL)&&(curpartp->type==PART_TYPE_HD)&&(curvolp==NULL)){ /* in S1000/S3000 harddisk partition on partition level? */
				if (byteend==(4+AKAI_PARTHEAD_TAGNUM*AKAI_NAME_LEN)){ /* valid file size? (Note: tags magic and tag names) */
					/* read tags-file into partition header */
					if (READ(fd,(void *)curpartp->head.hd.tagsmagic,byteend)!=(int)byteend){
						PRINTF_ERR("cannot read tags-file\n");
						ret=-1;
						goto tar_import_done;
					}
					/* XXX no check if valid tags magic */
					/* write partition header */
					if (akai_io_blks(curpartp,(u_char *)&curpartp->head.hd,
									 0,
									 AKAI_PARTHEAD_BLKS,
									 1,IO_BLKS_WRITE)<0){ /* 1: allocate cache if possible */
						PRINTF_ERR("cannot write partition header\n");
						ret=-1;
						goto tar_import_done;
					}
					if (verbose){
						PRINTF_OUT("tags imported\n");
						FLUSH_ALL;
					}
				}else{
					if (verbose){
						PRINTF_OUT("invalid file size of tags-file, skipping file\n");
						FLUSH_ALL;
					}
					skip+=byteend; /* skip file */
					continue; /* next */
				}
			}else{
				if (verbose){
					PRINTF_OUT("tags-file must be on partition level of S1000/S3000 harddisk, skipping file\n");
					FLUSH_ALL;
				}
				skip+=byteend; /* skip file */
				continue; /* next */
			}
			continue; /* next */
		}
#endif
#ifndef TAR_NOVOLPARAMFILE
		/* handle volparam-file */
		if (strcmp(dirnamebuf,TAR_VOLPARAMFILENAME)==0){
			if ((curvolp!=NULL)&&(curvolp->param!=NULL)){ /* on volume level and volume has parameters? */
				if (byteend==sizeof(struct akai_volparam_s)){ /* valid file size? */
					/* read volparam-file into volume */
					if (READ(fd,(void *)curvolp->param,byteend)!=(int)byteend){
						PRINTF_ERR("cannot read volparam-file\n");
						ret=-1;
						goto tar_import_done;
					}
					/* update volume */
					if (akai_rename_vol(curvolp,NULL,curvolp->lnum,curvolp->osver,curvolp->param)<0){
						PRINTF_ERR("cannot update volume\n");
						ret=-1;
						goto tar_import_done;
					}
					if ((curvolp->param!=NULL)&&verbose){
						PRINTF_OUT("volume parameters imported\n");
						FLUSH_ALL;
					}
				}else{
					if (verbose){
						PRINTF_OUT("invalid file size of volparam-file, skipping file\n");
						FLUSH_ALL;
					}
					skip+=byteend; /* skip file */
					continue; /* next */
				}
			}else{
				if (verbose){
					if (curvolp==NULL){
						PRINTF_OUT("volparam-file must be on volume level, skipping file\n");
					}else{
						PRINTF_OUT("volume has no parameters, skipping file\n");
					}
					FLUSH_ALL;
				}
				skip+=byteend; /* skip file */
				continue; /* next */
			}
			continue; /* next */
		}
#endif
#ifndef TAR_NOVOLINFO1FILE
#ifndef TAR_VOLINFO1FILENAME
#define TAR_VOLINFO1FILENAME	"VOLINFO1.DAT"
#endif
#ifndef TAR_VOLINFO1FILE_SKIPB
#define TAR_VOLINFO1FILE_SKIPB	0x0010 /* number of bytes to skip in volinfo1-file */
#endif
		/* handle volinfo1-file */
		if (strcmp(dirnamebuf,TAR_VOLINFO1FILENAME)==0){
			if ((curvolp!=NULL)&&(curvolp->param!=NULL)){ /* on volume level and volume has parameters? */
				if (byteend==(TAR_VOLINFO1FILE_SKIPB+sizeof(struct akai_volparam_s))){ /* valid file size? */
					if (LSEEK64(fd,(OFF64_T)TAR_VOLINFO1FILE_SKIPB,SEEK_CUR)<0){
						PERROR("lseek");
						ret=-1;
						goto tar_import_done;
					}
					/* read volume parameters from volinfo1-file into volume */
					l=sizeof(struct akai_volparam_s);
					if (READ(fd,(void *)curvolp->param,l)!=(int)l){
						PRINTF_ERR("cannot read volinfo1-file\n");
						ret=-1;
						goto tar_import_done;
					}
					/* update volume */
					if (akai_rename_vol(curvolp,NULL,curvolp->lnum,curvolp->osver,curvolp->param)<0){
						PRINTF_ERR("cannot update volume\n");
						ret=-1;
						goto tar_import_done;
					}
					if (verbose){
						PRINTF_OUT("volume parameters imported\n");
						FLUSH_ALL;
					}
				}else{
					if (verbose){
						PRINTF_OUT("invalid file size of volinfo1-file, skipping file\n");
						FLUSH_ALL;
					}
					skip+=byteend; /* skip file */
					continue; /* next */
				}
			}else{
				if (verbose){
					if (curvolp==NULL){
						PRINTF_OUT("volinfo1-file must be on volume level, skipping file\n");
					}else{
						PRINTF_OUT("volume has no parameters, skipping file\n");
					}
					FLUSH_ALL;
				}
				skip+=byteend; /* skip file */
				continue; /* next */
			}
			continue; /* next */
		}
#endif
#ifndef TAR_NOSKIPDATFILE
		/* skip dat-files */
		if ((nlen>=4)&&(strncasecmp(dirnamebuf+nlen-4,".dat",4)==0)){
			if (verbose){
				PRINTF_OUT("skipping dat-file\n");
				FLUSH_ALL;
			}
			skip+=byteend; /* skip file */
			continue; /* next */
		}
#endif

		if (curpartp==NULL){
			PRINTF_ERR("no files allowed on disk level\n");
			ret=-1;
			goto tar_import_done;
		}

		if (curpartp->type==PART_TYPE_DD){
			/* harddisk DD partition */

			/* import DD take */

			if ((flags&TAR_IMPORT_WAV)&&(nlen>=4)&&(strncasecmp(dirnamebuf+nlen-4,".wav",4)==0)){
				/* convert WAV file into DD take */

				/* find free index */
				for (ti=0;ti<AKAI_DDTAKE_MAXNUM;ti++){
					/* take */
					if (curpartp->head.dd.take[ti].stat==AKAI_DDTAKESTAT_FREE){ /* free? */
						break; /* done */
					}
				}
				if (ti==AKAI_DDTAKE_MAXNUM){ /* no free index? */
					PRINTF_ERR("no free take index\n");
					ret=-1;
					goto tar_import_done;
				}

				/* Note: don't check if name already used, create new DD take */
				wavret=akai_wav2take(fd,dirnamebuf,
									 curpartp,
									 ti,
									 &wavbcount,
									 0);
				if ((wavret<0)||(byteend<wavbcount)){ /* error? */
					ret=-1;
					goto tar_import_done;
				}
				skip+=byteend-wavbcount; /* skip unread rest */
				continue; /* file done, next */
			}

			/* check file name */
			if ((nlen<3)||(strcmp(dirnamebuf+nlen-3,AKAI_DDTAKE_FNAMEEND)!=0)){
					PRINTF_ERR("invalid file name for DD take, skipping file\n");
					skip+=byteend; /* skip file */
					continue; /* next */
			}

			/* find free index */
			for (ti=0;ti<AKAI_DDTAKE_MAXNUM;ti++){
				/* take */
				if (curpartp->head.dd.take[ti].stat==AKAI_DDTAKESTAT_FREE){ /* free? */
					break; /* done */
				}
			}
			if (ti==AKAI_DDTAKE_MAXNUM){ /* no free index? */
				PRINTF_ERR("no free take index\n");
				ret=-1;
				goto tar_import_done;
			}

			/* get DD take header (DD directory entry) */
			if (READ(fd,&t,sizeof(struct akai_ddtake_s))!=(int)sizeof(struct akai_ddtake_s)){
				PRINTF_ERR("cannot read DD take header\n");
				ret=-1;
				goto tar_import_done;
			}

			/* get csizes (sample clusters) and csizee (envelope clusters) */
			/* XXX use cstarts/cstarte fields in DD take header for csizes/csizee */
			csizes=(t.cstarts[1]<<8)
				+t.cstarts[0];
			csizee=(t.cstarte[1]<<8)
				+t.cstarte[0];
			/* Note: akai_import_take() will increase csizee if required */
			/* XXX preliminary check of csizes+csizee */
			if ((csizes+csizee)*AKAI_DDPART_CBLKS>curpartp->bfree){
				PRINTF_ERR("not enough space left\n");
				ret=-1;
				goto tar_import_done;
			}
			/* size in bytes */
			tsize=sizeof(struct akai_ddtake_s)+(csizes+csizee)*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE;
			if (tsize>byteend){
				PRINTF_ERR("inconsistent size\n");
				ret=-1;
				goto tar_import_done;
			}

			/* import take */
			/* Note: akai_import_take() will increase csizee if required */
			/* Note: akai_import_take() keeps name in DD take header */
			/* Note: don't check if name already used, create new DD take */
			if (akai_import_take(fd,curpartp,&t,ti,csizes,csizee)<0){
				PRINTF_ERR("cannot import take\n");
				ret=-1;
				goto tar_import_done;
			}

			/* skip unread rest */
			skip+=byteend-tsize;
			continue; /* next */
		}

		/* now, sampler partition */

		if (curvolp==NULL){ /* not in volume? */
			PRINTF_ERR("files only allowed in volume\n");
			ret=-1;
			goto tar_import_done;
		}
#if 0
		skip+=byteend; /* skip file */
		continue; /* next */
#endif

		/* import file */

		/* default osver */
		osver=curvolp->osver; /* default: from volume */
		if (vtype0==AKAI_VOL_TYPE_INACT){ /* no user-supplied type? */
			/* XXX use devmajor field for osver */
			sscanf(tarhd.devmajor,"%o",&osver);
		}else if (vtype0==AKAI_VOL_TYPE_S900){
			osver=0; /* non-compressed, akai_create_file() will correct osver if necessary */
		}else if ((vtype0==AKAI_VOL_TYPE_S3000)||(vtype0==AKAI_VOL_TYPE_CD3000)){
			osver=AKAI_OSVER_S3000MAX; /* XXX */
		}else{
			osver=AKAI_OSVER_S1000MAX; /* XXX */
		}
		/* Note: if sscanf finds nothing, osver remains! */
		/* default tags */
		tagp=NULL;
		if ((curvolp->type==AKAI_VOL_TYPE_S3000)||(curvolp->type==AKAI_VOL_TYPE_CD3000)){
			/* XXX use linkname for file tags */
			if ((tarhd.linkname[0]=='\0')&&(tarhd.linkname[1]=='T')){ /* XXX correct magic? */
				tagp=(u_char *)&tarhd.linkname[2];/* XXX +2: linkname starts with magic */
				/* XXX no check for valid tag entries */
				akai_sort_filetags(tagp);
			}
		}

		if ((flags&TAR_IMPORT_WAV)&&(nlen>=4)&&(strncasecmp(dirnamebuf+nlen-4,".wav",4)==0)){
			/* convert WAV file into sample file */

			/* sample file type */
			if (flags&(TAR_IMPORT_WAVS9|TAR_IMPORT_WAVS9C)){
				/* S900 sample */
				ftype=AKAI_SAMPLE900_FTYPE;
			}else if (flags&TAR_IMPORT_WAVS1){
				/* S1000 sample */
				ftype=AKAI_SAMPLE1000_FTYPE;
			}else if (flags&TAR_IMPORT_WAVS3){
				/* S3000 sample */
				ftype=AKAI_SAMPLE3000_FTYPE;
			}else{
				ftype=AKAI_FTYPE_FREE; /* invalid: derive sample file type from volume type/file osver */
				/* Note: if S900 sample, use non-compressed sample format */
			}

			/* Note: akai_wav2sample() will correct osver if necessary */
			wavret=akai_wav2sample(fd,dirnamebuf,
								   curvolp,
								   AKAI_CREATE_FILE_NOINDEX,
								   ftype,
								   (flags&TAR_IMPORT_WAVS9C)?1:0, /* if S900 sample, use compressed/non-compressed sample format */
								   osver,
								   tagp,
								   &wavbcount,
								   WAV2SAMPLE_OVERWRITE);
			if ((wavret<0)||(byteend<wavbcount)){ /* error? */
				ret=-1;
				goto tar_import_done;
			}
			skip+=byteend-wavbcount; /* skip unread rest */
			continue; /* file done, next */
		}

#if 1
		{
			static u_char anamebuf[AKAI_NAME_LEN+4+1]; /* +4 for ".<type>", +1 for '\0' */
			u_int osver;

			/* skip file with invalid name/type */
			if (ascii2akai_filename(dirnamebuf,anamebuf,&osver,curvolp->type==AKAI_VOL_TYPE_S900)==AKAI_FTYPE_FREE){
				if (verbose){
					PRINTF_OUT("invalid file name/type, skipping file\n");
					FLUSH_ALL;
				}
				skip+=byteend; /* skip file */
				continue; /* next */
			}
		}
#endif
		/* check if destination file already exists */
		if (akai_find_file(curvolp,&tmpfile,dirnamebuf)==0){
			/* exists */
			/* delete file */
			if (akai_delete_file(&tmpfile)<0){
				PRINTF_ERR("cannot overwrite existing file\n");
				ret=-1;
				goto tar_import_done;
			}
		}
		/* create destination file */
		/* Note: akai_create_file() will correct osver if necessary */
		if (akai_create_file(curvolp,
			&tmpfile,
			byteend,
			AKAI_CREATE_FILE_NOINDEX,
			dirnamebuf,
			osver,
			tagp)<0){
				PRINTF_ERR("cannot create file\n");
				ret=-1;
				goto tar_import_done;
		}
		/* import file */
		if (akai_write_file(fd,NULL,&tmpfile,0,tmpfile.size)<0){
			ret=-1;
			goto tar_import_done;
		}
		if ((curvolp->type==AKAI_VOL_TYPE_S900)&&(tmpfile.osver!=0) /* compressed file in S900 volume? */
			&&(osver==0)){ /* and supplied/determined osver is incompatible? */
				/* update uncompr value */
				akai_s900comprfile_updateuncompr(&tmpfile); /* ignore error */
		}
#if 1
		/* fix name in header (if necessary) */
		akai_fixramname(&tmpfile); /* ignore error */
#endif
	}

tar_import_done:
	restore_curdir();
	return ret;
}



/* EOF */
