/*
* Copyright (C) 2010,2019 Klaus Michael Indlekofer. All rights reserved.
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
#include "akaiutil_wav.h"



int
wav_write_head(int outdes,
			   u_int datasize,u_int chnr,u_int samprate,u_int bitnr,
			   u_int extrasize)
{
	struct wav_riffhead_s wavriffhead;
	struct wav_chunkhead_s wavchunkhead;
	struct wav_fmthead_s wavfmthead;
	u_int drate;

	/* create WAVE RIFF header */
	bzero(&wavriffhead,sizeof(struct wav_riffhead_s));
	bcopy(WAV_RIFFHEAD_RIFFSTR,wavriffhead.riffstr,4);
	{
		u_int fsize;

		fsize=WAV_HEAD_SIZE-sizeof(struct wav_chunkhead_s)+datasize+extrasize;
		wavriffhead.fsize[0]=0xff&fsize;
		wavriffhead.fsize[1]=0xff&(fsize>>8);
		wavriffhead.fsize[2]=0xff&(fsize>>16);
		wavriffhead.fsize[3]=0xff&(fsize>>24);
	}
	bcopy(WAV_RIFFHEAD_WAVESTR,wavriffhead.wavestr,4);

	/* write WAVE RIFF header */
	if (WRITE(outdes,&wavriffhead,sizeof(struct wav_riffhead_s))!=sizeof(struct wav_riffhead_s)){
		PRINTF_ERR("cannot write WAVE RIFF header\n");
		return -1;
	}

	/* create FMT chunk header */
	bcopy(WAV_CHUNKHEAD_FMTSTR,wavchunkhead.typestr,4);
	{
		u_int csize;

		csize=sizeof(struct wav_fmthead_s);
		wavchunkhead.csize[0]=0xff&csize;
		wavchunkhead.csize[1]=0xff&(csize>>8);
		wavchunkhead.csize[2]=0xff&(csize>>16);
		wavchunkhead.csize[3]=0xff&(csize>>24);
	}
	/* write chunk header */
	if (WRITE(outdes,&wavchunkhead,sizeof(struct wav_chunkhead_s))!=sizeof(struct wav_chunkhead_s)){
		PRINTF_ERR("cannot write WAVE FMT header\n");
		return -1;
	}

	/* set FMT header */
	wavfmthead.ftag[0]=0xff&WAV_HEAD_FTAG_PCM;
	wavfmthead.ftag[1]=0xff&(WAV_HEAD_FTAG_PCM>>8);
	wavfmthead.chnr[0]=0xff&chnr;
	wavfmthead.chnr[1]=0xff&(chnr>>8);
	wavfmthead.srate[0]=0xff&samprate;
	wavfmthead.srate[1]=0xff&(samprate>>8);
	wavfmthead.srate[2]=0xff&(samprate>>16);
	wavfmthead.srate[3]=0xff&(samprate>>24);
	drate=chnr*samprate*(bitnr/8); /* chnr!!! */
	wavfmthead.drate[0]=0xff&drate;
	wavfmthead.drate[1]=0xff&(drate>>8);
	wavfmthead.drate[2]=0xff&(drate>>16);
	wavfmthead.drate[3]=0xff&(drate>>24);
	wavfmthead.balign[0]=0xff&((bitnr/8)*chnr);      /* chnr!!! */
	wavfmthead.balign[1]=0xff&(((bitnr/8)*chnr)>>8); /* chnr!!! */
	wavfmthead.bitnr[0]=0xff&bitnr;
	wavfmthead.bitnr[1]=0xff&(bitnr>>8);

	/* write WAVE FMT header */
	if (WRITE(outdes,&wavfmthead,sizeof(struct wav_fmthead_s))!=sizeof(struct wav_fmthead_s)){
		PRINTF_ERR("cannot write WAVE FMT header\n");
		return -1;
	}

	/* create WAVE DATA chunk header */
	bcopy(WAV_CHUNKHEAD_DATASTR,wavchunkhead.typestr,4);
	wavchunkhead.csize[0]=0xff&datasize;
	wavchunkhead.csize[1]=0xff&(datasize>>8);
	wavchunkhead.csize[2]=0xff&(datasize>>16);
	wavchunkhead.csize[3]=0xff&(datasize>>24);

	/* write chunk header */
	if (WRITE(outdes,&wavchunkhead,sizeof(struct wav_chunkhead_s))!=sizeof(struct wav_chunkhead_s)){
		PRINTF_ERR("cannot write WAVE DATA header\n");
		return -1;
	}

	return 0;
}



int
wav_read_head(int indes,
			  u_int *bcountp,
			  u_int *datasizep,u_int *chnrp,u_int *sampratep,u_int *bitnrp,
			  u_int *extrasizep,
			  char **errstrp)
{
	struct wav_riffhead_s wavriffhead;
	struct wav_chunkhead_s wavchunkhead;
	struct wav_fmthead_s wavfmthead;
	u_int i;
	u_int bcount;
	u_int remain;
	u_int csize;
	u_int chnr;
	u_int samprate;
	u_int bitnr;

	bcount=0; /* no bytes read yet */

	/* read WAVE RIFF header */
	if (READ(indes,(char *)&wavriffhead,sizeof(struct wav_riffhead_s))!=sizeof(struct wav_riffhead_s)){
		if (errstrp!=NULL){
			*errstrp="read WAVE RIFF header";
		}
		return -1;
	}
	bcount+=sizeof(struct wav_riffhead_s);

	/* check riffstr */
	for (i=0;i<4;i++){
		if (WAV_RIFFHEAD_RIFFSTR[i]!=wavriffhead.riffstr[i]){
			if (errstrp!=NULL){
				*errstrp="no RIFF";
			}
			return -1;
		}
	}

	/* determine remaining file size */
	remain=wavriffhead.fsize[0]
		+(wavriffhead.fsize[1]<<8)
		+(wavriffhead.fsize[2]<<16)
		+(wavriffhead.fsize[3]<<24);
	if (remain<sizeof(struct wav_riffhead_s)-sizeof(struct wav_chunkhead_s)){
		if (errstrp!=NULL){
			*errstrp="invalid file size";
		}
		return -1;
	}
	remain-=sizeof(struct wav_riffhead_s)-sizeof(struct wav_chunkhead_s);

	/* check wavestr */
	for (i=0;i<4;i++){
		if (WAV_RIFFHEAD_WAVESTR[i]!=wavriffhead.wavestr[i]){
			if (errstrp!=NULL){
				*errstrp="no WAVE";
			}
			return -1;
		}
	}

	/* search for fmt chunk */
	for (;;){
		/* read next chunk */
		if (remain<sizeof(struct wav_chunkhead_s)){
			if (errstrp!=NULL){
				*errstrp="invalid file size";
			}
			return -1;
		}
		if (READ(indes,(char *)&wavchunkhead,sizeof(struct wav_chunkhead_s))!=sizeof(struct wav_chunkhead_s)){
			if (errstrp!=NULL){
				*errstrp="read WAVE FMT header";
			}
			return -1;
		}
		bcount+=sizeof(struct wav_chunkhead_s);
		remain-=sizeof(struct wav_chunkhead_s);

		/* get csize */
		csize=wavchunkhead.csize[0]
			+(wavchunkhead.csize[1]<<8)
			+(wavchunkhead.csize[2]<<16)
			+(wavchunkhead.csize[3]<<24);
		if (remain<csize){
			if (errstrp!=NULL){
				*errstrp="invalid file size";
			}
			return -1;
		}

		for (i=0;i<4;i++){
			if (WAV_CHUNKHEAD_FMTSTR[i]!=wavchunkhead.typestr[i]){
				break; /* not fmt chunk */
			}
		}
		if (i==4){
			break; /* found fmt */
		}

		/* skip rest of chunk */
		if (LSEEK64(indes,(OFF64_T)csize,SEEK_CUR)<0){
			if (errstrp!=NULL){
				*errstrp="lseek WAVE FMT header";
			}
			return -1;
		}
		bcount+=csize;
		remain-=csize;
	}
	if (csize!=sizeof(struct wav_fmthead_s)){
		if (errstrp!=NULL){
			*errstrp="invalid FMT csize";
		}
		return -1;
	}

	/* read rest of fmt chunk */
	if (READ(indes,(char *)&wavfmthead,sizeof(struct wav_fmthead_s))!=sizeof(struct wav_fmthead_s)){
		if (errstrp!=NULL){
			*errstrp="read WAVE FMT header";
		}
		return -1;
	}
	bcount+=sizeof(struct wav_fmthead_s);
	remain-=sizeof(struct wav_fmthead_s);

	/* check ftag */
	/* XXX allow only PCM */
	if ((wavfmthead.ftag[0]!=(0xff&WAV_HEAD_FTAG_PCM))
		||(wavfmthead.ftag[1]!=(0xff&(WAV_HEAD_FTAG_PCM>>8)))){
		if (errstrp!=NULL){
			*errstrp="invalid ftag, must be PCM format";
		}
		return -1;
	}

	/* get chnr */
	chnr=wavfmthead.chnr[0]
		+(wavfmthead.chnr[1]<<8);
	if (chnr==0){
		if (errstrp!=NULL){
			*errstrp="invalid chnr";
		}
		return -1;
	}

	/* set samprate */
	samprate=wavfmthead.srate[0]
		+(wavfmthead.srate[1]<<8)
		+(wavfmthead.srate[2]<<16)
		+(wavfmthead.srate[3]<<24);

	/* XXX check drate: must be chnr*samprate*(bitnr/8) */

	/* get bitnr */
	bitnr=wavfmthead.bitnr[0]
		+(wavfmthead.bitnr[1]<<8);
	if (bitnr==0){
		if (errstrp!=NULL){
			*errstrp="invalid bitnr";
		}
		return -1;
	}

	/* XXX check balign: must be (bitnr/8)*chnr */

	/* search for data chunk */
	for (;;){
		/* read next chunk */
		if (remain<sizeof(struct wav_chunkhead_s)){
			if (errstrp!=NULL){
				*errstrp="invalid file size";
			}
			return -1;
		}
		if (READ(indes,(char *)&wavchunkhead,sizeof(struct wav_chunkhead_s))!=sizeof(struct wav_chunkhead_s)){
			if (errstrp!=NULL){
				*errstrp="read WAVE DATA header";
			}
			return -1;
		}
		bcount+=sizeof(struct wav_chunkhead_s);
		remain-=sizeof(struct wav_chunkhead_s);

		/* get csize */
		csize=wavchunkhead.csize[0]
			+(wavchunkhead.csize[1]<<8)
			+(wavchunkhead.csize[2]<<16)
			+(wavchunkhead.csize[3]<<24);
		if (remain<csize){
			if (errstrp!=NULL){
				*errstrp="invalid file size";
			}
			return -1;
		}

		for (i=0;i<4;i++){
			if (WAV_CHUNKHEAD_DATASTR[i]!=wavchunkhead.typestr[i]){
				break; /* not data chunk */
			}
		}
		if (i==4){
			break; /* found fmt */
		}

		/* skip rest of chunk */
		if (LSEEK64(indes,(OFF64_T)csize,SEEK_CUR)<0){
			if (errstrp!=NULL){
				*errstrp="lseek WAVE DATA header";
			}
			return -1;
		}
		bcount+=csize;
		remain-=csize;
	}
	if (csize%chnr!=0){
		if (errstrp!=NULL){
			*errstrp="invalid DATA csize";
		}
		return -1;
	}

	if (bcountp!=NULL){
		*bcountp=bcount;
	}
	if (datasizep!=NULL){
		*datasizep=csize;
	}
	if (chnrp!=NULL){
		*chnrp=chnr;
	}
	if (sampratep!=NULL){
		*sampratep=samprate;
	}
	if (bitnrp!=NULL){
		*bitnrp=bitnr;
	}
	if (extrasizep!=NULL){
		*extrasizep=remain-csize;
	}
	
	return 0;
}



#ifndef WAV_AKAIHEAD_DISABLE
int
wav_find_akaihead(int indes,u_int *bcountp,u_int *csizep,u_int remain,u_int searchtype)
{
	struct wav_chunkhead_s wavchunkhead;
	u_int bcount;
	u_int csize;
	u_int type;
	u_int i;

	bcount=0; /* no bytes read yet */
	csize=0; /* no chunk yet */
	type=WAV_AKAIHEADTYPE_NONE; /* no AKAI header chunk found yet */

	/* search for AKAI header chunk */
	for (;;){
		/* read next chunk */
		if (remain<sizeof(struct wav_chunkhead_s)){
			break;
		}
		if (READ(indes,(char *)&wavchunkhead,sizeof(struct wav_chunkhead_s))<0){
			PRINTF_ERR("cannot read WAV chunk");
			return -1;
		}
		bcount+=sizeof(struct wav_chunkhead_s);
		remain-=sizeof(struct wav_chunkhead_s);

		/* get csize */
		csize=wavchunkhead.csize[0]
			+(wavchunkhead.csize[1]<<8)
			+(wavchunkhead.csize[2]<<16)
			+(wavchunkhead.csize[3]<<24);
		if (remain<csize){
			break;
		}

		if ((searchtype==WAV_AKAIHEADTYPE_NONE)||(searchtype==WAV_AKAIHEADTYPE_SAMPLE900)){
			/* check if S900 sample header chunk */
			for (i=0;i<4;i++){
				if (WAV_CHUNKHEAD_AKAIS900SAMPLEHEADSTR[i]!=wavchunkhead.typestr[i]){
					break; /* not data chunk */
				}
			}
			if (i==4){
				/* found */
				type=WAV_AKAIHEADTYPE_SAMPLE900;
				break;
			}
		}

		if ((searchtype==WAV_AKAIHEADTYPE_NONE)||(searchtype==WAV_AKAIHEADTYPE_SAMPLE1000)){
			/* check if S1000 sample header chunk */
			for (i=0;i<4;i++){
				if (WAV_CHUNKHEAD_AKAIS1000SAMPLEHEADSTR[i]!=wavchunkhead.typestr[i]){
					break; /* not data chunk */
				}
			}
			if (i==4){
				/* found */
				type=WAV_AKAIHEADTYPE_SAMPLE1000;
				break;
			}
		}

		if ((searchtype==WAV_AKAIHEADTYPE_NONE)||(searchtype==WAV_AKAIHEADTYPE_SAMPLE3000)){
			/* check if S3000 sample header chunk */
			for (i=0;i<4;i++){
				if (WAV_CHUNKHEAD_AKAIS3000SAMPLEHEADSTR[i]!=wavchunkhead.typestr[i]){
					break; /* not data chunk */
				}
			}
			if (i==4){
				/* found */
				type=WAV_AKAIHEADTYPE_SAMPLE3000;
				break;
			}
		}

		if ((searchtype==WAV_AKAIHEADTYPE_NONE)||(searchtype==WAV_AKAIHEADTYPE_DDTAKE)){
			/* check if DD take header chunk */
			for (i=0;i<4;i++){
				if (WAV_CHUNKHEAD_AKAIDDTAKEHEADSTR[i]!=wavchunkhead.typestr[i]){
					break; /* not data chunk */
				}
			}
			if (i==4){
				/* found */
				type=WAV_AKAIHEADTYPE_DDTAKE;
				break;
			}
		}

		/* skip rest of chunk */
		if (LSEEK64(indes,(OFF64_T)csize,SEEK_CUR)<0){
			PERROR("cannot lseek WAV chunk");
			return -1;
		}
		bcount+=csize;
		remain-=csize;
	}
	if (type==WAV_AKAIHEADTYPE_NONE){ /* no AKAI header chunk found? */
		csize=0;
	}

	if (bcountp!=NULL){
		*bcountp=bcount;
	}
	if (csizep!=NULL){
		*csizep=csize;
	}
	return (int)type;
}
#endif



/* EOF */
