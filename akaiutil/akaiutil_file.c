/*
* Copyright (C) 2010,2012,2018,2019,2020 Klaus Michael Indlekofer. All rights reserved.
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
#include "akaiutil.h"
#include "akaiutil_file.h"
#include "akaiutil_wav.h"



/* AKAI file info */
int
akai_file_info(struct file_s *fp,int verbose)
{
	static u_char hdrbuf[AKAI_FL_BLOCKSIZE]; /* XXX enough */
	u_int hdrsize;
	struct akai_genfilehdr_s *hdrp;
	static char nbuf[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	int s900flag;
	int sampleflag;
	int programflag;
	int cuelistflag;
	int takelistflag;
	int drum1000flag;
	u_int i;

	if ((fp==NULL)
		||(fp->volp==NULL)
		||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	if (verbose){
		PRINTF_OUT("%s\n",fp->name);
		PRINTF_OUT("fnr:     %u\n",fp->index+1);
		PRINTF_OUT("start:   block 0x%04x\n",fp->bstart);
		PRINTF_OUT("size:    %u bytes\n",fp->size);
		/* file type */
		PRINTF_OUT("type:    ");
		hdrsize=sizeof(struct akai_genfilehdr_s); /* default */
		s900flag=0; /* default */
		sampleflag=0; /* default */
		programflag=0; /* default */
		cuelistflag=0; /* default */
		takelistflag=0; /* default */
		drum1000flag=0; /* default */
		switch (fp->type){
		case AKAI_SAMPLE900_FTYPE: /* S900 sample file */
			PRINTF_OUT("S900 sample\n");
			hdrsize=sizeof(struct akai_sample900_s);
			s900flag=1;
			sampleflag=1;
			break;
		case AKAI_SAMPLE1000_FTYPE: /* S1000 sample file */
			PRINTF_OUT("S1000 sample\n");
			hdrsize=sizeof(struct akai_sample1000_s);
			sampleflag=1;
			break;
		case AKAI_SAMPLE3000_FTYPE: /* S3000 sample file */
			PRINTF_OUT("S3000 sample\n");
			hdrsize=sizeof(struct akai_sample3000_s);
			sampleflag=1;
			break;
		case AKAI_CDSAMPLE3000_FTYPE: /* CD3000 CD-ROM sample parameters file */
			PRINTF_OUT("CD3000 CD-ROM sample parameters\n");
			/* Note: contains S3000 sample header */
			hdrsize=sizeof(struct akai_sample3000_s);
			sampleflag=1;
			break;
		case AKAI_PROGRAM900_FTYPE: /* S900 program file */
			PRINTF_OUT("S900 program\n");
			s900flag=1;
			programflag=1;
			break;
		case AKAI_PROGRAM1000_FTYPE: /* S1000 program file */
			PRINTF_OUT("S1000 program\n");
			programflag=1;
			break;
		case AKAI_PROGRAM3000_FTYPE: /* S3000 program file */
			PRINTF_OUT("S3000 program\n");
			programflag=1;
			break;
		case AKAI_DRUM900_FTYPE: /* S900/S950 drum settings file */
			PRINTF_OUT("S900/S950 drum settings\n");
			/*s900flag=1;*/
			/* S900/S950 drum settings info */
			return akai_drum900_info(fp);
		case AKAI_DRUM1000_FTYPE: /* S1000 or S3000 drum settings file */
			PRINTF_OUT("S1000/S3000 drum settings\n");
			drum1000flag=1;
			break;
		case AKAI_FXFILE_FTYPE: /* S1100 or S3000 effects file */
			PRINTF_OUT("S1100/S3000 effects\n");
			break;
		case AKAI_QLFILE_FTYPE: /* S1100 or S3000 cue-list file */
			PRINTF_OUT("S1100/S3000 cue-list\n");
			cuelistflag=1;
			break;
		case AKAI_TLFILE_FTYPE: /* S1100 or S3000 take-list file */
			PRINTF_OUT("S1100/S3000 take-list\n");
			takelistflag=1;
			break;
		case AKAI_MULTI3000_FTYPE: /* S3000 multi file */
			PRINTF_OUT("S3000 multi\n");
			break; /* use generic file header */
		case AKAI_FIXUP900_FTYPE: /* S900 fixup file */
			PRINTF_OUT("S900 fixup\n");
			/*s900flag=1;*/
			return 0; /* done */
		case AKAI_MEMIMG900_FTYPE: /* S900 memory image file */
			PRINTF_OUT("S900 memory image\n");
			/*s900flag=1;*/
			return 0; /* done */
		case AKAI_SYS1000_FTYPE: /* S1000 operating system file */
			PRINTF_OUT("S1000 operating system\n");
			return 0; /* done */
		case AKAI_SYS3000_FTYPE: /* S3000 operating system file */
			PRINTF_OUT("S3000 operating system\n");
			return 0; /* done */
		case AKAI_OVS900_FTYPE: /* S900 overall settings file */
			PRINTF_OUT("S900 overall settings\n");
			/*s900flag=1;*/
			/* S900 overall settings info */
			return akai_ovs900_info(fp);
		case AKAI_CDSETUP3000_FTYPE:
			/* CD3000 CD-ROM setup file */
			/* Note: generic file header does not apply for this file type! */
			PRINTF_OUT("CD3000 CD-ROM setup\n");
			return akai_cdsetup3000_info(fp);
		default:
			/* unknown or unsupported */
			PRINTF_OUT("\?\?\?\n");
			return 1; /* no error */
		}

		/* read header */
		if (akai_read_file(0,hdrbuf,fp,0,hdrsize)<0){
			PRINTF_ERR("cannot read header\n");
			return -1;
		}

		/* name */
		if (s900flag){ /* S900 type? */
			/* Note: name in S900 sample/program file header starts at byte 0 */
			akai2ascii_name(hdrbuf,nbuf,1); /* 1: S900 */
		}else{
			/* generic file header */
			hdrp=(struct akai_genfilehdr_s *)hdrbuf;
			akai2ascii_name(hdrp->name,nbuf,0); /* 0: not S900 */
		}
		PRINTF_OUT("ramname: \"%s\"\n",nbuf);
		if (sampleflag){
			/* sample info */
			akai_sample_info(fp,hdrbuf);
		}else if (programflag){
			/* program info */
			return akai_program_info(fp);
		}else if (cuelistflag){
			/* cue-list info */
			return akai_cuelist_info(fp);
		}else if (takelistflag){
			/* take-list info */
			return akai_takelist_info(fp);
		}else if (drum1000flag){
			/* S1000/S3000 drum settings info */
			return akai_drum1000_info(fp);
		}
	}else{
		PRINTF_OUT("%3u  %-16s %9u    0x%04x  ",fp->index+1,fp->name,fp->size,fp->bstart);
		if (fp->volp->type==AKAI_VOL_TYPE_S900){
			/* S900 volume */
			if (fp->osver!=0){ /* compressed file? */
				/* number of un-compressed floppy blocks */
				PRINTF_OUT("  %5u",fp->osver);
			}
		}else{
			/* S1000/S3000 volume */
			/* OS version */
			if (((0xff&(fp->osver>>8))<100)&&((0xff&fp->osver)<100)){
				PRINTF_OUT("%2u.%02u  ",0xff&(fp->osver>>8),0xff&fp->osver);
			}else{
				PRINTF_OUT("%5u  ",fp->osver);
			}
			/* tags */
			for (i=0;i<AKAI_FILE_TAGNUM;i++){
				if ((fp->tag[i]>=1)&&(fp->tag[i]<=AKAI_PARTHEAD_TAGNUM)){
					PRINTF_OUT("%02u ",fp->tag[i]);
				}else if ((fp->tag[i]==AKAI_FILE_TAGFREE)||(fp->tag[i]==AKAI_FILE_TAGS1000)){
					PRINTF_OUT("   ");
				}else{
					PRINTF_OUT("?  ");
				}
			}
		}
		PRINTF_OUT("\n");
	}

	return 0;
}

void
akai_sample_info(struct file_s *fp,u_char *hdrp)
{
	struct akai_sample3000_s *s3000hdrp;
	struct akai_sample900_s *s900hdrp;
	u_int samplecount;
	u_int samplerate;
	u_int i,j,k;

	if ((fp==NULL)||(hdrp==NULL)){
		return;
	}

	if (fp->type==AKAI_SAMPLE900_FTYPE){
		/* S900 sample header */
		s900hdrp=(struct akai_sample900_s *)hdrp;
		/* number of samples */
		/* XXX should be an even number */
		samplecount=(s900hdrp->slen[3]<<24)
			+(s900hdrp->slen[2]<<16)
			+(s900hdrp->slen[1]<<8)
			+s900hdrp->slen[0];
		PRINTF_OUT("scount:  0x%08x (%9u)\n",samplecount,samplecount);
		samplerate=(s900hdrp->srate[1]<<8)+s900hdrp->srate[0];
		PRINTF_OUT("srate:   %uHz\n",samplerate);
		if (samplerate>0){
			PRINTF_OUT("sdur:    %.3lfms\n",1000.0*((double)samplecount)/((double)samplerate));
		}
		PRINTF_OUT("npitch:  %.2lf\n",((double)((s900hdrp->npitch[1]<<8)+s900hdrp->npitch[0]))/16.0);
		PRINTF_OUT("loud:    %+i\n",(((int)(char)s900hdrp->loud[1])<<8)+((int)(u_char)s900hdrp->loud[0]));
		PRINTF_OUT("dmadesa: 0x%02x%02x\n",
			s900hdrp->dmadesa[1],
			s900hdrp->dmadesa[0]);
		PRINTF_OUT("locat:   0x%02x%02x%02x%02x\n",
			s900hdrp->locat[3],
			s900hdrp->locat[2],
			s900hdrp->locat[1],
			s900hdrp->locat[0]);
		PRINTF_OUT("start:   0x%02x%02x%02x%02x\n",
			s900hdrp->start[3],
			s900hdrp->start[2],
			s900hdrp->start[1],
			s900hdrp->start[0]);
		PRINTF_OUT("end:     0x%02x%02x%02x%02x\n",
			s900hdrp->end[3],
			s900hdrp->end[2],
			s900hdrp->end[1],
			s900hdrp->end[0]);
		PRINTF_OUT("llen:    0x%02x%02x%02x%02x\n",
			s900hdrp->llen[3],
			s900hdrp->llen[2],
			s900hdrp->llen[1],
			s900hdrp->llen[0]);
		PRINTF_OUT("pmode:   ");
		switch (s900hdrp->pmode){
		case SAMPLE900_PMODE_ONESHOT:
			PRINTF_OUT("ONESHOT\n");
			break;
		case SAMPLE900_PMODE_LOOP:
			PRINTF_OUT("LOOP\n");
			break;
		case SAMPLE900_PMODE_ALTLOOP:
			PRINTF_OUT("ALTLOOP\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("dir:     ");
		switch (s900hdrp->dir){
		case SAMPLE900_DIR_NORM:
			PRINTF_OUT("NORM\n");
			break;
		case SAMPLE900_DIR_REV:
			PRINTF_OUT("REV\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("type:    ");
		switch (s900hdrp->type){
		case SAMPLE900_TYPE_NORM:
			PRINTF_OUT("NORM\n");
			break;
		case SAMPLE900_TYPE_VELXF:
			PRINTF_OUT("VELXF\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("compr:   %s\n",
			(fp->osver!=0)?"ON":"OFF"); /* S900 compressed/non-compressed sample format */
	}else if ((fp->type==AKAI_SAMPLE1000_FTYPE)
			  ||(fp->type==AKAI_SAMPLE3000_FTYPE)
			  ||(fp->type==AKAI_CDSAMPLE3000_FTYPE)){
		/* S1000/S3000 sample header */
		s3000hdrp=(struct akai_sample3000_s *)hdrp;
		/* Note: S1000 header is contained within S3000 header */
		/* number of samples */
		samplecount=(s3000hdrp->s1000.slen[3]<<24)
			+(s3000hdrp->s1000.slen[2]<<16)
			+(s3000hdrp->s1000.slen[1]<<8)
			+s3000hdrp->s1000.slen[0];
		PRINTF_OUT("scount:  0x%08x (%9u)\n",samplecount,samplecount);
		samplerate=(s3000hdrp->s1000.srate[1]<<8)+s3000hdrp->s1000.srate[0];
		PRINTF_OUT("srate:   %uHz\n",samplerate);
		if (samplerate>0){
			PRINTF_OUT("sdur:    %.3lfms\n",1000.0*((double)samplecount)/((double)samplerate));
		}
		PRINTF_OUT("bandw:   ");
		switch (s3000hdrp->s1000.bandw){
		case SAMPLE1000_BANDW_10KHZ:
			PRINTF_OUT("10kHz\n");
			break;
		case SAMPLE1000_BANDW_20KHZ:
			PRINTF_OUT("20kHz\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("rkey:    %u\n",s3000hdrp->s1000.rkey);
		PRINTF_OUT("ctune:   %+i\n",(int)(char)s3000hdrp->s1000.ctune);
		PRINTF_OUT("stune:   %+i\n",(int)(char)s3000hdrp->s1000.stune);
		PRINTF_OUT("hltoff:  %+i\n",(int)(char)s3000hdrp->s1000.hltoff);
		k=(s3000hdrp->s1000.stpaira[1]<<8)+s3000hdrp->s1000.stpaira[0];
		if (k!=AKAI_SAMPLE1000_STPAIRA_NONE){
			if (fp->type==AKAI_SAMPLE3000_FTYPE){
				k*=SAMPLE3000_STPAIRA_MULT;
				PRINTF_OUT("stpaira: 0x%05x\n",k);
			}else{
				PRINTF_OUT("stpaira: 0x%04x\n",k);
			}
		}
		PRINTF_OUT("locat:   0x%02x%02x%02x%02x\n",
			s3000hdrp->s1000.locat[3],
			s3000hdrp->s1000.locat[2],
			s3000hdrp->s1000.locat[1],
			s3000hdrp->s1000.locat[0]);
		PRINTF_OUT("start:   0x%02x%02x%02x%02x\n",
			s3000hdrp->s1000.start[3],
			s3000hdrp->s1000.start[2],
			s3000hdrp->s1000.start[1],
			s3000hdrp->s1000.start[0]);
		PRINTF_OUT("end:     0x%02x%02x%02x%02x\n",
			s3000hdrp->s1000.end[3],
			s3000hdrp->s1000.end[2],
			s3000hdrp->s1000.end[1],
			s3000hdrp->s1000.end[0]);
		PRINTF_OUT("pmode:   ");
		switch (s3000hdrp->s1000.pmode){
		case SAMPLE1000_PMODE_LOOP:
			PRINTF_OUT("LOOP\n");
			break;
		case SAMPLE1000_PMODE_LOOPNOTREL:
			PRINTF_OUT("LOOPNOTREL\n");
			break;
		case SAMPLE1000_PMODE_NOLOOP:
			PRINTF_OUT("NOLOOP\n");
			break;
		case SAMPLE1000_PMODE_TOEND:
			PRINTF_OUT("TOEND\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("lnum:    %u\n",s3000hdrp->s1000.lnum);
		PRINTF_OUT("lfirst:  %u\n",s3000hdrp->s1000.lfirst+1);
		for (i=0;i<AKAI_SAMPLE1000_LOOPNUM;i++){
			k=(s3000hdrp->s1000.loop[i].time[1]<<8)+s3000hdrp->s1000.loop[i].time[0];
			if (k==SAMPLE1000LOOP_TIME_NOLOOP){
				continue; /* next loop */
			}
			j=(s3000hdrp->s1000.loop[i].at[3]<<24)
				+(s3000hdrp->s1000.loop[i].at[2]<<16)
				+(s3000hdrp->s1000.loop[i].at[1]<<8)
				+s3000hdrp->s1000.loop[i].at[0];
			if (j>=samplecount){ /* invalid? */
				continue; /* next loop */
			}
			PRINTF_OUT("loop %u:\n",i+1);
			PRINTF_OUT("  loopat:  0x%08x\n",j);
			PRINTF_OUT("  length:  0x%02x%02x%02x%02x.0x%02x%02x\n",
				s3000hdrp->s1000.loop[i].len[3],
				s3000hdrp->s1000.loop[i].len[2],
				s3000hdrp->s1000.loop[i].len[1],
				s3000hdrp->s1000.loop[i].len[0],
				s3000hdrp->s1000.loop[i].flen[1],
				s3000hdrp->s1000.loop[i].flen[0]);
			if (k==SAMPLE1000LOOP_TIME_HOLD){
				PRINTF_OUT("  ltime:   HOLD\n");
			}else{
				PRINTF_OUT("  ltime:   %ums\n",k);
			}
		}
	}
}

int
akai_program_info(struct file_s *fp)
{
	static char nbuf[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	struct akai_program900_s *s900hdrp;
	struct akai_program900kg_s *s900kgp;
	struct akai_program3000_s *s3000hdrp;
	struct akai_program3000kg_s *s3000kgp;
	u_char *buf;
	u_int hdrsiz;
	u_int kgsiz;
	u_int kgnum;
	u_int i,j,k;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	if (fp->type==AKAI_PROGRAM900_FTYPE){
		hdrsiz=sizeof(struct akai_program900_s);
		kgsiz=sizeof(struct akai_program900kg_s);
	}else if (fp->type==AKAI_PROGRAM1000_FTYPE){
		hdrsiz=sizeof(struct akai_program1000_s);
		kgsiz=sizeof(struct akai_program1000kg_s);
	}else if (fp->type==AKAI_PROGRAM3000_FTYPE){
		hdrsiz=sizeof(struct akai_program3000_s);
		kgsiz=sizeof(struct akai_program3000kg_s);
	}else{
		return -1;
	}

	/* allocate buffer */
	if (fp->size<hdrsiz){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	if (fp->type==AKAI_PROGRAM900_FTYPE){
		/* S900 program header */
		s900hdrp=(struct akai_program900_s *)buf;
		PRINTF_OUT("kgxf:    %s\n",(s900hdrp->kgxf!=0x00)?"ON":"OFF");
		kgnum=s900hdrp->kgnum;
		PRINTF_OUT("kgnum:   %u\n",kgnum);
		if (fp->size<hdrsiz+kgnum*kgsiz){
			PRINTF_ERR("invalid file size\n");
			free(buf);
			return -1;
		}
		k=(s900hdrp->kg1a[1]<<8)+s900hdrp->kg1a[0];
		if (k!=PROGRAM900_KGA_NONE){
			PRINTF_OUT("kg1a:    0x%04x\n",k);
		}
		for (i=0;i<kgnum;i++){
			PRINTF_OUT("keygroup %u:\n",i+1);
			s900kgp=(struct akai_program900kg_s *)(buf+hdrsiz+i*kgsiz);
			PRINTF_OUT("  midichoff: %u\n",s900kgp->midichoff);
			PRINTF_OUT("  key lo-hi: %u-%u\n",s900kgp->keylo,s900kgp->keyhi);
			PRINTF_OUT("  outch:     ");
			switch (s900kgp->outch1){
			case PROGRAM900KG_OUTCH1_LEFT:
				PRINTF_OUT("LEFT\n");
				break;
			case PROGRAM900KG_OUTCH1_RIGHT:
				PRINTF_OUT("RIGHT\n");
				break;
			case PROGRAM900KG_OUTCH1_ANY:
				PRINTF_OUT("ANY\n");
				break;
			default:
				PRINTF_OUT("%u\n",s900kgp->outch1+1);
				break;
			}
			PRINTF_OUT("  pitch:     %s\n",((PROGRAM900KG_FLAGS_PCONST&s900kgp->flags)!=0x00)?"CONST":"TRACK");
			PRINTF_OUT("  oneshot:   %s\n",((PROGRAM900KG_FLAGS_ONESHOT&s900kgp->flags)!=0x00)?"ON":"OFF");
			PRINTF_OUT("  velxf:     %s\n",((PROGRAM900KG_FLAGS_VELXF&s900kgp->flags)!=0x00)?"ON":"OFF");
			PRINTF_OUT("  velxfv50:  %u\n",s900kgp->velxfv50);
			PRINTF_OUT("  velswth:   %u",s900kgp->velswth);
			if (s900kgp->velswth<=PROGRAM900KG_VELSWTH_NOSOFT){
				PRINTF_OUT(" (no soft sample)\n");
			}else if (s900kgp->velswth>=PROGRAM900KG_VELSWTH_NOLOUD){
				PRINTF_OUT(" (no loud sample)\n");
			}else{
				PRINTF_OUT("\n");
			}
			PRINTF_OUT("  sample 1 (soft):\n");
			PRINTF_OUT("    tune:    %+.2lf\n",((double)((((int)(char)s900kgp->tune1[1])<<8)+((int)(u_char)s900kgp->tune1[0])))/16.0);
			PRINTF_OUT("    filter:  %u\n",s900kgp->filter1);
			PRINTF_OUT("    loud:    %+i\n",(int)(char)s900kgp->loud1);
			akai2ascii_name(s900kgp->sname1,nbuf,1); /* 1: S900 */
			PRINTF_OUT("    sname:   \"%s\"\n",nbuf);
			k=(s900kgp->shdra1[1]<<8)+s900kgp->shdra1[0];
			if (k!=PROGRAM900KG_SHDRA_NONE){
				PRINTF_OUT("    shdra:   0x%04x\n",k);
			}
			PRINTF_OUT("  sample 2 (loud):\n");
			PRINTF_OUT("    tune:    %+.2lf\n",((double)((((int)(char)s900kgp->tune2[1])<<8)+((int)(u_char)s900kgp->tune2[0])))/16.0);
			PRINTF_OUT("    filter:  %u\n",s900kgp->filter2);
			PRINTF_OUT("    loud:    %+i\n",(int)(char)s900kgp->loud2);
			akai2ascii_name(s900kgp->sname2,nbuf,1); /* 1: S900 */
			PRINTF_OUT("    sname:   \"%s\"\n",nbuf);
			k=(s900kgp->shdra2[1]<<8)+s900kgp->shdra2[0];
			if (k!=PROGRAM900KG_SHDRA_NONE){
				PRINTF_OUT("    shdra:   0x%04x\n",k);
			}
			k=(s900kgp->kgnexta[1]<<8)+s900kgp->kgnexta[0];
			if (k!=PROGRAM900_KGA_NONE){
				PRINTF_OUT("  kgnexta:   0x%04x\n",k);
			}
		}
	}else{
		/* S1000/S3000 program header */
		s3000hdrp=(struct akai_program3000_s *)buf;
		/* Note: S1000 header is contained within S3000 header */
		PRINTF_OUT("midich:    ");
		if (s3000hdrp->s1000.midich1==PROGRAM1000_MIDICH1_OMNI){
			PRINTF_OUT("OMNI\n");
		}else{
			PRINTF_OUT("%u\n",s3000hdrp->s1000.midich1+1);
		}
		PRINTF_OUT("key lo-hi: %u-%u\n",s3000hdrp->s1000.keylo,s3000hdrp->s1000.keyhi);
		PRINTF_OUT("oct:       %+i\n",(int)(char)s3000hdrp->s1000.oct);
		PRINTF_OUT("auxch:     ");
		if (s3000hdrp->s1000.auxch1==PROGRAM1000_AUXCH1_OFF){
			PRINTF_OUT("OFF\n");
		}else{
			PRINTF_OUT("%u\n",s3000hdrp->s1000.auxch1+1);
		}
		PRINTF_OUT("kgxf:      %s\n",(s3000hdrp->s1000.kgxf!=0x00)?"ON":"OFF");
		kgnum=s3000hdrp->s1000.kgnum;
		PRINTF_OUT("kgnum:     %u\n",kgnum);
		if (fp->size<hdrsiz+kgnum*kgsiz){
			PRINTF_ERR("invalid file size\n");
			free(buf);
			return -1;
		}
		k=(s3000hdrp->s1000.kg1a[1]<<8)+s3000hdrp->s1000.kg1a[0];
		if (k!=PROGRAM1000_KGA_NONE){
			if (fp->type==AKAI_PROGRAM3000_FTYPE){
				k*=PROGRAM3000_KGA_MULT;
				PRINTF_OUT("kg1a:      0x%05x\n",k);
			}else{
				PRINTF_OUT("kg1a:      0x%04x\n",k);
			}
		}
		for (i=0;i<kgnum;i++){
			PRINTF_OUT("keygroup %u:\n",i+1);
			s3000kgp=(struct akai_program3000kg_s *)(buf+hdrsiz+i*kgsiz);
			PRINTF_OUT("  key lo-hi: %u-%u\n",s3000kgp->s1000.keylo,s3000kgp->s1000.keyhi);
			PRINTF_OUT("  ctune:     %+i\n",(int)(char)s3000kgp->s1000.ctune);
			PRINTF_OUT("  stune:     %+i\n",(int)(char)s3000kgp->s1000.stune);
			PRINTF_OUT("  filter:    %u\n",s3000kgp->s1000.filter);
			PRINTF_OUT("  velxf:     %s\n",(s3000kgp->s1000.velxf!=0x00)?"ON":"OFF");
			for (j=0;j<PROGRAM1000KG_VELZONENUM;j++){
				PRINTF_OUT("  velzone %u:\n",j+1);
				akai2ascii_name(s3000kgp->s1000.velzone[j].sname,nbuf,0); /* 0: not S900 */
				PRINTF_OUT("    vel lo-hi: %u-%u\n",s3000kgp->s1000.velzone[j].vello,s3000kgp->s1000.velzone[j].velhi);
				PRINTF_OUT("    ctune:     %+i\n",(int)(char)s3000kgp->s1000.velzone[j].ctune);
				PRINTF_OUT("    stune:     %+i\n",(int)(char)s3000kgp->s1000.velzone[j].stune);
				PRINTF_OUT("    loud:      %+i\n",(int)(char)s3000kgp->s1000.velzone[j].loud);
				PRINTF_OUT("    filter:    %+i\n",(int)(char)s3000kgp->s1000.velzone[j].filter);
				PRINTF_OUT("    pan:       %+i\n",(int)(char)s3000kgp->s1000.velzone[j].pan);
				PRINTF_OUT("    auxchoff:  %u\n",s3000kgp->s1000.auxchoff[j]);
				PRINTF_OUT("    pitch:     %s\n",(s3000kgp->s1000.pconst[j]!=0x00)?"CONST":"TRACK");
				PRINTF_OUT("    pmode:     ");
				switch (s3000kgp->s1000.velzone[j].pmode){
				case PROGRAM1000_PMODE_SAMPLE:
					PRINTF_OUT("SAMPLE\n");
					break;
				case PROGRAM1000_PMODE_LOOP:
					PRINTF_OUT("LOOP\n");
					break;
				case PROGRAM1000_PMODE_LOOPNOTREL:
					PRINTF_OUT("LOOPNOTREL\n");
					break;
				case PROGRAM1000_PMODE_NOLOOP:
					PRINTF_OUT("NOLOOP\n");
					break;
				case PROGRAM1000_PMODE_TOEND:
					PRINTF_OUT("TOEND\n");
					break;
				default:
					PRINTF_OUT("\?\?\?\n");
					break;
				}
				PRINTF_OUT("    sname:     \"%s\"\n",nbuf);
				k=(s3000kgp->s1000.velzone[j].shdra[1]<<8)+s3000kgp->s1000.velzone[j].shdra[0];
				if (k!=PROGRAM1000KG_SHDRA_NONE){
					if (fp->type==AKAI_PROGRAM3000_FTYPE){
						k*=PROGRAM3000_SHDRA_MULT;
						PRINTF_OUT("    shdra:     0x%05x\n",k);
					}else{
						PRINTF_OUT("    shdra:     0x%04x\n",k);
					}
				}
			}
			k=(s3000kgp->s1000.kgnexta[1]<<8)+s3000kgp->s1000.kgnexta[0];
			if (k!=PROGRAM1000_KGA_NONE){
				if (fp->type==AKAI_PROGRAM3000_FTYPE){
					k*=PROGRAM3000_KGA_MULT;
					PRINTF_OUT("  kgnexta:   0x%05x\n",k);
				}else{
					PRINTF_OUT("  kgnexta:   0x%04x\n",k);
				}
			}
		}
	}

	free(buf);
	return 0;
}

int
akai_cuelist_info(struct file_s *fp)
{
	static char nbuf[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	struct akai_cuelist_s *hdrp;
	struct akai_cue_s *cuep;
	u_char *buf;
	u_int cuenum;
	char *tname;
	u_int i;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* allocate buffer */
	if (fp->size<sizeof(struct akai_cuelist_s)){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	hdrp=(struct akai_cuelist_s *)buf;
	cuenum=(u_int)hdrp->cuenum;
	PRINTF_OUT("cuenum:  %u\n",cuenum);
	if (fp->size<sizeof(struct akai_cuelist_s)+cuenum*sizeof(struct akai_cue_s)){
		PRINTF_ERR("invalid file size\n");
		free(buf);
		return -1;
	}

	if (cuenum>0){
		/* cues */
		PRINTF_OUT("\ncue  time           type  name          mnote mvel  lvl  pan\n");
		PRINTF_OUT("------------------------------------------------------------\n");
		for (i=0;i<cuenum;i++){
			cuep=(struct akai_cue_s *)(buf+sizeof(struct akai_cuelist_s)+i*sizeof(struct akai_cue_s));
			akai2ascii_name(cuep->name,nbuf,0); /* 0: not S900 */
			switch (cuep->type){
			case AKAI_CUETYPE_MOFF:
				tname="MOFF";
				break;
			case AKAI_CUETYPE_MON:
				tname="MON ";
				break;
			case AKAI_CUETYPE_TOFF:
				tname="TOFF";
				break;
			case AKAI_CUETYPE_TON:
				tname="TON ";
				break;
			default:
				tname="\?\?\? ";
				break;
			}
			PRINTF_OUT("%3u  %02x:%02x:%02x:%02x.%x  %s  %-12s    ",
				i+1,
				(u_int)cuep->time[4],
				(u_int)cuep->time[3],
				(u_int)cuep->time[2],
				(u_int)cuep->time[1],
				(u_int)(cuep->time[0]>>4),
				tname,
				nbuf);
			if ((cuep->type==AKAI_CUETYPE_MOFF)||(cuep->type==AKAI_CUETYPE_MON)){
				PRINTF_OUT("%3u  %3u\n",
					(u_int)cuep->midinote,
					(u_int)cuep->midivel);
			}else if ((cuep->type==AKAI_CUETYPE_TOFF)||(cuep->type==AKAI_CUETYPE_TON)){
				PRINTF_OUT("          %3u  %+3i\n",
					(u_int)cuep->stlvl,
					(int)(char)cuep->pan);
			}else{
				PRINTF_OUT("\n");
			}
		}
		PRINTF_OUT("------------------------------------------------------------\n");
	}

	free(buf);
	return 0;
}

int
akai_takelist_info(struct file_s *fp)
{
	static char nbuf[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	struct akai_cuelist_s *hdrp;
	struct akai_cue_s *cuep;
	u_char *buf;
	u_int cuenum;
	u_int i;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* allocate buffer */
	if (fp->size<sizeof(struct akai_cuelist_s)){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	hdrp=(struct akai_cuelist_s *)buf;
	cuenum=(u_int)hdrp->cuenum;
	PRINTF_OUT("cuenum:  %u\n",cuenum);
	if (fp->size<sizeof(struct akai_cuelist_s)+cuenum*sizeof(struct akai_cue_s)){
		PRINTF_ERR("invalid file size\n");
		free(buf);
		return -1;
	}

	if (cuenum>0){
		/* cues */
		PRINTF_OUT("\ncue  rep  name          mnote  mch  lvl  pan  fadein fadeout\n");
		PRINTF_OUT("------------------------------------------------------------\n");
		for (i=0;i<cuenum;i++){
			cuep=(struct akai_cue_s *)(buf+sizeof(struct akai_cuelist_s)+i*sizeof(struct akai_cue_s));
			akai2ascii_name(cuep->name,nbuf,0); /* 0: not S900 */
			PRINTF_OUT("%3u  %3u  %-12s    %3u  %3u  %3u  %+3i   %5u   %5u\n",
				i+1,
				((u_int)cuep->repeat1)+1,
				nbuf,
				(u_int)cuep->midinote,
				((u_int)cuep->midich1)+1,
				(u_int)cuep->stlvl,
				(int)(char)cuep->pan,
				(((u_int)cuep->fadein[1])<<8)|((u_int)cuep->fadein[0]),
				(((u_int)cuep->fadeout[1])<<8)|((u_int)cuep->fadeout[0]));
		}
		PRINTF_OUT("------------------------------------------------------------\n");
	}

	free(buf);
	return 0;
}

int
akai_drum1000_info(struct file_s *fp)
{
	struct akai_drum1000_s *dfp;
	struct akai_drum1000des_s *ddp;
	u_char *buf;
	u_int i,j;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* allocate buffer */
	if (fp->size<sizeof(struct akai_drum1000_s)){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	dfp=(struct akai_drum1000_s *)buf;
	ddp=dfp->ddes1; /* for ME35T unit 1 */
	for (i=0;i<DRUM1000_UNITNUM;i++){
		PRINTF_OUT("\nS1000/S3000 drum settings for ME35T unit %u:\n",i+1);
		PRINTF_OUT("inp midich mnote  sens  trig vcurv captt recovt ontime\n");
		PRINTF_OUT("------------------------------------------------------\n");
		for (j=0;j<DRUM1000_INPUTNUM;j++){
			PRINTF_OUT("%3u    %3u   %3u   %3u   %3u  %3u  %3ums  %3ums  %3ums\n",
				j+1,
				((u_int)ddp->midich1)+1,
				(u_int)ddp->midinote,
				(u_int)ddp->sens,
				(u_int)ddp->trig,
				((u_int)ddp->vcurv1)+1,
				(u_int)ddp->captt,
				(u_int)ddp->recovt,
				(u_int)ddp->ontime);
		}
		PRINTF_OUT("------------------------------------------------------\n");
		ddp=dfp->ddes2; /* for ME35T unit 2 */
	}

	free(buf);
	return 0;
}

int
akai_drum900_info(struct file_s *fp)
{
	struct akai_drum900_s *dfp;
	struct akai_drum900des_s *ddp;
	u_char *buf;
	u_int captt,ontime,recovt;
	u_int j;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* allocate buffer */
	if (fp->size<sizeof(struct akai_drum900_s)){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	dfp=(struct akai_drum900_s *)buf;
	PRINTF_OUT("ASK90 enable:      %s\n",dfp->ask90enab?"ON":"OFF");
	PRINTF_OUT("ASK90 sensitivity: %s\n",dfp->ask90sens?"HIGH":"LOW");
	PRINTF_OUT("\nS900/S950 drum settings for ASK90/ME35T:\n");
	PRINTF_OUT("itype inp midich mnote  gain  trig vcurv captt ontime recovt resetba   adcla\n");
	PRINTF_OUT("----------------------------------------------------------------------------\n");
	for (j=0;j<DRUM900_INPUTNUM;j++){
		ddp=&dfp->ddes[j];
		captt=((ddp->captt4[1]<<8)+ddp->captt4[0])>>2;
		ontime=((ddp->ontime4[1]<<8)+ddp->ontime4[0])>>2;
		recovt=((ddp->recovt4[1]<<8)+ddp->recovt4[0])>>2;
		if (ddp->itype==DRUM900DES_ITYPE_ME35T){
			/* for ME35T */
			PRINTF_OUT("ME35T %3u    %3u   %3u   %3u   %3u  %3u  %3ums  %3ums  %3ums       -       -\n",
				((u_int)ddp->inpnr1)+1,
				((u_int)ddp->midich1)+1,
				(u_int)ddp->midinote,
				(u_int)ddp->gain,
				(u_int)ddp->me35t_trig,
				((u_int)ddp->me35t_vcurv1)+1,
				captt,
				ontime,
				recovt);
		}else{
			/* for ASK90 */
			PRINTF_OUT("ASK90 %3u    %3u   %3u   %3u   %3u    -  %3ums  %3ums  %3ums  0x%02x%02x  0x%02x%02x\n",
				((u_int)ddp->inpnr1)+1,
				((u_int)ddp->midich1)+1,
				(u_int)ddp->midinote,
				(u_int)ddp->gain,
				(u_int)ddp->ask90_trig,
				captt,
				ontime,
				recovt,
				(u_int)ddp->ask90_resba[1],(u_int)ddp->ask90_resba[0],
				(u_int)ddp->ask90_adcla[1],(u_int)ddp->ask90_adcla[0]);
		}
	}
	PRINTF_OUT("----------------------------------------------------------------------------\n");

	free(buf);
	return 0;
}

int
akai_ovs900_info(struct file_s *fp)
{
	static char nbuf[AKAI_NAME_LEN_S900+1]; /* +1 for '\0' */
	struct akai_ovs900_s *ofp;
	u_char *buf;
	u_int i;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* allocate buffer */
	if (fp->size<sizeof(struct akai_ovs900_s)){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	ofp=(struct akai_ovs900_s *)buf;
	akai2ascii_name((u_char *)ofp->progname,nbuf,1); /* 1: S900 */
	PRINTF_OUT("selected program name: \"%s\"\n",nbuf);
	i=(ofp->testmidich[1]<<8)+ofp->testmidich[0];
	PRINTF_OUT("test MIDI channel:     %u\n",i);
	i=(ofp->testmidikey[1]<<8)+ofp->testmidikey[0];
	PRINTF_OUT("test MIDI key:         %u\n",i);
	i=(ofp->testmidivel[1]<<8)+ofp->testmidivel[0];
	PRINTF_OUT("test MIDI velocity:    %u\n",i);
	PRINTF_OUT("basic MIDI channel:    %u\n",((u_int)(OVS900_BMIDICH1MASK&ofp->bmidich1omni))+1);
	PRINTF_OUT("MIDI OMNI:             %s\n",(OVS900_OMNIMASK&ofp->bmidich1omni)?"ON":"OFF");
	PRINTF_OUT("loudness:              %s\n",ofp->loudness?"ON":"OFF");
	PRINTF_OUT("control port:          ");
	switch (ofp->ctrlport){
	case OVS900_CTRLPORT_MIDI:
		PRINTF_OUT("MIDI\n");
		break;
	case OVS900_CTRLPORT_RS232:
		PRINTF_OUT("RS232\n");
		break;
	default:
		PRINTF_OUT("\?\?\?\n");
		break;
	}
	PRINTF_OUT("program change:        %s\n",ofp->progchange?"ON":"OFF");
	PRINTF_OUT("pitch wheel range:     %u\n",(u_int)ofp->pwheelrange);
	i=((ofp->rs232brate10[1]<<8)+ofp->rs232brate10[0])*10;
	PRINTF_OUT("RS232 baudrate:        %u\n",i);

	free(buf);
	return 0;
}

int
akai_cdsetup3000_info(struct file_s *fp)
{
	char nbuf[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	u_char *buf;
	struct akai_cdsetup3000_s *hp;
	struct akai_cdsetup3000_entry_s *ep;
	u_int enr;
	u_int e,i;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* allocate buffer */
	if (fp->size<sizeof(struct akai_cdsetup3000_s)){
		PRINTF_ERR("invalid file size\n");
		return -1;
	}
	if ((buf=(u_char *)malloc(fp->size))==NULL){
		PRINTF_ERR("cannot allocate memory\n");
		return -1;
	}

	/* header */
	hp=(struct akai_cdsetup3000_s *)buf;
	/* marked file entries */
	ep=(struct akai_cdsetup3000_entry_s *)(buf+sizeof(struct akai_cdsetup3000_s));
	/* number of entries */
	enr=(fp->size-sizeof(struct akai_cdsetup3000_s))/sizeof(struct akai_cdsetup3000_entry_s);

	/* read file */
	if (akai_read_file(0,buf,fp,0,fp->size)<0){
		PRINTF_ERR("cannot read file\n");
		free(buf);
		return -1;
	}

	/* name */
	akai2ascii_name(hp->name,nbuf,fp->volp->type==AKAI_VOL_TYPE_S900);
	PRINTF_OUT("ramname: \"%s\"\n\n",nbuf);

	/* CD-ROM label */
	akai2ascii_name(hp->cdlabel,nbuf,fp->volp->type==AKAI_VOL_TYPE_S900);
	PRINTF_OUT("CD-ROM label: \"%s\"\n",nbuf);

	/* print marked file entries */
	PRINTF_OUT("\nmarked files (part:vol/file):\n---------------------------------------------\n");
	i=0;
	for (e=0;e<enr;e++){
		if (ep[e].parti!=0xff){ /* used entry? */
			PRINTF_OUT("%c:%03u/%03u   ",
				'A'+ep[e].parti,
				ep[e].voli+1,
				(ep[e].filei[1]<<8)+ep[e].filei[0]+1);
			if (i%4==3){
				PRINTF_OUT("\n");
			}
			i++;
		}
	}
	if (i%4!=0){
		PRINTF_OUT("\n");
	}
	PRINTF_OUT("---------------------------------------------\n");

	free(buf);
	return 0;
}



/* fix RAM name of AKAI file */
int
akai_fixramname(struct file_s *fp)
{
	static char nbuf[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	static u_char buf[sizeof(struct akai_genfilehdr_s)]; /* XXX enough for all cases */
	u_char *np;
	int s900flag;

	if ((fp==NULL)||(fp->volp==NULL)||(fp->index>=fp->volp->fimax)){
		return -1;
	}

	/* file type */
	s900flag=0; /* default */
	switch (fp->type){
	case AKAI_SAMPLE900_FTYPE: /* S900 sample */
	case AKAI_PROGRAM900_FTYPE: /* S900 program */
		/* Note: name in S900 sample/program file header starts at byte 0 */
		np=buf;
		s900flag=1;
		break;
	case AKAI_SAMPLE1000_FTYPE: /* S1000 sample */
	case AKAI_SAMPLE3000_FTYPE: /* S3000 sample */
	case AKAI_CDSAMPLE3000_FTYPE: /* CD3000 CD-ROM sample parameters */
	case AKAI_PROGRAM1000_FTYPE: /* S1000 program */
	case AKAI_PROGRAM3000_FTYPE: /* S3000 program */
	case AKAI_DRUM1000_FTYPE: /* S1000 or S3000 drum settings file */
	case AKAI_FXFILE_FTYPE: /* S1100 or S3000 effects file */
	case AKAI_QLFILE_FTYPE: /* S1100 or S3000 cue-list file */
	case AKAI_TLFILE_FTYPE: /* S1100 or S3000 take-list file */
	case AKAI_MULTI3000_FTYPE: /* S3000 multi file */
		{
			/* use generic file header */
			struct akai_genfilehdr_s *p;
			p=(struct akai_genfilehdr_s *)buf;
			np=p->name;
		}
		break;
	case AKAI_CDSETUP3000_FTYPE:
		{
			/* CD3000 CD-ROM setup file */
			/* Note: generic file header does not apply for this file type! */
			struct akai_cdsetup3000_s *p;
			p=(struct akai_cdsetup3000_s *)buf;
			np=p->name;
		}
		break;
	default:
		/* unknown or unsupported */
		return 1; /* no error */
	}

	/* read header */
	if (akai_read_file(0,buf,fp,0,sizeof(struct akai_genfilehdr_s))<0){
		PRINTF_ERR("cannot read header\n");
		return -1;
	}

	/* copy file name to name in RAM */
	akai2ascii_name(fp->volp->file[fp->index].name,nbuf,fp->volp->type==AKAI_VOL_TYPE_S900);
	ascii2akai_name(nbuf,np,s900flag);

	/* write header */
	if (akai_write_file(0,buf,fp,0,sizeof(struct akai_genfilehdr_s))<0){
		PRINTF_ERR("cannot write header\n");
		return -1;
	}

	return 0;
};



int
akai_s900comprfile_updateuncompr(struct file_s *fp)
{
	u_int osver;

	if (fp==NULL){
		return -1;
	}

	/* check if compressed file in S900 volume */
	if ((fp->volp==NULL)||(fp->volp->type!=AKAI_VOL_TYPE_S900)||(fp->osver==0)){
		return -1;
	}

	/* check if supported file type */
	if (fp->type!=AKAI_SAMPLE900_FTYPE){
		/* no error, keep osver */
		return 0;
	}

	/* S900 sample file, compressed sample format */
	/* non-compressed sample size in bytes */
	osver=akai_sample900_getsamplesize(fp);
	/* number of un-compressed floppy blocks */
	/* Note: without sample header */
	osver=(osver+AKAI_FL_BLOCKSIZE-1)/AKAI_FL_BLOCKSIZE; /* round up */
	if (osver==0){ /* unsuitable osver? */
		osver=1; /* XXX non zero */
	}

	/* set osver of file */
	/* Note: akai_rename_file() will correct osver if necessary */
	if (akai_rename_file(fp,NULL,fp->volp,AKAI_CREATE_FILE_NOINDEX,NULL,osver)<0){
		return -1;
	}

	return 0;
}



u_int
akai_sample900_getsamplesize(struct file_s *fp)
{
	static struct akai_sample900_s s900hdr;
	static u_int samplecount;
	static u_int samplecountpart;
	static u_int samplesize;

	if (fp==NULL){
		return 0;
	}
	if (fp->type!=(u_char)AKAI_SAMPLE900_FTYPE){ /* not S900 sample? */
		return 0;
	}

	/* read header to memory */
	if (akai_read_file(0,(u_char *)&s900hdr,fp,0,sizeof(struct akai_sample900_s))<0){
		PRINTF_ERR("cannot read sample header\n");
		return 0;
	}

	/* number of samples */
	/* XXX should be an even number */
	samplecount=(s900hdr.slen[3]<<24)
		+(s900hdr.slen[2]<<16)
		+(s900hdr.slen[1]<<8)
		+s900hdr.slen[0];
	/* number of samples per part  */
	samplecountpart=(samplecount+1)/2; /* round up */
	/* size in bytes in S900 non-compressed sample format */
	samplesize=3*samplecountpart;
	return samplesize;
}



void
akai_sample900noncompr_sample2wav(u_char *sbuf,u_char *wavbuf,u_int samplecountpart)
{
	u_int i;

	if ((sbuf==NULL)||(wavbuf==NULL)){
		return;
	}
	if (samplecountpart==0){
		return;
	}

	/* convert 12bit S900 non-compressed sample format into 16bit WAV sample format */
	for (i=0;i<samplecountpart;i++){ /* first part */
		wavbuf[i*2+1]=sbuf[i*2+1];
		wavbuf[i*2+0]=0xf0&sbuf[i*2+0];
	}
	for (i=0;i<samplecountpart;i++){ /* second part */
		wavbuf[samplecountpart*2+i*2+1]=sbuf[samplecountpart*2+i];
		wavbuf[samplecountpart*2+i*2+0]=0xf0&(sbuf[i*2+0]<<4);
	}
}

void
akai_sample900noncompr_wav2sample(u_char *sbuf,u_char *wavbuf,u_int samplecountpart)
{
	u_int i;

	if ((sbuf==NULL)||(wavbuf==NULL)){
		return;
	}
	if (samplecountpart==0){
		return;
	}

	/* convert 16bit WAV sample format into 12bit S900 non-compressed sample format */
	for (i=0;i<samplecountpart;i++){ /* first part */
		sbuf[i*2+1]=wavbuf[i*2+1];
		sbuf[i*2+0]=0xf0&wavbuf[i*2+0];
	}
	for (i=0;i<samplecountpart;i++){ /* second part */
		sbuf[samplecountpart*2+i]=wavbuf[samplecountpart*2+i*2+1];
		sbuf[i*2+0]|=0x0f&(wavbuf[samplecountpart*2+i*2+0]>>4);
	}
}



u_int
akai_sample900compr_getbits(u_char *buf,u_int bitpos,u_int bitnum)
{
	u_int bytepos;
	u_char bmask;
	u_int val;
	u_int i;

	if ((buf==NULL)||(bitnum==0)){
		return 0;
	}
	/* XXX no check if bitnum too large for u_int */
	/* XXX no check if bitpos+bitnum too large for buf */

	val=0;
	for (i=0;i<bitnum;i++,bitpos++){
		bytepos=(bitpos>>3); /* /8: 8 bits per byte */
		bmask=(1<<(7-(7&bitpos))); /* Note: upper bit first */
		val<<=1;
		if ((bmask&buf[bytepos])!=0){
			val|=1;
		}
	}

	return val;
}

u_int
akai_sample900compr_sample2wav(u_char *sbuf,u_char *wavbuf,u_int sbufsiz,u_int wavbufsiz)
{
	u_char upbitnum;
	u_char upsign;
	u_short upabsval;
	short curval;
	short curinc;
	u_int bitremain;
	u_int bitpos;
	u_int wavpos;
	u_char code;
	u_int i,j;

	if ((sbuf==NULL)||(wavbuf==NULL)){
		return 0;
	}
	if ((sbufsiz==0)||(wavbufsiz==0)){
		return 0;
	}

	/* convert S900 compressed sample format into 16bit WAV sample format */
	curval=0;
	curinc=0;
	bitremain=sbufsiz*8; /* 8 bits per byte */
	bitpos=0;
	wavpos=0;
	for (;(bitremain>0)&&(wavpos+1<wavbufsiz);){
		/* get code nibble */
		if (bitremain<4){
			break; /* end */
		}
		code=(u_char)akai_sample900compr_getbits(sbuf,bitpos,4);
#ifdef SAMPLE900COMPR_DEBUG
		PRINTF_OUT("%08x: code=%x\n",bitpos,code);
#endif
		bitpos+=4;
		bitremain-=4;
		/* parse code */
		if (code==0x0){
			/* Note: no further bits for this group */
			/* generate signal */
			for (i=0;(i<SAMPLE900COMPR_GROUP_SAMPNUM)&&(wavpos+1<wavbufsiz);i++){
				/* update curval */
				curval+=curinc;
				/* convert 12bit sample into 16bit WAV sample */
				wavbuf[wavpos++]=0xf0&(u_char)(curval<<4);
				wavbuf[wavpos++]=0xff&(u_char)(curval>>4);
			}
		}else{
			/* number of bits for upabsval */
			upbitnum=SAMPLE900COMPR_UPBITNUM_NEGCODE_OFF-code;
			/* Note: upbitnum>SAMPLE900COMPR_UPBITNUM_MAX is supported here, but should not occur normally */
#ifdef SAMPLE900COMPR_DEBUG
			if (upbitnum>SAMPLE900COMPR_UPBITNUM_MAX){
				PRINTF_OUT("          upbitnum>SAMPLE900COMPR_UPBITNUM_MAX\n");
			}
#endif
			/* number of required remaining bits for instruction code */
			j=SAMPLE900COMPR_GROUP_SAMPNUM*(1+((u_int)upbitnum)); /* Note: per word: 1 sign flag bit and n bits */
			if (bitremain<j){
				break; /* end */
			}
			/* generate signal */
			for (i=0;(i<SAMPLE900COMPR_GROUP_SAMPNUM)&&(wavpos+1<wavbufsiz);i++){
				/* get sign flag bit */
				upsign=(u_char)akai_sample900compr_getbits(sbuf,bitpos+0+i,1);
				/* get value word */
				upabsval=(u_short)akai_sample900compr_getbits(sbuf,bitpos+SAMPLE900COMPR_GROUP_SAMPNUM+i*((u_int)upbitnum),(u_int)upbitnum);
#ifdef SAMPLE900COMPR_DEBUG
				PRINTF_OUT("          %c%u\n",(upsign==0)?'+':'-',(u_int)upabsval);
#endif
				/* update curinc */
				if (upsign==0){
					/* plus */
					curinc+=(short)upabsval;
				}else{
					/* minus */
					curinc-=(short)upabsval;
				}
				/* update curval */
				curval+=curinc;
				/* Note: SAMPLE900COMPR_BITMASK&curval contains sample value */
				/* convert 12bit sample into 16bit WAV sample */
				wavbuf[wavpos++]=0xf0&(u_char)(curval<<4);
				wavbuf[wavpos++]=0xff&(u_char)(curval>>4);
			}
			bitpos+=j;
			bitremain-=j;
		}
	}

	return wavpos;
}

void
akai_sample900compr_setbits(u_char *buf,u_int bitpos,u_int bitnum,u_int val)
{
	u_int bytepos;
	u_char bmask;
	u_int i;

	if ((buf==NULL)||(bitnum==0)){
		return;
	}
	/* XXX no check if bitnum too large for u_int */
	/* XXX no check if bitpos+bitnum too large for buf */

	for (i=0;i<bitnum;i++,bitpos++){
		bytepos=(bitpos>>3); /* /8: 8 bits per byte */
		bmask=(1<<(7-(7&bitpos))); /* Note: upper bit first */
		if (((1<<(bitnum-1-i))&val)!=0){ /* bit set? (Note: upper bit first) */
			/* set bit */
			buf[bytepos]|=bmask;
		}else{
			/* clear bit */
			buf[bytepos]&=~bmask;
		}
	}
}

int
akai_sample900compr_wav2sample(u_char *sbuf,u_char *wavbuf,u_int samplecountpart)
{
	/* must be static for multiple calls, must be initialized */
	static u_char *upbitnumbuf=NULL;
	static u_int *upsignbuf=NULL;
	static u_short *upabsvalbuf=NULL;
	static u_int groupcount=0;

	short sval;
	short curval;
	short curinc;
	short upval;
	u_short upabsval;
	u_char upbitnum;
	u_int upsign;
	u_int bitpos;
	u_char code;
	u_int g,s,i,j;
	int ret;

	if ((sbuf==NULL)&&(wavbuf==NULL)){
		ret=0; /* no error */
		/* free buffers if still allocated */
		goto akai_sample900compr_wav2sample_freebufexit;
	}

	ret=-1; /* no success so far */

	/* convert 16bit WAV sample format into S900 compressed sample format */

	if (sbuf==NULL){
		/* first pass */

		/* free buffers if still allocated */
		if (upbitnumbuf!=NULL){
			free(upbitnumbuf);
			upbitnumbuf=NULL;
		}
		if (upsignbuf!=NULL){
			free(upsignbuf);
			upsignbuf=NULL;
		}
		if (upabsvalbuf!=NULL){
			free(upabsvalbuf);
			upabsvalbuf=NULL;
		}

		/* number of groups */
		/* Note: 2*samplecountpart+1 to encode at least one additional zero sample behind end */
		groupcount=(2*samplecountpart+1+SAMPLE900COMPR_GROUP_SAMPNUM-1)/SAMPLE900COMPR_GROUP_SAMPNUM; /* round up */

		/* allocate buffers */
		upbitnumbuf=(u_char *)malloc(groupcount*sizeof(u_char));
		if (upbitnumbuf==NULL){
			PERROR("cannot allocate upbitnumbuf");
			goto akai_sample900compr_wav2sample_freebufexit;
		}
		upsignbuf=(u_int *)malloc(groupcount*sizeof(u_int));
		if (upsignbuf==NULL){
			PERROR("cannot allocate upsignbuf");
			goto akai_sample900compr_wav2sample_freebufexit;
		}
		upabsvalbuf=(u_short *)malloc(groupcount*SAMPLE900COMPR_GROUP_SAMPNUM*sizeof(u_short));
		if (upabsvalbuf==NULL){
			PERROR("cannot allocate upabsvalbuf");
			goto akai_sample900compr_wav2sample_freebufexit;
		}

		/* gather groups */
		curval=0;
		curinc=0;
		sval=0;
		for (g=0,s=0;g<groupcount;g++){
			upbitnum=0;
			upsign=0;
			/* samples within group */
			for (i=0;i<SAMPLE900COMPR_GROUP_SAMPNUM;i++,s++){
				if (s<2*samplecountpart){
					/* get 12bit sample from 16bit WAV sample */
					sval=(((short)(char)wavbuf[s*2+1])<<4)+(((short)(u_char)wavbuf[s*2+0])>>4);
				}else{
					/* encode zero sample behind end */
					sval=0;
				}

				/* determine required upsign,upabsval */
				upsign<<=1;
				upval=sval-(curval+curinc);
				/* choose optimum interval position for min. abs. value */
				while (upval>SAMPLE900COMPR_INTERVALSIZ2){
					upval-=SAMPLE900COMPR_INTERVALSIZ;
				}
				while (upval<-SAMPLE900COMPR_INTERVALSIZ2){
					upval+=SAMPLE900COMPR_INTERVALSIZ;
				}
				/* Note: max./min. possible value for resulting upval is +/-SAMPLE900COMPR_INTERVALSIZ2 */
				/*       which requires SAMPLE900COMPR_UPBITNUM_MAX bits for upabsval */
				if (upval>=0){
					/* plus */
					/* upsign|=0; */
					upabsval=SAMPLE900COMPR_BITMASK&(u_short)upval;
					/* update curinc */
					curinc+=(short)upabsval;
				}else{
					/* minus */
					upsign|=1;
					upabsval=SAMPLE900COMPR_BITMASK&(u_short)(-upval);
					/* update curinc */
					curinc-=(short)upabsval;
				}
				upabsvalbuf[s]=upabsval;

				/* determine max. number of required bits for upabsval in group */
				/* start with previous upbitnum */
				for (j=(u_int)upbitnum;j<SAMPLE900COMPR_UPBITNUM_MAX;j++){
					if (((1<<j)&upabsval)!=0){ /* bit j set? */
						upbitnum=(u_char)(j+1);
					}
				}

				/* update curval */
				curval+=curinc;
				/* Note: SAMPLE900COMPR_BITMASK&curval contains sample value */
				if ((SAMPLE900COMPR_BITMASK&(curval-sval))!=0){
					/* XXX should not happen */
					PRINTF_ERR("%06x,%02x: error: sval=%i curval=%i\n",g,i,sval,curval);
					goto akai_sample900compr_wav2sample_freebufexit;
				}
			}
#ifdef SAMPLE900COMPR_DEBUG
			PRINTF_OUT("%06x: upbitnum=%2u upsign=%04x\n",g,(u_int)upbitnum,upsign);
#endif
			upbitnumbuf[g]=upbitnum;
			upsignbuf[g]=upsign;
		}

		/* determine number of bits required in sbuf for second pass */
		bitpos=0;
		for (g=0;g<groupcount;g++){
			/* see "encode groups" below */
			bitpos+=4;
			if (upbitnumbuf[g]!=0){
				bitpos+=SAMPLE900COMPR_GROUP_SAMPNUM*(1+((u_int)upbitnumbuf[g]));
			}
		}
		bitpos+=(7&(8-(7&bitpos))); /* number of bits missing to full byte */

		ret=(int)(bitpos>>3); /* success, return number of required bytes in sbuf for second pass */
		/* keep buffers for second pass */
		goto akai_sample900compr_wav2sample_keepbufexit;
	}else{
		/* second pass */

		if (wavbuf==NULL){
			goto akai_sample900compr_wav2sample_freebufexit;
		}

		/* check allocation of buffers */
		if ((upbitnumbuf==NULL)||(upsignbuf==NULL)||(upabsvalbuf==NULL)){
			PRINTF_ERR("error: buffers not allocated in second pass\n");
			goto akai_sample900compr_wav2sample_freebufexit;
		}

		/* XXX ignore samplecountpart in second pass */

		/* encode groups */
		bitpos=0;
		for (g=0,s=0;g<groupcount;g++){
			/* determine code nibble */
			upbitnum=upbitnumbuf[g];
			if (upbitnum==0){
				code=0x0;
			}else{
				code=SAMPLE900COMPR_UPBITNUM_NEGCODE_OFF-upbitnum;
			}
#ifdef SAMPLE900COMPR_DEBUG
			PRINTF_OUT("%08x: code=%x\n",bitpos,code);
#endif
			/* save code nibble */
			akai_sample900compr_setbits(sbuf,bitpos,4,(u_int)code);
			bitpos+=4;

			if (upbitnum==0){
				s+=SAMPLE900COMPR_GROUP_SAMPNUM; /* advance in upabsvalbuf[] */
				/* no further bits to save for this group */
			}else{
				/* save upsign */
				upsign=upsignbuf[g];
				akai_sample900compr_setbits(sbuf,bitpos,SAMPLE900COMPR_GROUP_SAMPNUM,upsign);
				bitpos+=SAMPLE900COMPR_GROUP_SAMPNUM;
				/* samples within group */
				for (i=0;i<SAMPLE900COMPR_GROUP_SAMPNUM;i++,s++){
					/* save upabsval */
					upabsval=upabsvalbuf[s];
					akai_sample900compr_setbits(sbuf,bitpos,(u_int)upbitnum,(u_int)upabsval);
					bitpos+=(u_int)upbitnum;
#ifdef SAMPLE900COMPR_DEBUG
					PRINTF_OUT("          %c%u\n",
						(((1<<(SAMPLE900COMPR_GROUP_SAMPNUM-1-i))&upsign)==0)?'+':'-',(u_int)upabsval); /* Note: upper bit first */
#endif
				}
			}
		}
		j=(7&(8-(7&bitpos))); /* number of bits missing to full byte */
		if (j!=0){
			/* zero padding to full byte */
			akai_sample900compr_setbits(sbuf,bitpos,j,0);
			bitpos+=j;
		}

		ret=(int)(bitpos>>3); /* success, return number of used bytes in sbuf */
		/* free buffers */
		goto akai_sample900compr_wav2sample_freebufexit;
	}

akai_sample900compr_wav2sample_freebufexit:
	if (upbitnumbuf!=NULL){
		free(upbitnumbuf);
		upbitnumbuf=NULL;
	}
	if (upsignbuf!=NULL){
		free(upsignbuf);
		upsignbuf=NULL;
	}
	if (upabsvalbuf!=NULL){
		free(upabsvalbuf);
		upabsvalbuf=NULL;
	}

akai_sample900compr_wav2sample_keepbufexit:
	return ret;
}



int
akai_sample900_compr2noncompr(struct file_s *fp,struct vol_s *volp)
{
	struct file_s tmpfile;
	struct akai_sample900_s s900hdr;
	u_int samplecount;
	u_int samplecountpart;
	u_int samplesizecompr;
	u_char *sbufcompr;
	u_int samplesizenoncompr;
	u_char *sbufnoncompr;
	u_int wavsamplesize;
	u_char *wavbuf;
	static char fname[AKAI_NAME_LEN_S900+3+1]; /* name (ASCII), +3 for ".S9", +1 for '\0' */
	int replaceflag;
	u_int findex;
	u_int i;
	int ret;

	if (fp==NULL){
		return -1;
	}

	if (volp==NULL){
		/* destination volume same as source volume */
		volp=fp->volp;
		replaceflag=1; /* replace file */
	}else{
		replaceflag=0; /* keep source file */
	}
	if ((volp==NULL)||(volp->partp==NULL)){
		return -1;
	}

	/* file type */
	if ((fp->type!=(u_char)AKAI_SAMPLE900_FTYPE)||(fp->osver==0)){ /* not S900 compressed sample file? */
		PRINTF_ERR("not an S900 compressed sample file\n");
		return -1;
	}
	if (fp->size<sizeof(struct akai_sample900_s)){
		PRINTF_ERR("invalid sample size\n");
		return -1;
	}

	sbufcompr=NULL; /* no sample so far */
	sbufnoncompr=NULL; /* no sample so far */
	wavbuf=NULL; /* no sample so far */
	ret=-1; /* no success so far */

	/* read header to memory */
	if (akai_read_file(0,(u_char *)&s900hdr,fp,0,sizeof(struct akai_sample900_s))<0){
		PRINTF_ERR("cannot read sample\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* number of samples */
	/* XXX should be an even number */
	samplecount=(s900hdr.slen[3]<<24)
		+(s900hdr.slen[2]<<16)
		+(s900hdr.slen[1]<<8)
		+s900hdr.slen[0];
	/* number of samples per part  */
	samplecountpart=(samplecount+1)/2; /* round up */
	/* size in bytes in S900 non-compressed sample format */
	samplesizenoncompr=3*samplecountpart;
	/* WAV size in bytes */
	wavsamplesize=2*samplecountpart*2; /* *2 for 16bit per WAV sample word */
	/* S900 compressed sample size */
	samplesizecompr=fp->size-sizeof(struct akai_sample900_s);

	/* create non-compressed sample file name */
	/* Note: use RAM name as basis (-> akai_fixramname() not needed afterwards) */
	akai2ascii_name((u_char *)s900hdr.name,fname,1); /* 1: S900 */
	strcat(fname,".S9");

	/* check if destination file already exists */
	if (akai_find_file(volp,&tmpfile,fname)==0){
		/* exists */
		PRINTF_ERR("destination file name \"%s\" already used\n",fname);
		goto akai_sample900_compr2noncompr_exit;
	}

	/* check if enough free blocks */
	/* required size of non-compressed file in blocks */
	i=(sizeof(struct akai_sample900_s)+samplesizenoncompr+volp->partp->blksize-1)/volp->partp->blksize;
	if (replaceflag){
		if (samplesizecompr<samplesizenoncompr){
			/* subtract size of existing compressed file in blocks */
			i-=(sizeof(struct akai_sample900_s)+samplesizecompr+volp->partp->blksize-1)/volp->partp->blksize;
		}else{
			i=0;
		}
	}
	if (volp->partp->bfree<i){
		/* not enough space left */
		PRINTF_ERR("not enough space left for destination file \"%s\"\n",fname);
		goto akai_sample900_compr2noncompr_exit;
	}

	/* allocate compressed sample buffer */
	sbufcompr=(u_char *)malloc(samplesizecompr);
	if (sbufcompr==NULL){
		PRINTF_ERR("cannot allocate compressed sample buffer\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* allocate non-compressed sample buffer */
	sbufnoncompr=(u_char *)malloc(samplesizenoncompr);
	if (sbufnoncompr==NULL){
		PRINTF_ERR("cannot allocate non-compressed sample buffer\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* allocate WAV sample buffer */
	wavbuf=(u_char *)malloc(wavsamplesize);
	if (wavbuf==NULL){
		PRINTF_ERR("cannot allocate WAV buffer\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* read compressed sample to memory */
	if (akai_read_file(0,sbufcompr,fp,sizeof(struct akai_sample900_s),sizeof(struct akai_sample900_s)+samplesizecompr)<0){
		PRINTF_ERR("cannot read sample\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* convert S900 compressed sample format into 16bit WAV sample format */
	i=akai_sample900compr_sample2wav(sbufcompr,wavbuf,samplesizecompr,wavsamplesize);
	if (i<wavsamplesize){
		PRINTF_ERR("warning: incomplete sample data\n");
		/* zero padding */
		bzero(wavbuf+i,wavsamplesize-i);
	}

	/* convert 16bit WAV sample format into S900 non-compressed sample format */
	akai_sample900noncompr_wav2sample(sbufnoncompr,wavbuf,samplecountpart);

	if (replaceflag){
		/* keep file index */
		findex=fp->index;
		/* delete source file */
		if (akai_delete_file(fp)<0){
			PRINTF_ERR("cannot overwrite existing file\n");
			goto akai_sample900_compr2noncompr_exit;
		}
	}else{
		findex=AKAI_CREATE_FILE_NOINDEX;
	}
	/* create file */
	/* Note: akai_create_file() will correct osver if necessary */
	if (akai_create_file(volp,&tmpfile,
						 sizeof(struct akai_sample900_s)+samplesizenoncompr,
						 findex,
						 fname,
						 0, /* non-compressed */
						 NULL)<0){
		PRINTF_ERR("cannot create file\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* write sample header */
	if (akai_write_file(0,(u_char *)&s900hdr,&tmpfile,0,sizeof(struct akai_sample900_s))<0){
		PRINTF_ERR("cannot write sample header\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	/* write non-compressed sample */
	if (akai_write_file(0,sbufnoncompr,&tmpfile,sizeof(struct akai_sample900_s),sizeof(struct akai_sample900_s)+samplesizenoncompr)<0){
		PRINTF_ERR("cannot write sample\n");
		goto akai_sample900_compr2noncompr_exit;
	}

	ret=0; /* success */

akai_sample900_compr2noncompr_exit:
	if (sbufcompr!=NULL){
		free(sbufcompr);
	}
	if (sbufnoncompr!=NULL){
		free(sbufnoncompr);
	}
	if (wavbuf!=NULL){
		free(wavbuf);
	}
	return ret;
}

int
akai_sample900_noncompr2compr(struct file_s *fp,struct vol_s *volp)
{
	struct file_s tmpfile;
	struct akai_sample900_s s900hdr;
	u_int samplecount;
	u_int samplecountpart;
	u_int samplesizecompr;
	u_char *sbufcompr;
	u_int samplesizenoncompr;
	u_char *sbufnoncompr;
	u_int wavsamplesize;
	u_char *wavbuf;
	static char fname[AKAI_NAME_LEN_S900+4+1]; /* name (ASCII), +4 for ".S9C", +1 for '\0' */
	int replaceflag;
	u_int findex;
	u_int osver;
	u_int i;
	int r;
	int ret;

	if (fp==NULL){
		return -1;
	}

	if (volp==NULL){
		/* destination volume same as source volume */
		volp=fp->volp;
		replaceflag=1; /* replace file */
	}else{
		replaceflag=0; /* keep source file */
	}
	if ((volp==NULL)||(volp->partp==NULL)){
		return -1;
	}

	/* file type */
	if ((fp->type!=(u_char)AKAI_SAMPLE900_FTYPE)||(fp->osver!=0)){ /* not S900 non-compressed sample file? */
		PRINTF_ERR("not an S900 non-compressed sample file\n");
		return -1;
	}
	if (fp->size<sizeof(struct akai_sample900_s)){
		PRINTF_ERR("invalid sample size\n");
		return -1;
	}

	sbufcompr=NULL; /* no sample so far */
	sbufnoncompr=NULL; /* no sample so far */
	wavbuf=NULL; /* no sample so far */
	ret=-1; /* no success so far */

	/* read header to memory */
	if (akai_read_file(0,(u_char *)&s900hdr,fp,0,sizeof(struct akai_sample900_s))<0){
		PRINTF_ERR("cannot read sample\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* number of samples */
	/* XXX should be an even number */
	samplecount=(s900hdr.slen[3]<<24)
		+(s900hdr.slen[2]<<16)
		+(s900hdr.slen[1]<<8)
		+s900hdr.slen[0];
	/* number of samples per part  */
	samplecountpart=(samplecount+1)/2; /* round up */
	/* size in bytes in S900 non-compressed sample format */
	samplesizenoncompr=3*samplecountpart;
	/* WAV size in bytes */
	wavsamplesize=2*samplecountpart*2; /* *2 for 16bit per WAV sample word */

	/* allocate non-compressed sample buffer */
	sbufnoncompr=(u_char *)malloc(samplesizenoncompr);
	if (sbufnoncompr==NULL){
		PRINTF_ERR("cannot allocate non-compressed sample buffer\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* allocate WAV sample buffer */
	wavbuf=(u_char *)malloc(wavsamplesize);
	if (wavbuf==NULL){
		PRINTF_ERR("cannot allocate WAV buffer\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* read non-compressed sample to memory */
	if (akai_read_file(0,sbufnoncompr,fp,sizeof(struct akai_sample900_s),sizeof(struct akai_sample900_s)+samplesizenoncompr)<0){
		PRINTF_ERR("cannot read sample\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* convert S900 non-compressed sample format into 16bit WAV sample format */
	akai_sample900noncompr_sample2wav(sbufnoncompr,wavbuf,samplecountpart);

	/* first pass to convert 16bit WAV sample format into S900 compressed sample format */
	/* Note: first pass of akai_sample900compr_wav2sample() requires WAV sample to be loaded to wavbuf */
	r=akai_sample900compr_wav2sample(NULL,wavbuf,samplecountpart); /* NULL: first pass */
	if (r<0){
		goto akai_sample900_noncompr2compr_exit;
	}
	/* S900 compressed sample size in bytes */
	samplesizecompr=(u_int)r;

	/* create compressed sample file name */
	/* Note: use RAM name as basis (-> akai_fixramname() not needed afterwards) */
	akai2ascii_name((u_char *)s900hdr.name,fname,1); /* 1: S900 */
	strcat(fname,".S9C");

	/* check if destination file already exists */
	if (akai_find_file(volp,&tmpfile,fname)==0){
		/* exists */
		PRINTF_ERR("destination file name \"%s\" already used\n",fname);
		goto akai_sample900_noncompr2compr_exit;
	}

	/* check if enough free blocks */
	/* required size of compressed file in blocks */
	i=(sizeof(struct akai_sample900_s)+samplesizecompr+volp->partp->blksize-1)/volp->partp->blksize;
	if (replaceflag){
		if (samplesizenoncompr<samplesizecompr){
			/* subtract size of existing non-compressed file in blocks */
			i-=(sizeof(struct akai_sample900_s)+samplesizenoncompr+volp->partp->blksize-1)/volp->partp->blksize;
		}else{
			i=0;
		}
	}
	if (volp->partp->bfree<i){
		/* not enough space left */
		PRINTF_ERR("not enough space left for destination file \"%s\"\n",fname);
		goto akai_sample900_noncompr2compr_exit;
	}

	/* allocate compressed sample buffer */
	sbufcompr=(u_char *)malloc(samplesizecompr);
	if (sbufcompr==NULL){
		PRINTF_ERR("cannot allocate compressed sample buffer\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* second pass to convert 16bit WAV sample format into S900 compressed sample format */
	if (akai_sample900compr_wav2sample(sbufcompr,wavbuf,samplecountpart)<0){ /* sbufcompr!=NULL: second pass */
		goto akai_sample900_noncompr2compr_exit;
	}

	if (replaceflag){
		/* keep file index */
		findex=fp->index;
		/* delete source file */
		if (akai_delete_file(fp)<0){
			PRINTF_ERR("cannot overwrite existing file\n");
			goto akai_sample900_noncompr2compr_exit;
		}
	}else{
		findex=AKAI_CREATE_FILE_NOINDEX;
	}
	/* osver for S900 compressed sample file: number of un-compressed floppy blocks */
	/* Note: without sample header */
	osver=(samplesizenoncompr+AKAI_FL_BLOCKSIZE-1)/AKAI_FL_BLOCKSIZE; /* round up */
	if (osver==0){ /* unsuitable osver? */
		osver=1; /* XXX non zero */
	}
	/* create file */
	/* Note: akai_create_file() will correct osver if necessary */
	if (akai_create_file(volp,&tmpfile,
						 sizeof(struct akai_sample900_s)+samplesizecompr,
						 findex,
						 fname,
						 osver,
						 NULL)<0){
		PRINTF_ERR("cannot create file\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* write sample header */
	if (akai_write_file(0,(u_char *)&s900hdr,&tmpfile,0,sizeof(struct akai_sample900_s))<0){
		PRINTF_ERR("cannot write sample header\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	/* write compressed sample */
	if (akai_write_file(0,sbufcompr,&tmpfile,sizeof(struct akai_sample900_s),sizeof(struct akai_sample900_s)+samplesizecompr)<0){
		PRINTF_ERR("cannot write sample\n");
		goto akai_sample900_noncompr2compr_exit;
	}

	ret=0; /* success */

akai_sample900_noncompr2compr_exit:
	if (sbufcompr!=NULL){
		free(sbufcompr);
	}
	if (sbufnoncompr!=NULL){
		free(sbufnoncompr);
	}
	if (wavbuf!=NULL){
		free(wavbuf);
	}
	akai_sample900compr_wav2sample(NULL,NULL,0); /* NULL,NULL: free buffers if still allocated */
	return ret;
}



int
akai_sample2wav(struct file_s *fp,int wavfd,u_int *sizep,char **wavnamep,int what)
{
	/* Note: static for multiple calls with different what */
	static struct akai_sample3000_s s3000hdr;
	static struct akai_sample900_s *s900hdrp;
	static u_int hdrsize;
	static u_int samplecount;
	static u_int samplecountpart;
	static u_int samplesize;
	static u_int samplerate;
	static u_char *sbuf;
	static u_int wavsamplesize;
	static u_char *wavbuf;
	static int ret;
	static char wavname[AKAI_NAME_LEN+4+1]; /* name (ASCII), +4 for ".<type>", +1 for '\0' */
	static u_int nlen;
	static u_int i;

	if (fp==NULL){
		return -1;
	}

	sbuf=NULL; /* no sample so far */
	wavbuf=NULL; /* no sample so far */
	ret=-1; /* no success so far */

	if (what&SAMPLE2WAV_CHECK){

		/* file type */
		if (fp->type==(u_char)AKAI_SAMPLE900_FTYPE){
			/* S900 sample */
			hdrsize=sizeof(struct akai_sample900_s);
		}else if (fp->type==(u_char)AKAI_SAMPLE1000_FTYPE){
			/* S1000 sample */
			hdrsize=sizeof(struct akai_sample1000_s);
		}else if (fp->type==(u_char)AKAI_SAMPLE3000_FTYPE){
			/* S3000 sample */
			hdrsize=sizeof(struct akai_sample3000_s);
		}else{
			/* unknown or unsupported */
			return 1; /* no error */
		}

		/* read header to memory */
		/* Note: use S3000 header as buffer for S900 header */
		if (akai_read_file(0,(u_char *)&s3000hdr,fp,0,hdrsize)<0){
			PRINTF_ERR("cannot read sample\n");
			goto akai_sample2wav_exit;
		}

		/* parse header */
		if (fp->type==(u_char)AKAI_SAMPLE900_FTYPE){
			/* S900 sample */
			s900hdrp=(struct akai_sample900_s *)&s3000hdr;
			/* number of samples */
			/* XXX should be an even number */
			samplecount=(s900hdrp->slen[3]<<24)
				+(s900hdrp->slen[2]<<16)
				+(s900hdrp->slen[1]<<8)
				+s900hdrp->slen[0];
			/* number of samples per part  */
			samplecountpart=(samplecount+1)/2; /* round up */
			samplecount=2*samplecountpart; /* XXX correct samplecount */
			if (fp->osver==0){
				/* S900 non-compressed sample format */
				/* size in bytes */
				samplesize=3*samplecountpart;
			}else{
				/* S900 compressed sample format */
				/* size in bytes */
				if (fp->size<hdrsize){
					PRINTF_ERR("invalid sample size\n");
					goto akai_sample2wav_exit;
				}
				samplesize=fp->size-hdrsize;
			}
			samplerate=(s900hdrp->srate[1]<<8)
				+s900hdrp->srate[0];
		}else{
			/* S1000/S3000 sample */
			/* Note: S1000 header is contained within S3000 header */
			/* number of samples */
			samplecount=(s3000hdr.s1000.slen[3]<<24)
				+(s3000hdr.s1000.slen[2]<<16)
				+(s3000hdr.s1000.slen[1]<<8)
				+s3000hdr.s1000.slen[0];
			/* size in bytes */
			samplesize=samplecount*2; /* *2 for 16bit per sample word */
			samplecountpart=0;
			samplerate=(s3000hdr.s1000.srate[1]<<8)
				+s3000hdr.s1000.srate[0];
		}
		/* size in bytes */
		wavsamplesize=samplecount*2; /* *2 for 16bit per WAV sample word */
		/* Note: wavsamplesize==0 is allowed here */

#ifdef DEBUG
		PRINTF_OUT("type:        %15i\n",fp->type);
		PRINTF_OUT("samplecount: %15u\n",samplecount);
		PRINTF_OUT("samplesize:  %15u bytes\n",samplesize);
		PRINTF_OUT("samplerate:  %15u Hz\n",samplerate);
#endif

		/* check size */
		if (hdrsize+samplesize>fp->size){
			PRINTF_ERR("invalid sample size\n");
			goto akai_sample2wav_exit;
		}

		if (sizep!=NULL){
			/* WAV file size */
			*sizep=WAV_HEAD_SIZE+wavsamplesize;
#ifndef WAV_AKAIHEAD_DISABLE
			*sizep+=sizeof(struct wav_chunkhead_s)+hdrsize; /* sample header chunk (see below) */
#endif
		}

		/* create WAV name */
		if ((fp->volp!=NULL)&&(fp->index<fp->volp->fimax)){
			akai2ascii_name(fp->volp->file[fp->index].name,wavname,fp->volp->type==AKAI_VOL_TYPE_S900);
			nlen=(u_int)strlen(wavname);
		}else{
			nlen=0;
		}
		bcopy(".wav",wavname+nlen,5);

		if (wavnamep!=NULL){
			/* pointer to name (wavname must be static!) */
			*wavnamep=wavname;
		}
	}

	if (what&SAMPLE2WAV_EXPORT){

		if (wavsamplesize>0){
			/* allocate WAV sample buffer */
			wavbuf=(u_char *)malloc(wavsamplesize);
			if (wavbuf==NULL){
				PRINTF_ERR("cannot allocate WAV buffer\n");
				goto akai_sample2wav_exit;
			}

			if (fp->type==(u_char)AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
				/* allocate sample buffer */
				sbuf=(u_char *)malloc(samplesize);
				if (sbuf==NULL){
					PRINTF_ERR("cannot allocate sample buffer\n");
					goto akai_sample2wav_exit;
				}
			}else{
				/* Note: no sample format conversion necessary for S1000/S3000 */
				sbuf=wavbuf;
			}

			/* read sample to memory */
			if (akai_read_file(0,sbuf,fp,hdrsize,hdrsize+samplesize)<0){
				PRINTF_ERR("cannot read sample\n");
				goto akai_sample2wav_exit;
			}

			if (fp->type==(u_char)AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
				if (fp->osver==0){
					/* S900 non-compressed sample format */
					/* convert S900 non-compressed sample format into 16bit WAV sample format */
					akai_sample900noncompr_sample2wav(sbuf,wavbuf,samplecountpart);
				}else{
					/* S900 compressed sample format */
					/* convert S900 compressed sample format into 16bit WAV sample format */
					i=akai_sample900compr_sample2wav(sbuf,wavbuf,samplesize,wavsamplesize);
					if (i<wavsamplesize){
						PRINTF_ERR("warning: incomplete sample data\n");
						/* zero padding */
						bzero(wavbuf+i,wavsamplesize-i);
					}
				}
			}
			/* Note: no sample format conversion necessary for S1000/S3000 */
		}

		if (what&SAMPLE2WAV_CREATE){
			/* create WAV file */
			if ((wavfd=OPEN(wavname,O_RDWR|O_CREAT|O_TRUNC|O_BINARY,0666))<0){
				PERROR("create WAV");
				goto akai_sample2wav_exit;
			}
		}

		/* write WAV header */
		if (wav_write_head(wavfd,
						   wavsamplesize,1,samplerate,16, /* 1: mono, 16: 16bit */
#ifndef WAV_AKAIHEAD_DISABLE
						   sizeof(struct wav_chunkhead_s)+hdrsize /* sample header chunk (see below) */
#else
						   0
#endif
						   )<0){
			PRINTF_ERR("cannot write WAV header\n");
			goto akai_sample2wav_exit;
		}

		if (wavsamplesize>0){
			/* write WAV sample to WAV file */
			if (WRITE(wavfd,wavbuf,wavsamplesize)!=(int)wavsamplesize){
				PRINTF_ERR("cannot write WAV samples\n");
				goto akai_sample2wav_exit;
			}
		}

#ifndef WAV_AKAIHEAD_DISABLE
		{
			struct wav_chunkhead_s wavchunkhead;

			/* create sample header chunk */
			if (fp->type==(u_char)AKAI_SAMPLE900_FTYPE){
				/* S900 sample */
				bcopy(WAV_CHUNKHEAD_AKAIS900SAMPLEHEADSTR,wavchunkhead.typestr,4);
			}else if (fp->type==(u_char)AKAI_SAMPLE1000_FTYPE){
				/* S1000 sample */
				bcopy(WAV_CHUNKHEAD_AKAIS1000SAMPLEHEADSTR,wavchunkhead.typestr,4);
			}else{
				/* S3000 sample */
				bcopy(WAV_CHUNKHEAD_AKAIS3000SAMPLEHEADSTR,wavchunkhead.typestr,4);
			}
			wavchunkhead.csize[0]=0xff&hdrsize;
			wavchunkhead.csize[1]=0xff&(hdrsize>>8);
			wavchunkhead.csize[2]=0xff&(hdrsize>>16);
			wavchunkhead.csize[3]=0xff&(hdrsize>>24);
			/* write WAV chunk header */
			if (WRITE(wavfd,(u_char *)&wavchunkhead,sizeof(struct wav_chunkhead_s))!=(int)sizeof(struct wav_chunkhead_s)){
				PRINTF_ERR("cannot write WAV chunk\n");
				goto akai_sample2wav_exit;
			}
			/* write sample header */
			/* Note: use S3000 header as buffer for S900 header */
			/* Note: S1000 header is contained within S3000 header */
			if (WRITE(wavfd,(u_char *)&s3000hdr,hdrsize)!=(int)hdrsize){
				PRINTF_ERR("cannot write WAV chunk\n");
				goto akai_sample2wav_exit;
			}
		}
#endif
#if 1
		PRINTF_OUT("sample exported to WAV\n");
#endif
	}

	ret=0; /* success */

akai_sample2wav_exit:
	if (wavbuf!=NULL){
		free(wavbuf);
	}
	if (fp->type==(u_char)AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
		if (sbuf!=NULL){
			free(sbuf);
		}
	}
	if (what&SAMPLE2WAV_CREATE){
		if (wavfd>=0){
			CLOSE(wavfd);
		}
	}
	return ret;
}



int
akai_wav2sample(int wavfd,char *wavname,struct vol_s *volp,u_int findex,
				u_int type,int s9cflag,u_int osver,u_char *tagp,
				u_int *bcountp,int what)
{
	/* Note: static for multiple calls with different what */
	struct file_s tmpfile;
	struct akai_sample900_s s900hdr;
	struct akai_sample3000_s s3000hdr;
	static u_int hdrsize;
	static u_int samplerate;
	static u_int samplecount;
	static u_int samplecountpart;
	static u_int samplesize;
	static u_char *sbuf;
	static u_int wavchnr;
	static u_int wavbitnr;
	static int wavmono16flag;
	static u_int wavsamplesize;
	static u_int wavsamplecount;
	static u_int wavch;
	static u_char *wavbuf16;
	static u_int wavbuf16size;
	static u_char *wavbuf;
	static char *errstrp;
	static u_int nlen,nlenmax;
	static char *tname;
	static char fname[AKAI_NAME_LEN+4+1]; /* name (ASCII), +4 for ".<type>", +1 for '\0' */
	static char sname[AKAI_NAME_LEN+1]; /* +1 for '\0' */
	static int r;
	static u_int bcount;
#ifndef WAV_AKAIHEAD_DISABLE
	static u_int extrasize;
	static int wavakaiheadfound;
#endif
	static u_int i;
	static int ret;

	if (bcountp!=NULL){
		*bcountp=0; /* no bytes read yet */
	}

	if (volp==NULL){
		return -1;
	}

	/* check WAV name */
	if (wavname==NULL){
		return -1;
	}
	nlen=(u_int)strlen(wavname);
	if ((nlen<4)
#if 0 /* XXX assume that wavname has been checked for correct suffix */
		||(strncasecmp(wavname+nlen-4,".wav",4)!=0)
#endif
		){
		/* not a WAV file */
		return 1; /* no error */
	}
	nlen-=4; /* remove suffix */

	/* sample file type */
	if (type==AKAI_FTYPE_FREE){ /* invalid file type? */
		/* derive sample file type from volume type/file osver */
		if (volp->type==AKAI_VOL_TYPE_S900){
			/* S900 sample */
			type=AKAI_SAMPLE900_FTYPE;
			/* Note: ignore given osver, osver will be overwritten below */
		}else{
#if 1
			if (osver==AKAI_OSVER_S900VOL){
				/* S900 sample */
				type=AKAI_SAMPLE900_FTYPE;
			}else if (osver<=AKAI_OSVER_S1100MAX){
				/* S1000 sample */
				type=AKAI_SAMPLE1000_FTYPE;
			}else{
				/* S3000 sample */
				type=AKAI_SAMPLE3000_FTYPE;
			}
#else
			if (volp->type==AKAI_VOL_TYPE_S1000){
				/* S1000 sample */
				type=AKAI_SAMPLE1000_FTYPE;
			}else if ((volp->type==AKAI_VOL_TYPE_S3000)||(volp->type==AKAI_VOL_TYPE_CD3000)){
				/* S3000 sample */
				type=AKAI_SAMPLE3000_FTYPE;
			}else{
				return -1;
			}
#endif
		}
	}
	if (type==AKAI_SAMPLE900_FTYPE){
		/* S900 sample */
		hdrsize=sizeof(struct akai_sample900_s);
		if (s9cflag){
			/* S900 compressed sample format */
			tname=".S9C";
		}else{
			/* S900 non-compressed sample format */
			tname=".S9";
		}
	}else if (type==AKAI_SAMPLE1000_FTYPE){
		/* S1000 sample */
		hdrsize=sizeof(struct akai_sample1000_s);
		tname=".S1";
	}else if (type==AKAI_SAMPLE3000_FTYPE){
		/* S3000 sample */
		hdrsize=sizeof(struct akai_sample3000_s);
		tname=".S3";
	}else{
		return -1;
	}

	wavmono16flag=1; /* XXX unknown yet */
	wavbuf16=NULL; /* not allocated yet */
	wavbuf=NULL; /* not allocated yet */
	sbuf=NULL; /* not allocated yet */
	ret=-1; /* no success so far */
	bcount=0; /* no bytes read yet */

	if (what&WAV2SAMPLE_OPEN){
		/* open external WAV file */
		if ((wavfd=akai_openreadonly_extfile(wavname))<0){
			PERROR("open WAV");
			goto akai_wav2sample_exit;
		}
	}

	/* read and parse WAV header */
	if (wav_read_head(wavfd,&bcount,
							&wavsamplesize,&wavchnr,&samplerate,&wavbitnr,
#ifndef WAV_AKAIHEAD_DISABLE
							&extrasize,
#else
							NULL,
#endif
							&errstrp)<0){
		if (errstrp!=NULL){
			PRINTF_ERR("%s\n",errstrp);
		}
		/* error, don't know how many bytes read */
		goto akai_wav2sample_exit;
	}

	/* check parameters */
	if ((wavchnr!=1)&&(wavchnr!=2)){
		PRINTF_ERR("invalid number of channels, must be mono or stereo\n");
		/* unknown or unsupported */
		ret=1; /* no error */
		goto akai_wav2sample_exit;
	}
	if ((wavbitnr!=8)&&(wavbitnr!=16)&&(wavbitnr!=24)&&(wavbitnr!=32)){
		PRINTF_ERR("invalid WAV format, must be 8bit or 16bit or 24bit or 32bit PCM\n");
		/* unknown or unsupported */
		ret=1; /* no error */
		goto akai_wav2sample_exit;
	}
	if ((wavchnr==1)&&(wavbitnr==16)){
		/* WAV file is mono 16bit */
		wavmono16flag=1;
	}else{
		wavmono16flag=0;
	}

	/* number of samples in WAV file (per channel) */
	/* Note: can be an odd number */
	wavsamplecount=wavsamplesize/(wavchnr*(wavbitnr/8));
#if 0
	if (wavsamplecount==0){
		PRINTF_ERR("no samples in WAV file\n");
		ret=1; /* no error */
		goto akai_wav2sample_exit;
	}
#endif /* else: Note: wavesamplecount==0 is allowed here */

	if (wavsamplecount>0){
		/* determine number of samples and sample size */
		if (type==AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
			/* S900 sample */
			/* number of samples per part  */
			samplecountpart=(wavsamplecount+1)/2; /* round up */
			samplecount=2*samplecountpart; /* samplecount must be an even number */
			/* set samplesize for S900 non-compressed sample format */
			/* Note: for S900 compressed sample format, samplesize will be overwritten below */
			/* size in bytes */
			samplesize=3*samplecountpart;
		}else{
			/* S1000/S3000 sample */
			samplecountpart=0;
			samplecount=wavsamplecount;
			/* size in bytes */
			samplesize=samplecount*2; /* *2 for 16bit per sample word */
		}

		/* allocate WAV 16bit sample buffer */
		wavbuf16size=2*samplecount; /* *2 for 16bit per sample word in wavbuf16 */
		if (wavmono16flag&&(wavbuf16size<wavsamplesize)){
			/* Note: need at least wavsamplesize for wavbuf==wavbuf16 below */
			wavbuf16size=wavsamplesize;
		}
		wavbuf16=(u_char *)malloc(wavbuf16size);
		if (wavbuf16==NULL){
			PRINTF_ERR("cannot allocate WAV 16bit buffer\n");
			goto akai_wav2sample_exit;
		}
		if (type!=AKAI_SAMPLE900_FTYPE){ /* S1000/S3000 sample? */
			/* use wavbuf16 as sample buffer */
			sbuf=wavbuf16;
		}
		/* Note: for S900 sample, sbuf will be allocated below */

		/* WAV sample buffer */
		if (wavmono16flag){ /* WAV file is mono 16bit? */
			/* use wavbuf16 as WAV sample buffer */
			wavbuf=wavbuf16;
		}else{
			/* allocate WAV sample buffer */
			wavbuf=(u_char *)malloc(wavsamplesize);
			if (wavbuf==NULL){
				PRINTF_ERR("cannot allocate WAV buffer\n");
				goto akai_wav2sample_exit;
			}
		}

		/* read WAV sample to memory */
		if (READ(wavfd,wavbuf,wavsamplesize)!=(int)wavsamplesize){
			PRINTF_ERR("cannot read sample\n");
			goto akai_wav2sample_exit;
		}
		bcount+=wavsamplesize;

		/* zero padding if necessary */
		/* Note: must be after READ to wavbuf for case that wavbuf==wavbuf16 */
		if (samplecount>wavsamplecount){
			/* 16bit sample word */
			wavbuf16[wavsamplecount*2+1]=0x00;
			wavbuf16[wavsamplecount*2+0]=0x00;
		}
	}else{
		samplecountpart=0;
		samplecount=0;
		samplesize=0;
	}

	/* correct osver if necessary */
	if (type==AKAI_SAMPLE900_FTYPE){
		/* S900 sample */
		/* Note: here, samplesize==non-compressed sample size in bytes */
		if (s9cflag){
			/* S900 compressed sample format */
			/* set osver=number of un-compressed floppy blocks */
			/* Note: without sample header */
			osver=(samplesize+AKAI_FL_BLOCKSIZE-1)/AKAI_FL_BLOCKSIZE; /* round up */
			if (osver==0){ /* unsuitable osver? */
				osver=1; /* XXX non zero */
			}
		}else{
			/* S900 non-compressed sample format */
			osver=0;
		}
	}else if (type==AKAI_SAMPLE1000_FTYPE){
		/* S1000 sample */
		if ((osver==AKAI_OSVER_S900VOL)||(osver>AKAI_OSVER_S1100MAX)){
			osver=AKAI_OSVER_S1000MAX; /* XXX */
		}
	}else{
		/* S3000 sample */
		if ((osver==AKAI_OSVER_S900VOL)||(osver>AKAI_OSVER_S3000MAX)){
			osver=AKAI_OSVER_S3000MAX; /* XXX */
		}
	}

	/* sample name */
	if ((type==AKAI_SAMPLE900_FTYPE)||(volp->type==AKAI_VOL_TYPE_S900)){ /* S900 sample or S900 volume? */
		nlenmax=AKAI_NAME_LEN_S900;
	}else{
		nlenmax=AKAI_NAME_LEN;
	}
	if (wavchnr==1){
		/* mono */
		if (nlen>nlenmax){
			nlen=nlenmax;
		}
		bcopy(wavname,sname,nlen);
		sname[nlen]='\0';
	}else{
		/* stereo */
		if (nlen>nlenmax-2){
			nlen=nlenmax-2;
		}
		bcopy(wavname,sname,nlen);
		for (i=nlen;i<nlenmax-2;i++){
			sname[i]=' ';
		}
		sname[i++]='-';
		sname[i++]='L'; /* left channel first */
		sname[i]='\0';
	}
	sprintf(fname,"%s%s",sname,tname);
	/* Note: if stereo, channel letter in sname,fname will be updated below */

	/* sample header */
#ifndef WAV_AKAIHEAD_DISABLE
	/* check for sample header chunk in WAV file */
	{
		u_int wavakaiheadsearchtype;
		int wavakaiheadtype;
		u_int wavakaiheadsize;
		u_int bc;

		/* matching type */
		if (type==AKAI_SAMPLE900_FTYPE){
			wavakaiheadsearchtype=WAV_AKAIHEADTYPE_SAMPLE900;
		}else if (type==AKAI_SAMPLE1000_FTYPE){
			wavakaiheadsearchtype=WAV_AKAIHEADTYPE_SAMPLE1000;
		}else{
			wavakaiheadsearchtype=WAV_AKAIHEADTYPE_SAMPLE3000;
		}

		wavakaiheadtype=wav_find_akaihead(wavfd,&bc,&wavakaiheadsize,extrasize,wavakaiheadsearchtype);
		if (wavakaiheadtype<0){
			goto akai_wav2sample_exit;
		}
		bcount+=bc;

		if ((wavakaiheadtype==(int)wavakaiheadsearchtype)&&(wavakaiheadsize==hdrsize)){
			/* found matching sample header chunk */
			wavakaiheadfound=1;
		}else{
			wavakaiheadfound=0;
		}
	}
#endif
	if (type==AKAI_SAMPLE900_FTYPE){
		/* S900 sample */
#ifndef WAV_AKAIHEAD_DISABLE
		if (wavakaiheadfound){
			/* read S900 sample header */
			if (READ(wavfd,(u_char *)&s900hdr,hdrsize)!=(int)hdrsize){
				PRINTF_ERR("cannot read sample header\n");
				goto akai_wav2sample_exit;
			}
			bcount+=hdrsize;
#if 1
			PRINTF_OUT("S900 sample header imported from WAV\n");
#endif
		}else
#endif
		{
			/* create S900 sample header */
			bzero(&s900hdr,sizeof(struct akai_sample900_s));

			s900hdr.srate[1]=0xff&(samplerate>>8);
			s900hdr.srate[0]=0xff&samplerate;

			s900hdr.npitch[1]=0xff&(SAMPLE900_NPITCH_DEF>>8); /* XXX */
			s900hdr.npitch[0]=0xff&SAMPLE900_NPITCH_DEF; /* XXX */

			s900hdr.pmode=SAMPLE900_PMODE_ONESHOT; /* XXX */

			/* Note: use wavsamplecount for end */
			s900hdr.end[3]=0xff&(wavsamplecount>>24);
			s900hdr.end[2]=0xff&(wavsamplecount>>16);
			s900hdr.end[1]=0xff&(wavsamplecount>>8);
			s900hdr.end[0]=0xff&wavsamplecount;

			/* Note: use wavsamplecount for llen */
			s900hdr.llen[3]=0xff&(wavsamplecount>>24);
			s900hdr.llen[2]=0xff&(wavsamplecount>>16);
			s900hdr.llen[1]=0xff&(wavsamplecount>>8);
			s900hdr.llen[0]=0xff&wavsamplecount;

			s900hdr.dir=SAMPLE900_DIR_NORM; /* XXX */
		}

		/* set correct slen */
		s900hdr.slen[3]=0xff&(samplecount>>24);
		s900hdr.slen[2]=0xff&(samplecount>>16);
		s900hdr.slen[1]=0xff&(samplecount>>8);
		s900hdr.slen[0]=0xff&samplecount;
	}else{
		/* S1000/S3000 sample */
#ifndef WAV_AKAIHEAD_DISABLE
		if (wavakaiheadfound){
			/* read S1000/S3000 sample header */
			/* Note: S1000 header is contained within S3000 header */
			if (READ(wavfd,(u_char *)&s3000hdr,hdrsize)!=(int)hdrsize){
				PRINTF_ERR("cannot read sample header\n");
				goto akai_wav2sample_exit;
			}
			bcount+=hdrsize;
#if 1
			if (type==AKAI_SAMPLE1000_FTYPE){
				PRINTF_OUT("S1000 sample header imported from WAV\n");
			}else{
				PRINTF_OUT("S3000 sample header imported from WAV\n");
			}
#endif
		}else
#endif
		{
			/* create S3000 sample header */
			/* Note: S1000 header is contained within S3000 header */
			bzero(&s3000hdr,sizeof(struct akai_sample3000_s));

			s3000hdr.s1000.blockid=SAMPLE1000_BLOCKID;
			s3000hdr.s1000.bandw=SAMPLE1000_BANDW_20KHZ; /* XXX */
			s3000hdr.s1000.rkey=60; /* XXX */
			s3000hdr.s1000.dummy1=0x80; /* XXX */

			/* Note: use wavsamplecount-1 for end */
			s3000hdr.s1000.end[3]=0xff&((wavsamplecount-1)>>24);
			s3000hdr.s1000.end[2]=0xff&((wavsamplecount-1)>>16);
			s3000hdr.s1000.end[1]=0xff&((wavsamplecount-1)>>8);
			s3000hdr.s1000.end[0]=0xff&(wavsamplecount-1);

			s3000hdr.s1000.stpaira[1]=0xff&(AKAI_SAMPLE1000_STPAIRA_NONE>>8);
			s3000hdr.s1000.stpaira[0]=0xff&AKAI_SAMPLE1000_STPAIRA_NONE;

			s3000hdr.s1000.srate[1]=0xff&(samplerate>>8);
			s3000hdr.s1000.srate[0]=0xff&samplerate;
		}

		/* set correct slen */
		s3000hdr.s1000.slen[3]=0xff&(samplecount>>24);
		s3000hdr.s1000.slen[2]=0xff&(samplecount>>16);
		s3000hdr.s1000.slen[1]=0xff&(samplecount>>8);
		s3000hdr.s1000.slen[0]=0xff&samplecount;
	}
	/* Note: RAM name of sample in sample header will be set below */

	/* process WAV channels */
	wavch=0;
	for (;;){
		if (wavsamplecount>0){
			if (!wavmono16flag){ /* WAV file is not mono 16bit? */
				/* de-interleave WAV channels and convert sample format */
				if (wavbitnr==8){
					/* convert 8bit WAV sample format into 16bit WAV sample format */
					for (i=0;i<wavsamplecount;i++){
						wavbuf16[i*2+1]=0x80^wavbuf[wavchnr*i+wavch]; /* toggle sign bit */
						wavbuf16[i*2+0]=0x00;
					}
				}else if (wavbitnr==16){
					/* keep 16bit WAV sample format */
					for (i=0;i<wavsamplecount;i++){
						wavbuf16[i*2+0]=wavbuf[(wavchnr*i+wavch)*2+0];
						wavbuf16[i*2+1]=wavbuf[(wavchnr*i+wavch)*2+1];
					}
				}else if (wavbitnr==24){
					/* convert 24bit WAV sample format into 16bit WAV sample format */
					for (i=0;i<wavsamplecount;i++){
						/* Note: copy upper 16 bits, discard lower 8 bits */
						wavbuf16[i*2+0]=wavbuf[(wavchnr*i+wavch)*3+1];
						wavbuf16[i*2+1]=wavbuf[(wavchnr*i+wavch)*3+2];
					}
				}else if (wavbitnr==32){
					/* convert 32bit WAV sample format into 16bit WAV sample format */
					for (i=0;i<wavsamplecount;i++){
						/* Note: copy upper 16 bits, discard lower 16 bits */
						wavbuf16[i*2+0]=wavbuf[(wavchnr*i+wavch)*4+2];
						wavbuf16[i*2+1]=wavbuf[(wavchnr*i+wavch)*4+3];
					}
				}
			} /* Note: else: wavbuf==wavbuf16 */

			if ((type==AKAI_SAMPLE900_FTYPE)&&s9cflag){ /* S900 compressed sample format? */
				/* determine sample size */
				/* first pass to convert 16bit WAV sample format into S900 compressed sample format */
				/* Note: first pass of akai_sample900compr_wav2sample() requires 16bit WAV sample to be loaded to wavbuf16 */
				r=akai_sample900compr_wav2sample(NULL,wavbuf16,samplecountpart); /* NULL: first pass */
				if (r<0){
					goto akai_wav2sample_exit;
				}
				/* size in bytes */
				samplesize=(u_int)r;
			}
			/* check sample size */
			if (hdrsize+samplesize>AKAI_FILE_SIZEMAX){
				PRINTF_ERR("WAV file is too large\n");
				/* unknown or unsupported */
				ret=1; /* no error */
				goto akai_wav2sample_exit;
			}
			if (type==AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
				/* allocate sample buffer */
				sbuf=(u_char *)malloc(samplesize);
				if (sbuf==NULL){
					PRINTF_ERR("cannot allocate sample buffer\n");
					goto akai_wav2sample_exit;
				}
			}
		}

		if (wavch==1){ /* right channel? */
			/* update sample name */
			i=nlenmax-1;
			sname[i]='R'; /* right channel */
			fname[i]=sname[i];
		}

#ifdef WAV2SAMPLE_OVERWRITE
		if ((findex==AKAI_CREATE_FILE_NOINDEX)
			/* check if destination file already exists */
			&&(akai_find_file(volp,&tmpfile,fname)==0)){
			/* exists */
			if (what&WAV2SAMPLE_OVERWRITE){
				/* delete file */
				if (akai_delete_file(&tmpfile)<0){
					PRINTF_ERR("cannot overwrite existing file\n");
					goto akai_wav2sample_exit;
				}
			}else{
				PRINTF_ERR("file name already used\n");
				goto akai_wav2sample_exit;
			}
		}
#endif

		/* create file */
		/* Note: akai_create_file() will correct osver if necessary */
		if (akai_create_file(volp,&tmpfile,
							 hdrsize+samplesize,
							 findex,
							 fname,
							 osver,
							 tagp)<0){
			PRINTF_ERR("cannot create file\n");
			goto akai_wav2sample_exit;
		}

		if (type==AKAI_SAMPLE900_FTYPE){
			/* S900 sample */

			/* set RAM name of sample in sample header */
			/* Note: akai_fixramname() not needed afterwards */
			ascii2akai_name(sname,(u_char *)s900hdr.name,1); /* 1: S900 */

			/* write sample header */
			if (akai_write_file(0,(u_char *)&s900hdr,&tmpfile,0,hdrsize)<0){
				PRINTF_ERR("cannot write sample header\n");
				goto akai_wav2sample_exit;
			}

			if (wavsamplecount>0){
				if (s9cflag){
					/* second pass to convert 16bit WAV sample format into S900 compressed sample format */
					if (akai_sample900compr_wav2sample(sbuf,wavbuf16,samplecountpart)<0){ /* sbuf!=NULL: second pass */
						goto akai_wav2sample_exit;
					}
				}else{
					/* convert 16bit WAV sample format into S900 non-compressed sample format */
					akai_sample900noncompr_wav2sample(sbuf,wavbuf16,samplecountpart);
				}
			}
		}else{
			/* S1000/S3000 sample */

			/* set RAM name of sample in sample header */
			/* Note: akai_fixramname() not needed afterwards */
			ascii2akai_name(sname,s3000hdr.s1000.name,0); /* 0: not S900 */

			/* write sample header */
			if (akai_write_file(0,(u_char *)&s3000hdr,&tmpfile,0,hdrsize)<0){
				PRINTF_ERR("cannot write sample header\n");
				goto akai_wav2sample_exit;
			}

			/* Note: sbuf==wavbuf16 for S1000/S3000 sample */
		}

		if (wavsamplecount>0){
			/* write sample */
			if (akai_write_file(0,sbuf,&tmpfile,hdrsize,hdrsize+samplesize)<0){
				PRINTF_ERR("cannot write sample\n");
				goto akai_wav2sample_exit;
			}
		}

		if (wavchnr==1){ /* mono? */
#if 1
			PRINTF_OUT("sample imported from WAV\n");
#endif
			break; /* done */
		} /* else: stereo */
#if 1
		PRINTF_OUT("%s sample imported from WAV\n",(wavch==0)?"left":"right");
#endif

		/* next WAV channel */
		wavch++;
		if (wavch>=wavchnr){
			break; /* done */
		}
		if (findex!=AKAI_CREATE_FILE_NOINDEX){
			findex++; /* XXX next file index */
		}
		if (type==AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
			if (sbuf!=NULL){
				free(sbuf);
				sbuf=NULL;
			}
			if (s9cflag){
				akai_sample900compr_wav2sample(NULL,NULL,0); /* NULL,NULL: free buffers if still allocated */
			}
		}
	}

	ret=0; /* success */

akai_wav2sample_exit:
	if (wavbuf16!=NULL){
		free(wavbuf16);
	}
	if ((!wavmono16flag)&&(wavbuf!=NULL)){
		free(wavbuf);
	}
	if (type==AKAI_SAMPLE900_FTYPE){ /* S900 sample? */
		if (sbuf!=NULL){
			free(sbuf);
		}
		if (s9cflag){
			akai_sample900compr_wav2sample(NULL,NULL,0); /* NULL,NULL: free buffers if still allocated */
		}
	}
	if (what&WAV2SAMPLE_OPEN){
		if (wavfd>=0){
			CLOSE(wavfd);
		}
	}
	if (bcountp!=NULL){
		*bcountp=bcount;
	}
	return ret;
}



/* EOF */
