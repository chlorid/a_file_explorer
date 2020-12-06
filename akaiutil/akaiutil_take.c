/*
* Copyright (C) 2012,2018,2019,2020 Klaus Michael Indlekofer. All rights reserved.
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
#include "akaiutil_take.h"
#include "akaiutil_wav.h"



int
akai_ddtake_info(struct part_s *pp,u_int ti,int verbose)
{
	char namebuf[DIRNAMEBUF_LEN+1]; /* +1 for '\0' */
	struct akai_ddtake_s *tp;
	u_int cstarts,cstarte;
	u_int csizes,csizee;
	u_int scount;
	u_int srate;
	double vspeed;

	if ((pp==NULL)||(!pp->valid)){
		return -1;
	}
	if (pp->type!=PART_TYPE_DD){
		return -1;
	}

	if (ti>=AKAI_DDTAKE_MAXNUM){
		if (verbose){
			PRINTF_ERR("invalid take index\n");
		}
		return -1;
	}

	tp=&pp->head.dd.take[ti];

	if (tp->stat==AKAI_DDTAKESTAT_FREE){ /* free? */
		if (verbose){
			PRINTF_ERR("take not found\n");
		}
		return -1;
	}

	/* clusters */
	cstarts=(tp->cstarts[1]<<8)
		    +tp->cstarts[0];
	cstarte=(tp->cstarte[1]<<8)
		    +tp->cstarte[0];

	/* determine csizes (sample clusters) and csizee (envelope clusters) */
	if (cstarts!=0){ /* sample not empty? */
		csizes=akai_count_ddfatchain(pp,cstarts);
	}else{
		csizes=0;
	}
	if (cstarte!=0){ /* envelope not empty? */
		csizee=akai_count_ddfatchain(pp,cstarte);
	}else{
		csizee=0;
	}

	/* used sample count */
	scount=(tp->wend[3]<<24)
		+(tp->wend[2]<<16)
		+(tp->wend[1]<<8)
		+tp->wend[0]
		-(tp->wstart[3]<<24)
		-(tp->wstart[2]<<16)
		-(tp->wstart[1]<<8)
		-tp->wstart[0];
	/* XXX no check if scount is compatible with csizes */

	/* samplerate */
	srate=(tp->srate[1]<<8)
		  +tp->srate[0];

	/* name */
	akai2ascii_name(tp->name,namebuf,0); /* 0: not S900 */

	if (verbose){
		PRINTF_OUT("%s%s\n",namebuf,AKAI_DDTAKE_FNAMEEND);
		PRINTF_OUT("tnr:      %u\n",ti+1);
		PRINTF_OUT("tname:    \"%s\"\n",namebuf);
		PRINTF_OUT("cstarts:  cluster 0x%04x\n",cstarts);
		PRINTF_OUT("cstarte:  cluster 0x%04x\n",cstarte);
		PRINTF_OUT("csizes:   0x%04x clusters (%9u bytes)\n",
			csizes,
			csizes*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE);
		PRINTF_OUT("csizee:   0x%04x clusters (%9u bytes)\n",
			csizee,
			csizee*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE);
		PRINTF_OUT("scount:   0x%08x (%9u)\n",scount,scount);
		PRINTF_OUT("stype:    %s\n",(tp->stype==AKAI_DDTAKESTYPE_MONO)?"MONO":"STEREO");
		PRINTF_OUT("srate:    %uHz\n",srate);
		vspeed=((double)((((int)(char)tp->vspeed[1])<<8)+((int)(u_char)tp->vspeed[0])))/256.0; /* in % */
		PRINTF_OUT("vspeed:   %+06.2lf%%\n",vspeed);
		PRINTF_OUT("finerate: %+05ippm\n",(((int)(char)tp->finerate[1])<<8)+((int)(u_char)tp->finerate[0]));
		PRINTF_OUT("wstart:   0x%02x%02x%02x%02x\n",tp->wstart[3],tp->wstart[2],tp->wstart[1],tp->wstart[0]);
		PRINTF_OUT("wend:     0x%02x%02x%02x%02x\n",tp->wend[3],tp->wend[2],tp->wend[1],tp->wend[0]);
		PRINTF_OUT("wstartm:  0x%02x%02x%02x%02x\n",tp->wstartm[3],tp->wstartm[2],tp->wstartm[1],tp->wstartm[0]);
		PRINTF_OUT("wendm:    0x%02x%02x%02x%02x\n",tp->wendm[3],tp->wendm[2],tp->wendm[1],tp->wendm[0]);
		PRINTF_OUT("fadein:   %ums\n",(tp->fadein[1]<<8)+tp->fadein[0]);
		PRINTF_OUT("fadeout:  %ums\n",(tp->fadeout[1]<<8)+tp->fadeout[0]);
		PRINTF_OUT("stlvl:    %u\n",tp->stlvl);
		PRINTF_OUT("pan:      %+i\n",(int)(char)tp->pan);
		PRINTF_OUT("stmix:    %s\n",tp->stmix?"ON":"OFF");
		PRINTF_OUT("midich:   %u\n",tp->midich1+1);
		PRINTF_OUT("midinote: %u\n",tp->midinote);
		PRINTF_OUT("startm:   ");
		switch (tp->startm){
		case 0x00:
			PRINTF_OUT("IMMEDIATE\n");
			break;
		case 0x01:
			PRINTF_OUT("MIDI FOOTSW\n");
			break;
		case 0x02:
			PRINTF_OUT("MIDI NOTE\n");
			break;
		case 0x03:
			PRINTF_OUT("M.NOTE+DEL\n");
			break;
		case 0x04:
			PRINTF_OUT("START SONG\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("deemph:   %s\n",tp->deemph?"ON":"OFF");
		PRINTF_OUT("predel:   %u\n",(tp->predel[1]<<8)+tp->predel[0]);
		PRINTF_OUT("outlvl:   %u\n",tp->outlvl);
		PRINTF_OUT("outch:    ");
		switch (tp->outch){
		case 0x00:
			PRINTF_OUT("OFF\n");
			break;
		case 0x01:
			PRINTF_OUT("1/2\n");
			break;
		case 0x02:
			PRINTF_OUT("3/4\n");
			break;
		case 0x03:
			PRINTF_OUT("5/6\n");
			break;
		case 0x04:
			PRINTF_OUT("7/8\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("fxbus:    ");
		switch (tp->fxbus){
		case 0x00:
			PRINTF_OUT("OFF\n");
			break;
		case 0x01:
			PRINTF_OUT("FX1\n");
			break;
		case 0x02:
			PRINTF_OUT("FX2\n");
			break;
		case 0x03:
			PRINTF_OUT("RV3\n");
			break;
		case 0x04:
			PRINTF_OUT("RV4\n");
			break;
		default:
			PRINTF_OUT("\?\?\?\n");
			break;
		}
		PRINTF_OUT("sendlvl:  %u\n",tp->sendlvl);
	}else{
		PRINTF_OUT("%3u  %-12s  0x%04x  0x%04x  0x%04x  0x%04x  %9u  %5u  %s\n",
			ti+1,namebuf,
			cstarts,cstarte,
			csizes,csizee,
			scount,
			srate,
			(tp->stype==AKAI_DDTAKESTYPE_MONO)?"MONO  ":"STEREO");
	}

	return 0;
}



int
akai_import_take(int inpfd,struct part_s *pp,struct akai_ddtake_s *tp,u_int ti,u_int csizes,u_int csizee)
{
	u_int cstarts,cstarte;
	u_int samplesize;
	u_int envsiz;
	u_char *envbuf;
	u_int bsize;
	int ret;

	if ((pp==NULL)||(!pp->valid)||(pp->fat==NULL)){
		return -1;
	}
	if (pp->type!=PART_TYPE_DD){
		return -1;
	}

	if (tp==NULL){
		return -1;
	}

	if (ti>=AKAI_DDTAKE_MAXNUM){
		return -1;
	}

	/* Note: don't check if name already used, create new DD take */

	ret=-1; /* no success so far */
	envsiz=0;
	envbuf=NULL; /* not allocated yet */

	/* total number of bytes for sample */
	samplesize=csizes*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE;

	if ((csizee==0) /* no envelope in file? */
		&&(samplesize>0)){ /* and sample not empty? */
		/* new envelope */
		envsiz=((samplesize/2)+AKAI_DDTAKE_ENVBLKSIZW-1)/AKAI_DDTAKE_ENVBLKSIZW; /* number of bytes for envelope, round up (Note: /2 for 16bit per sample word) */
		csizee=(envsiz+AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE-1)/(AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE); /* in clusters, round up */
		/* allocate envelope buffer */
		if ((envbuf=(u_char *)malloc(envsiz))==NULL){
			PRINTF_ERR("cannot allocate envelope buffer\n");
			goto akai_import_take_exit;
		}
	}

	if ((csizes+csizee)*AKAI_DDPART_CBLKS>pp->bfree){
		PRINTF_ERR("not enough space left\n");
		goto akai_import_take_exit;
	}
	/* allocate space for take */
	if (csizes>0){
		/* sample */
#if 1
		if (akai_allocate_ddfatchain(pp,csizes,&cstarts,1)<0){ /* 1: not necessarily contiguous */
#else
		if (akai_allocate_ddfatchain(pp,csizes,&cstarts,csizes)<0){ /* contiguous */
#endif
			PRINTF_ERR("cannot allocate FAT chain for sample\n");
			goto akai_import_take_exit;
		}
	}else{
		cstarts=0; /* empty sample */
	}
	if (csizee>0){
		/* envelope */
#if 1
		if (akai_allocate_ddfatchain(pp,csizee,&cstarte,1)<0){ /* 1: not necessarily contiguous */
#else
		if (akai_allocate_ddfatchain(pp,csizee,&cstarte,csizee)<0){ /* contiguous */
#endif
			PRINTF_ERR("cannot allocate FAT chain for envelope\n");
			if (csizes>0){
				/* free FAT at cstarts */
				akai_free_ddfatchain(pp,cstarts,1); /* XXX ignore error */
				goto akai_import_take_exit;
			}
		}
	}else{
		cstarte=0; /* empty envelope */
	}

	/* fundamental settings of DD take */
	tp->stat=AKAI_DDTAKESTAT_USED; /* used */
	/* set cstarts and cstarte in DD take header */
	tp->cstarts[1]=0xff&(cstarts>>8);
	tp->cstarts[0]=0xff&cstarts;
	tp->cstarte[1]=0xff&(cstarte>>8);
	tp->cstarte[0]=0xff&cstarte;
	/* XXX keep name */
	/* copy DD take header into directory entry */
	bcopy(tp,&pp->head.dd.take[ti],sizeof(struct akai_ddtake_s));
	/* write partition header */
	if (akai_io_blks(pp,(u_char *)&pp->head.dd,
					 0,
					 AKAI_DDPARTHEAD_BLKS,
					 1,IO_BLKS_WRITE)<0){ /* 1: allocate cache if possible */
		PRINTF_ERR("cannot write partition header\n");
		goto akai_import_take_exit;
	}

	if (csizes>0){
		/* sample */
		if (akai_import_ddfatchain(pp,cstarts,0,samplesize,inpfd,NULL)<0){
			PRINTF_ERR("cannot import DD take\n");
			goto akai_import_take_exit;
		}
	}

	if (csizee>0){
		/* envelope */
		if (envbuf!=NULL){ /* no envelope in file? (see above) */
			/* calculate envelope from sample */
			if (akai_take_setenv(pp,cstarts,samplesize,envbuf,envsiz)<0){
				goto akai_import_take_exit;
			}
			/* write envelope to DD take */
			if (akai_import_ddfatchain(pp,cstarte,0,envsiz,-1,envbuf)<0){
				PRINTF_ERR("cannot save envelope\n");
				goto akai_import_take_exit;
			}
		}else{
			/* import envelope from file */
			bsize=csizee*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE;
			if (akai_import_ddfatchain(pp,cstarte,0,bsize,inpfd,NULL)<0){
				PRINTF_ERR("cannot import DD take\n");
				goto akai_import_take_exit;
			}
		}
	}

	ret=0; /* success */

akai_import_take_exit:
	if (envbuf!=NULL){
		free(envbuf);
	}
	return ret;
}



int
akai_export_take(int outfd,struct part_s *pp,struct akai_ddtake_s *tp,u_int csizes,u_int csizee,u_int cstarts,u_int cstarte)
{
	u_int bsize;

	if ((pp==NULL)||(!pp->valid)||(pp->fat==NULL)){
		return -1;
	}
	if (pp->type!=PART_TYPE_DD){
		return -1;
	}

	if (tp!=NULL){
		/* export DD take header */
		if (WRITE(outfd,tp,sizeof(struct akai_ddtake_s))!=(int)sizeof(struct akai_ddtake_s)){
			PRINTF_ERR("cannot export DD take header\n");
			return -1;
		}
	}

	if (csizes>0){
		/* sample */
		bsize=csizes*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE; /* in bytes */
		if (akai_export_ddfatchain(pp,cstarts,0,bsize,outfd,NULL)<0){
			PRINTF_ERR("cannot export DD take sample\n");
			return -1;
		}
	}

	if (csizee>0){
		/* envelope */
		bsize=csizee*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE; /* in bytes */
		if (akai_export_ddfatchain(pp,cstarte,0,bsize,outfd,NULL)<0){
			PRINTF_ERR("cannot export DD take envelope\n");
			return -1;
		}
	}

	return 0;
}



int
akai_take2wav(struct part_s *pp,u_int ti,int wavfd,u_int *sizep,char **wavnamep,int what)
{
	/* Note: static for multiple calls with different what */
	static u_int cstarts;
	static u_int samplestart;
	static u_int samplesize;
	static u_int samplerate;
	static u_int samplechnr;
	static int ret;
	static char wavname[AKAI_NAME_LEN+4+1]; /* name (ASCII), +4 for ".<type>", +1 for '\0' */
	static u_int nlen;

	if ((pp==NULL)||(!pp->valid)||(pp->fat==NULL)){
		return -1;
	}
	if (pp->type!=PART_TYPE_DD){
		return -1;
	}

	if (ti>=AKAI_DDTAKE_MAXNUM){
		return -1;
	}

	ret=-1; /* no success so far */

	if (what&TAKE2WAV_CHECK){

		if (pp->head.dd.take[ti].stat==AKAI_DDTAKESTAT_FREE){ /* free? */
			return -1;
		}

		/* sample start cluster */
		cstarts=(pp->head.dd.take[ti].cstarts[1]<<8)
			    +pp->head.dd.take[ti].cstarts[0];
		if (cstarts!=0){ /* sample not empty? */
#if 1
			/* Note: words are 16bit */
			/* start word */
			samplestart=(pp->head.dd.take[ti].wstart[3]<<24)
				+(pp->head.dd.take[ti].wstart[2]<<16)
				+(pp->head.dd.take[ti].wstart[1]<<8)
				+pp->head.dd.take[ti].wstart[0];
			/* end word */
			samplesize=(pp->head.dd.take[ti].wend[3]<<24)
				+(pp->head.dd.take[ti].wend[2]<<16)
				+(pp->head.dd.take[ti].wend[1]<<8)
				+pp->head.dd.take[ti].wend[0];
			if (samplesize<samplestart){ /* end<start? */
				return -1;
			}
			samplesize-=samplestart; /* size in words */
			samplestart*=2; /* *2 for 16bit per sample word */
			samplesize*=2; /* *2 for 16bit per sample word */
			/* XXX check samplestart and samplesize */
#else
			/* all clusters */
			samplestart=0;
			samplesize=akai_count_ddfatchain(pp,cstarts)*AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE; /* in bytes */
#endif
		}else{
			samplestart=0;
			samplesize=0;
		}
		/* Note: samplesize==0 is allowed here */

		/* take parameters */
		samplerate=(pp->head.dd.take[ti].srate[1]<<8)
			+pp->head.dd.take[ti].srate[0];
		samplechnr=(pp->head.dd.take[ti].stype==AKAI_DDTAKESTYPE_MONO)?1:2;

#ifdef DEBUG
		PRINTF_OUT("samplestart: %15u bytes\n",samplestart);
		PRINTF_OUT("samplesize:  %15u bytes\n",samplesize);
		PRINTF_OUT("samplerate:  %15u Hz\n",samplerate);
		PRINTF_OUT("samplechnr:  %15u\n",samplechnr);
#endif

		if (sizep!=NULL){
			/* WAV file size */
			*sizep=WAV_HEAD_SIZE+samplesize;
#ifndef WAV_AKAIHEAD_DISABLE
			*sizep+=sizeof(struct wav_chunkhead_s)+sizeof(struct akai_ddtake_s); /* DD take header chunk (see below) */
#endif
		}

		/* create WAV name */
		akai2ascii_name(pp->head.dd.take[ti].name,wavname,0); /* 0: not S900 */
		nlen=(u_int)strlen(wavname);
		bcopy(".wav",wavname+nlen,5);

		if (wavnamep!=NULL){
			/* pointer to name (wavname must be static!) */
			*wavnamep=wavname;
		}
	}

	if (what&TAKE2WAV_EXPORT){

		if (what&TAKE2WAV_CREATE){
			/* create WAV file */
			if ((wavfd=OPEN(wavname,O_RDWR|O_CREAT|O_TRUNC|O_BINARY,0666))<0){
				PERROR("create WAV");
				goto akai_take2wav_exit;
			}
		}

		/* write WAV header */
		if (wav_write_head(wavfd,
						   samplesize,samplechnr,samplerate,16, /* 16: 16bit */
#ifndef WAV_AKAIHEAD_DISABLE
						   sizeof(struct wav_chunkhead_s)+sizeof(struct akai_ddtake_s) /* DD take header chunk (see below) */
#else
						   0
#endif
						   )<0){
			PRINTF_ERR("cannot write WAV header\n");
			goto akai_take2wav_exit;
		}

		if (samplesize>0){
			/* Note: no sample format conversion necessary */
			/* export sample */
			if (akai_export_ddfatchain(pp,cstarts,samplestart,samplesize,wavfd,NULL)<0){
				PERROR("write WAV samples");
				goto akai_take2wav_exit;
			}
		}

#ifndef WAV_AKAIHEAD_DISABLE
		{
			struct wav_chunkhead_s wavchunkhead;
			struct akai_ddtake_s t;
			u_int hdrsize;
			u_int csizes;

			/* create DD take header chunk */
			bcopy(WAV_CHUNKHEAD_AKAIDDTAKEHEADSTR,wavchunkhead.typestr,4);
			hdrsize=sizeof(struct akai_ddtake_s);
			wavchunkhead.csize[0]=0xff&hdrsize;
			wavchunkhead.csize[1]=0xff&(hdrsize>>8);
			wavchunkhead.csize[2]=0xff&(hdrsize>>16);
			wavchunkhead.csize[3]=0xff&(hdrsize>>24);
			/* write WAV chunk header */
			if (WRITE(wavfd,(u_char *)&wavchunkhead,sizeof(struct wav_chunkhead_s))!=(int)sizeof(struct wav_chunkhead_s)){
				PERROR("write WAV chunk");
				goto akai_take2wav_exit;
			}
			/* get DD take header from directory entry */
			bcopy((u_char *)&pp->head.dd.take[ti],&t,hdrsize);
			/* determine csizes (sample clusters) and csizee (envelope clusters) */
			csizes=akai_count_ddfatchain(pp,cstarts);
			/* Note: no envelope in WAV file -> csizee=0 */
			/* XXX use cstarts/cstarte fields in DD take header for csizes/csizee */
			t.cstarts[1]=0xff&(csizes>>8);
			t.cstarts[0]=0xff&csizes;
			t.cstarte[1]=0x00;
			t.cstarte[0]=0x00;
			/* write DD take header */
			if (WRITE(wavfd,(u_char *)&t,hdrsize)!=(int)hdrsize){
				PERROR("write WAV chunk");
				goto akai_take2wav_exit;
			}
		}
#endif
#if 1
		PRINTF_OUT("DD take exported to WAV\n");
#endif
	}

	ret=0; /* success */

akai_take2wav_exit:
	if (what&TAKE2WAV_CREATE){
		if (wavfd>=0){
			CLOSE(wavfd);
		}
	}
	return ret;
}



int
akai_wav2take(int wavfd,char *wavname,struct part_s *pp,u_int ti,u_int *bcountp,int what)
{
	/* Note: static for multiple calls with different what */
	static struct akai_ddtake_s t;
	static u_int samplerate;
	static u_int samplesize;
	static u_int wavchnr;
	static u_int wavbitnr;
	static u_int wavsamplesize;
	static u_int wavsamplesizealloc;
	static u_int wavsamplecount;
	static u_char *wavbuf;
	static char *errstrp;
	static u_int nlen;
	static char name[AKAI_NAME_LEN+1]; /* name (ASCII), +1 for '\0' */
	static u_int cstarts,cstarte;
	static u_int csizes,csizee;
	static u_int envsiz;
	static u_char *envbuf;
	static u_int bcount;
#ifndef WAV_AKAIHEAD_DISABLE
	static u_int extrasize;
	static int wavakaiheadfound;
#endif
	static u_int i,j;
	static int ret;

	if (bcountp!=NULL){
		*bcountp=0; /* no bytes read yet */
	}

	if ((pp==NULL)||(!pp->valid)||(pp->fat==NULL)){
		return -1;
	}
	if (pp->type!=PART_TYPE_DD){
		return -1;
	}

	if (ti>=AKAI_DDTAKE_MAXNUM){
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

	/* Note: don't check if name already used, create new DD take */

	ret=-1; /* no success so far */
	bcount=0; /* no bytes read yet */
	envbuf=NULL; /* not allocated yet */
	wavbuf=NULL; /* not allocated yet */

	if (what&WAV2TAKE_OPEN){
		/* open external WAV file */
		if ((wavfd=akai_openreadonly_extfile(wavname))<0){
			PERROR("open WAV");
			goto akai_wav2take_exit;
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
		goto akai_wav2take_exit;
	}

	/* check parameters */
	if ((wavchnr!=1)&&(wavchnr!=2)){
		PRINTF_ERR("invalid number of channels, must be mono or stereo\n");
		/* unknown or unsupported */
		ret=1; /* no error */
		goto akai_wav2take_exit;
	}
	if ((wavbitnr!=8)&&(wavbitnr!=16)&&(wavbitnr!=24)&&(wavbitnr!=32)){
		PRINTF_ERR("invalid WAV format, must be 8bit or 16bit or 24bit or 32bit PCM\n");
		/* unknown or unsupported */
		ret=1; /* no error */
		goto akai_wav2take_exit;
	}

	/* number of samples in WAV file */
	/* Note: sum over all channels */
	wavsamplecount=wavsamplesize/(wavbitnr/8);
#if 0
	if (wavsamplecount==0){
		PRINTF_ERR("no samples in WAV file\n");
		ret=1; /* no error */
		goto akai_wav2take_exit;
	}
#endif /* else: Note: wavesamplecount==0 is allowed here */

	if (wavsamplecount>0){
		/* determine csizes (sample clusters) and csizee (envelope clusters) */
		samplesize=wavsamplecount*2; /* 16bit per sample word in DD take */
		csizes=(samplesize+AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE-1)/(AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE); /* in clusters, round up */
		envsiz=((samplesize/2)+AKAI_DDTAKE_ENVBLKSIZW-1)/AKAI_DDTAKE_ENVBLKSIZW; /* number of bytes for envelope, round up (Note: /2 for 16bit per sample word) */
		csizee=(envsiz+AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE-1)/(AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE); /* in clusters, round up */

		/* allocate envelope buffer */
		if ((envbuf=(u_char *)malloc(envsiz))==NULL){
			PRINTF_ERR("cannot allocate envelope buffer\n");
			goto akai_wav2take_exit;
		}

		/* allocate space for take */
#if 1
		if (akai_allocate_ddfatchain(pp,csizes,&cstarts,1)<0){ /* 1: not necessarily contiguous */
#else
		if (akai_allocate_ddfatchain(pp,csizes,&cstarts,csizes)<0){ /* contiguous */
#endif
			PRINTF_ERR("cannot allocate FAT chain for sample\n");
			goto akai_wav2take_exit;
		}
#if 1
		if (akai_allocate_ddfatchain(pp,csizee,&cstarte,1)<0){ /* 1: not necessarily contiguous */
#else
		if (akai_allocate_ddfatchain(pp,csizee,&cstarte,csizee)<0){ /* contiguous */
#endif
			PRINTF_ERR("cannot allocate FAT chain for envelope\n");
			/* free FAT at cstarts */
			akai_free_ddfatchain(pp,cstarts,1); /* XXX ignore error */
			goto akai_wav2take_exit;
		}
	}else{
		samplesize=0;
		csizes=0;
		envsiz=0;
		csizee=0;
		cstarts=0; /* no sample */
		cstarte=0; /* no envelope */
	}

	if (csizes>0){
		/* read WAV sample and write sample to DD take */
#if 1
		if (wavbitnr==16){
			/* Note: no sample format conversion necessary */
			/* copy sample from WAV file to DD take */
			if (akai_import_ddfatchain(pp,cstarts,0,samplesize,wavfd,NULL)<0){
				PRINTF_ERR("cannot import DD take\n");
				goto akai_wav2take_exit;
			}
			bcount+=samplesize;
		}else
#endif
		{
			/* allocate WAV sample buffer */
			/* buffer for at least 16bit */
			if (wavbitnr>16){
				wavsamplesizealloc=wavsamplecount*(wavbitnr/8);
			}else{
				wavsamplesizealloc=wavsamplecount*(16/8);
			}
			wavbuf=(u_char *)malloc(wavsamplesizealloc);
			if (wavbuf==NULL){
				PRINTF_ERR("cannot allocate WAV buffer\n");
				goto akai_wav2take_exit;
			}

			/* read WAV sample to memory */
			if (READ(wavfd,wavbuf,wavsamplesize)!=(int)wavsamplesize){
				PRINTF_ERR("cannot read sample\n");
				goto akai_wav2take_exit;
			}
			bcount+=wavsamplesize;
			if (wavbitnr==8){
				/* convert 8bit WAV sample format into 16bit WAV sample format */
				/* Note: write pointer must not overtake read pointer => address sequence must be downwards */
				for (i=0;i<wavsamplecount;i++){
					j=wavsamplecount-1-i;
					wavbuf[j*2+1]=0x80^wavbuf[j]; /* toggle sign bit */
					wavbuf[j*2+0]=0x00;
				}
			}else if (wavbitnr==24){
				/* convert 24bit WAV sample format into 16bit WAV sample format */
				/* Note: write pointer must not overtake read pointer => address sequence must be upwards */
				for (i=0;i<wavsamplecount;i++){
					/* Note: copy upper 16 bits, discard lower 8 bits */
					wavbuf[i*2+0]=wavbuf[i*3+1];
					wavbuf[i*2+1]=wavbuf[i*3+2];
				}
			}else if (wavbitnr==32){
				/* convert 32bit WAV sample format into 16bit WAV sample format */
				/* Note: write pointer must not overtake read pointer => address sequence must be upwards */
				for (i=0;i<wavsamplecount;i++){
					/* Note: copy upper 16 bits, discard lower 16 bits */
					wavbuf[i*2+0]=wavbuf[i*4+2];
					wavbuf[i*2+1]=wavbuf[i*4+3];
				}
			}

			/* write sample to DD take */
			if (akai_import_ddfatchain(pp,cstarts,0,samplesize,-1,wavbuf)<0){
				PRINTF_ERR("cannot import DD take\n");
				goto akai_wav2take_exit;
			}
		}
	}

	if (csizee>0){
		/* Note: no envelope in WAV file */
		/* calculate envelope from sample */
		if (akai_take_setenv(pp,cstarts,samplesize,envbuf,envsiz)<0){
			goto akai_wav2take_exit;
		}
		/* write envelope to DD take */
		if (akai_import_ddfatchain(pp,cstarte,0,envsiz,-1,envbuf)<0){
			PRINTF_ERR("cannot save envelope\n");
			goto akai_wav2take_exit;
		}
	}

#ifndef WAV_AKAIHEAD_DISABLE
	/* check for DD take header chunk in WAV file */
	{
		int wavakaiheadtype;
		u_int wavakaiheadsize;
		u_int bc;

		wavakaiheadtype=wav_find_akaihead(wavfd,&bc,&wavakaiheadsize,extrasize,WAV_AKAIHEADTYPE_DDTAKE);
		if (wavakaiheadtype<0){
			goto akai_wav2take_exit;
		}
		bcount+=bc;

		if ((wavakaiheadtype==(int)WAV_AKAIHEADTYPE_DDTAKE)&&(wavakaiheadsize==sizeof(struct akai_ddtake_s))){
			/* found matching DD take header chunk */
			wavakaiheadfound=1;
		}else{
			wavakaiheadfound=0;
		}
	}

	if (wavakaiheadfound){
		/* read DD take header */
		if (READ(wavfd,(u_char *)&t,sizeof(struct akai_ddtake_s))!=(int)sizeof(struct akai_ddtake_s)){
			PRINTF_ERR("cannot read DD take header\n");
			goto akai_wav2take_exit;
		}
		bcount+=sizeof(struct akai_ddtake_s);
#if 1
		PRINTF_OUT("DD take header imported from WAV\n");
#endif
		/* Note: ignore cstarts/cstarte fields in DD take header */
		/* Note: keep name in DD take header */
		/* Note: must overwrite some settings in DD take header (see below) */
	}else
#endif
	{
		u_int wstart,wend;
		u_int wstartm,wendm;

		/* create DD take header */
		bzero((void *)&t,sizeof(struct akai_ddtake_s));
		/* default parameter settings */
		wstart=0;
		wend=samplesize/2; /* /2 for 16bit per sample word */
		wstartm=wstart;
		wendm=wend;
		t.wstart[3]=0xff&(wstart>>24);
		t.wstart[2]=0xff&(wstart>>16);
		t.wstart[1]=0xff&(wstart>>8);
		t.wstart[0]=0xff&wstart;
		t.wend[3]=0xff&(wend>>24);
		t.wend[2]=0xff&(wend>>16);
		t.wend[1]=0xff&(wend>>8);
		t.wend[0]=0xff&wend;
		t.wstartm[3]=0xff&(wstartm>>24);
		t.wstartm[2]=0xff&(wstartm>>16);
		t.wstartm[1]=0xff&(wstartm>>8);
		t.wstartm[0]=0xff&wstartm;
		t.wendm[3]=0xff&(wendm>>24);
		t.wendm[2]=0xff&(wendm>>16);
		t.wendm[1]=0xff&(wendm>>8);
		t.wendm[0]=0xff&wendm;
		t.fadein[1]=0xff&(10>>8); /* XXX */
		t.fadein[0]=0xff&10; /* XXX */
		t.fadeout[1]=0xff&(50>>8); /* XXX */
		t.fadeout[0]=0xff&50; /* XXX */
		t.stlvl=99; /* XXX */
		t.stmix=0x01; /* XXX */
		t.midich1=16-1; /* XXX */
		t.midinote=60; /* XXX */
		t.startm=0x03; /* XXX */
		t.predel[1]=0xff&(500>>8); /* XXX */
		t.predel[0]=0xff&500; /* XXX */
		t.outlvl=50; /* XXX */
		t.sendlvl=25; /* XXX */
		/* derive DD take name from WAV file name */
		if (nlen>AKAI_NAME_LEN){
			nlen=AKAI_NAME_LEN;
		}
		bcopy(wavname,name,nlen);
		name[nlen]='\0';
		ascii2akai_name(name,t.name,0); /* 0: not S900 */
	}
	/* fundamental settings of DD take */
	t.stat=AKAI_DDTAKESTAT_USED; /* used */
	/* set cstarts and cstarte in DD take header */
	t.cstarts[1]=0xff&(cstarts>>8);
	t.cstarts[0]=0xff&cstarts;
	t.cstarte[1]=0xff&(cstarte>>8);
	t.cstarte[0]=0xff&cstarte;
	t.stype=(wavchnr==2)?AKAI_DDTAKESTYPE_STEREO:AKAI_DDTAKESTYPE_MONO;
	t.srate[1]=0xff&(samplerate>>8);
	t.srate[0]=0xff&samplerate;

	/* copy DD take header into directory entry */
	bcopy(&t,&pp->head.dd.take[ti],sizeof(struct akai_ddtake_s));
	/* write partition header */
	if (akai_io_blks(pp,(u_char *)&pp->head.dd,
					 0,
					 AKAI_DDPARTHEAD_BLKS,
					 1,IO_BLKS_WRITE)<0){ /* 1: allocate cache if possible */
		PRINTF_ERR("cannot write partition header\n");
		goto akai_wav2take_exit;
	}

#if 1
	PRINTF_OUT("DD take imported from WAV\n");
#endif

	ret=0; /* success */

akai_wav2take_exit:
	if (what&WAV2TAKE_OPEN){
		if (wavfd>=0){
			CLOSE(wavfd);
		}
	}
	if (bcountp!=NULL){
		*bcountp=bcount;
	}
	if (envbuf!=NULL){
		free(envbuf);
	}
	if (wavbuf!=NULL){
		free(wavbuf);
	}
	return ret;
}



/* set envelope for take */
int
akai_take_setenv(struct part_s *pp,u_int cstarts,u_int samplesize,u_char *envbuf,u_int envsiz)
{
	static u_char logabstab[0x100]; /* must be static */
	static int logabstabready=0; /* must be static */
	u_char l,lmax;
	u_int i,j;
	u_int ba,bremain,bchunk,la,lchunk;
	static u_char sbuf[AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE]; /* 1 cluster */

	if (!logabstabready){ /* logabstab not initialized yet? */
		double lconst;

		/* initialize logabstab */
		lconst=((double)(AKAI_DDTAKE_ENVMAXVAL-1))/log((double)0x80);
		logabstab[0x00]=0;
		for (i=0x01;i<=0x7f;i++){
			logabstab[i]=((u_char)(lconst*log((double)i)))+1;
		}
		logabstab[0x80]=AKAI_DDTAKE_ENVMAXVAL;
		for (i=0x81;i<=0xff;i++){
			logabstab[i]=logabstab[0x100-i];
		}

		logabstabready=1; /* logabstab is initialized */
	}

	if ((pp==NULL)||(!pp->valid)||(pp->fat==NULL)){
		return -1;
	}
	if (pp->type!=PART_TYPE_DD){
		return -1;
	}

	if (envbuf==NULL){
		return -1;
	}

	/* zero envbuf */
	bzero(envbuf,envsiz);

	if (samplesize==0){
		return 0;
	}

	/* calculate envelope of samples */
	ba=0;
	bremain=samplesize;
	i=0;
	while ((bremain>0)&&(i<envsiz)){
		/* cluster */
		bchunk=AKAI_DDPART_CBLKS*AKAI_HD_BLOCKSIZE;
		if (bchunk>bremain){
			bchunk=bremain;
		}

		/* read samples from take */
		if (akai_export_ddfatchain(pp,cstarts,ba,bchunk,-1,sbuf)<0){
			PRINTF_ERR("cannot read take\n");
#if 1
			break;
#else
			return -1;
#endif
		}

		la=0;
		while ((bchunk>0)&&(i<envsiz)){
			/* envelope block */
			lchunk=AKAI_DDTAKE_ENVBLKSIZW<<1; /* *2 for 16bit per sample word */
			if (lchunk>bchunk){
				lchunk=bchunk;
			}
			/* determine peak level within envelope block */
			lmax=0;
			for (j=0;j<lchunk;j+=2){ /* 2 for 16bit per sample word */
				l=logabstab[sbuf[la+j+1]]; /* logabs of MSB */
				if (l>lmax){
					lmax=l;
				}
			}
			envbuf[i]=lmax;

			ba+=lchunk;
			bremain-=lchunk;
			la+=lchunk;
			bchunk-=lchunk;
			i++;
		}
	}

	return 0;
}



/* EOF */
