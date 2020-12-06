#ifndef __AKAIUTIL_WAV_H
#define __AKAIUTIL_WAV_H
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



/* RIFF WAVE format header */
/* Note: WAV-file is little-endian!!! */

struct wav_riffhead_s{
/* begin RIFFWAVEChunk */
#define WAV_RIFFHEAD_RIFFSTR "RIFF"
	char riffstr[4];   /* must be "RIFF" */
	u_char fsize[4];   /* following file-size in Bytes */
#define WAV_RIFFHEAD_WAVESTR "WAVE"
	char wavestr[4];   /* must be "WAVE" */
/* end RIFFWAVEChunk */
};

struct wav_chunkhead_s{
	char typestr[4];   /* chunk type */
	u_char csize[4];   /* following chunk-size in Bytes */
};
#define WAV_CHUNKHEAD_FMTSTR	"fmt "
#define WAV_CHUNKHEAD_DATASTR	"data"

struct wav_fmthead_s{
/* continued FormatChunk */
#define WAV_HEAD_FTAG_PCM 0x0001 /* 1: PCM/umcompressed */
	u_char ftag[2];    /* format tag */
	u_char chnr[2];    /* number of channels */
	u_char srate[4];   /* sample rate in samples/sec */
	u_char drate[4];   /* total data rate in Bytes/sec */
	u_char balign[2];  /* number of Bytes per sample * chnr */
	u_char bitnr[2];   /* number of Bits per sample */
/* end FormatChunk */
};



#define WAV_HEAD_SIZE	(sizeof(struct wav_riffhead_s)+sizeof(struct wav_chunkhead_s)+sizeof(struct wav_fmthead_s)+sizeof(struct wav_chunkhead_s))



#ifndef WAV_AKAIHEAD_DISABLE
/* AKAI header chunks */
#define WAV_CHUNKHEAD_AKAIS900SAMPLEHEADSTR		"S9H "
#define WAV_CHUNKHEAD_AKAIS1000SAMPLEHEADSTR	"S1H "
#define WAV_CHUNKHEAD_AKAIS3000SAMPLEHEADSTR	"S3H "
#define WAV_CHUNKHEAD_AKAIDDTAKEHEADSTR			"TKH "
#endif



/* Declarations */

extern int wav_write_head(int outdes,
						  u_int datasize,u_int chnr,u_int samprate,u_int bitnr,
						  u_int extrasize);

extern int wav_read_head(int indes,u_int *bcountp,
						 u_int *datasizep,u_int *chnrp,u_int *sampratep,u_int *bitnrp,
						 u_int *extrasizep,
						 char **errstrp);

#ifndef WAV_AKAIHEAD_DISABLE
#define WAV_AKAIHEADTYPE_NONE			0x00
#define WAV_AKAIHEADTYPE_SAMPLE900		0x09
#define WAV_AKAIHEADTYPE_SAMPLE1000		0x01
#define WAV_AKAIHEADTYPE_SAMPLE3000		0x03
#define WAV_AKAIHEADTYPE_DDTAKE			0xdd
int wav_find_akaihead(int indes,u_int *bcountp,u_int *csizep,u_int remain,u_int searchtype);
#endif



#endif /* !__AKAIUTIL_WAV_H */
