#ifndef __AKAIUTIL_FILE_H
#define __AKAIUTIL_FILE_H
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
//TODO: This is ugly and has to go. it seems not to be enough to wrap the includes in C** into the extern C :-(
#ifdef __cplusplus
extern "C" {
#endif


/* AKAI files */

/* Note: all data types are little endian */



/* AKAI file type ranges */
#define AKAI_S900_FTYPE_MIN		'A' /* min. file type for S900 */
#define AKAI_S900_FTYPE_MAX		'Z' /* max. file type for S900 */
#define AKAI_S1000_FTYPE_MIN	'a' /* min. file type for S1000 */
#define AKAI_S1000_FTYPE_MAX	'z' /* max. file type for S1000 */
#define AKAI_S3000_FTYPE_MIN	('a'+0x80) /* min. file type for S3000 */
#define AKAI_S3000_FTYPE_MAX	('z'+0x80) /* max. file type for S3000 */
/* Note: exception: AKAI_CDSETUP3000_FTYPE=='T' is within range of file types for S900 */



/* AKAI S900 sample header */
struct akai_sample900_s{
	char name[AKAI_NAME_LEN_S900]; /* sample name (in RAM) */
	u_char dummy1[6];   /* XXX 0x00 */
	u_char slen[4];     /* number of samples */
	u_char srate[2];    /* sample rate in Hz */
#define SAMPLE900_NPITCH_DEF		(60*16) /* default */
	u_char npitch[2];   /* nominal pitch in 1/16 semitones */
	u_char loud[2];     /* loudness offset (signed) */
#define SAMPLE900_PMODE_ONESHOT		'O'
#define SAMPLE900_PMODE_LOOP		'L'
#define SAMPLE900_PMODE_ALTLOOP		'A'
	u_char pmode;       /* playback mode */
	u_char dummy2;      /* XXX */
	u_char end[4];      /* end marker */
	u_char start[4];    /* start marker */
	u_char llen[4];     /* loop length */
	u_char dmadesa[2];  /* address of DMA descriptor list (updated by sampler) */
#define SAMPLE900_TYPE_NORM		0x00 /* normal */
#define SAMPLE900_TYPE_VELXF	0xff /* velocity crossfade */
	u_char type;        /* sample type */
#define SAMPLE900_DIR_NORM		'N'
#define SAMPLE900_DIR_REV		'R'
	u_char dir;         /* time direction */
	u_char dummy3[10];  /* XXX */
	u_char locat[4];    /* absolute address (updated by sampler) */
	u_char dummy4[2];  /* XXX */
}; /* Note: should be 0x003c Bytes */
/* Note: header is followed by sample data (in non-compressed or compressed sample format) */

/* S900 compressed sample format */
#define SAMPLE900COMPR_GROUP_SAMPNUM		10 /* number of samples per group */
#define SAMPLE900COMPR_UPBITNUM_NEGCODE_OFF	16 /* offset between bit number for upabsval and -code */
#define SAMPLE900COMPR_UPBITNUM_MAX			12 /* max. bit number to be used for upabsval */
#define SAMPLE900COMPR_INTERVALSIZ			(1<<SAMPLE900COMPR_UPBITNUM_MAX)  /* interval size for max. bit number */
#define SAMPLE900COMPR_BITMASK				(SAMPLE900COMPR_INTERVALSIZ-1)  /* bit mask for interval */
#define SAMPLE900COMPR_INTERVALSIZ2			(SAMPLE900COMPR_INTERVALSIZ>>1) /* half interval size */

#define AKAI_SAMPLE900_FTYPE	'S' /* file type */



/* AKAI S900 program header */
struct akai_program900_s{
	char name[AKAI_NAME_LEN_S900]; /* program name (in RAM) */
	u_char dummy1[6];   /* XXX 0x00 */
	u_char dummy2[2];   /* XXX */
#define PROGRAM900_KGA_NONE		0x0000 /* value for none */
	u_char kg1a[2];     /* address of keygroup 1 (updated by sampler) */
	u_char dummy3;      /* XXX */
	u_char kgxf;        /* keygroup (positional) crossfade enable */
	u_char dummy4;      /* XXX */
	u_char kgnum;       /* number of keygroups */
	u_char dummy5[14];  /* XXX */
}; /* Note: should be 0x0026 Bytes */
/* Note: header is followed by keygroups */

/* AKAI S900 program: keygroup */
struct akai_program900kg_s{
	u_char keyhi;       /* high MIDI key */
	u_char keylo;       /* low MIDI key */
#define PROGRAM900KG_VELSWTH_NOSOFT	0   /* value for no soft sample */
#define PROGRAM900KG_VELSWTH_NOLOUD	128 /* value for no loud sample */
	u_char velswth;     /* velocity switch threshold */
	u_char dummy1[15];  /* XXX */
#define PROGRAM900KG_FLAGS_PCONST	0x01 /* constant pitch enable */
#define PROGRAM900KG_FLAGS_VELXF	0x02 /* velocity crossfade enable */
#define PROGRAM900KG_FLAGS_ONESHOT	0x08 /* one-shot trigger mode enable */
	u_char flags;       /* flags */
#define PROGRAM900KG_OUTCH1_LEFT	0x08 /* code for LEFT */
#define PROGRAM900KG_OUTCH1_RIGHT	0x09 /* code for RIGHT */
#define PROGRAM900KG_OUTCH1_ANY		0xff /* code for ANY */
	u_char outch1;      /* audio output channel: channel number-1 or code */
	u_char midichoff;   /* MIDI channel offset */
	u_char dummy2[3];   /* XXX */
	u_char sname1[AKAI_NAME_LEN_S900]; /* sample name (in RAM) for sample 1 ("soft") */
	u_char dummy3[4];   /* XXX */
	u_char velxfv50;    /* if velocity crossfade: velocity value for 50% */
	u_char dummy4;      /* XXX */
#define PROGRAM900KG_SHDRA_NONE		0x0000 /* value for none */
	u_char shdra1[2];	/* address of sample header for sample 1 (updated by sampler) */
	u_char tune1[2];    /* tuning offset (transpose) in 1/16 semitones (signed) for sample 1 */
	u_char filter1;     /* filter for sample 1 */
	u_char loud1;       /* loudness offset (signed) for sample 1 */
	u_char sname2[AKAI_NAME_LEN_S900]; /* sample name (in RAM) for sample 2 ("loud") */
	u_char dummy5[6];   /* XXX */
	u_char shdra2[2];	/* address of sample header for sample 2 (updated by sampler) */
	u_char tune2[2];    /* tuning offset in 1/16 semitones (signed) for sample 2 */
	u_char filter2;     /* filter for sample 2 */
	u_char loud2;       /* loudness offset (signed) for sample 2 */
	u_char kgnexta[2];  /* address of next keygroup (updated by sampler) */
}; /* Note: should be 0x0046 Bytes */

#define AKAI_PROGRAM900_FTYPE	'P' /* file type */



/* AKAI S900/S950 drum settings descriptor */
struct akai_drum900des_s{
	u_char inpnr1;       /* input number-1 */
	u_char midich1;      /* MIDI channel-1 */
	u_char midinote;     /* MIDI note */
	u_char gain;         /* gain */
	u_char ask90_trig;   /* if ASK90: trigger threshold */
	                     /* if ME35T: (ignored) */
	u_char dummy1[3];    /* XXX */
#define DRUM900DES_ITYPE_ASK90		0x00 /* interface type: ASK90 */
#define DRUM900DES_ITYPE_ME35T		0xff /* interface type: ME35T */
	u_char itype;        /* interface type */
	u_char me35t_trig;   /* if ASK90: (ignored) */
	                     /* if ME35T: trigger threshold */
	u_char me35t_vcurv1; /* if ASK90: (ignored) */
	                     /* if ME35T: "V-curve"-1 */
	u_char dummy2;       /* XXX */
	u_char captt4[2];    /* 4* capture time in msec */
	u_char ontime4[2];   /* 4* on-time in msec */
	u_char recovt4[2];   /* 4* recovery time in msec */
	u_char dummy3[8];
	u_char ask90_resba[2]; /* if ASK90: ASK90 base address for CH RESET */
	                       /* if ME35T: (ignored) */
	u_char ask90_adcla[2]; /* if ASK90: ASK90 address for ADC latch */
	                       /* if ME35T: (ignored) */
}; /* Note: should be 0x001e Bytes */

/* AKAI S900/S950 drum settings file */
struct akai_drum900_s{
	u_char ask90enab;  /* if ASK90: ASK90 enable */
	                   /* if ME35T: (ignored) */
	u_char ask90sens;  /* if ASK90: ASK90 sensitivity */
	                   /* if ME35T: (ignored) */
	u_char dummy1[20]; /* XXX 0x00 ... 0x00 */
#define DRUM900_INPUTNUM		8 /* number of drum trigger inputs */
	struct akai_drum900des_s ddes[DRUM900_INPUTNUM]; /* drum setting descriptors */
}; /* Note: should be 0x0106 Bytes */

#define AKAI_DRUM900_FTYPE		'D' /* file type */
#define AKAI_DRUM900_FNAME		"DRUM SET  " /* default file name */



/* AKAI S900 overall settings file */
struct akai_ovs900_s{
	char progname[AKAI_NAME_LEN_S900]; /* selected program name */
	u_char dummy1[6];       /* XXX 0x20 ... 0x20 */
	u_char dummy2[4];       /* XXX */
	u_char testmidich[2];   /* test MIDI channel */
	u_char testmidikey[2];  /* test MIDI key */
	u_char testmidivel[2];  /* test MIDI velocity */
	u_char dummy3; /* XXX */
#define OVS900_BMIDICH1MASK		0x7f /* bit mask for basic MIDI channel-1 */
#define OVS900_OMNIMASK			0x80 /* bit mask for OMNI flag */
	u_char bmidich1omni;    /* basic MIDI channel-1 | OMNI flag */
	u_char loudness;        /* loudness enable */
#define OVS900_CTRLPORT_MIDI	0x01 /* control port: MIDI */
#define OVS900_CTRLPORT_RS232	0x02 /* control port: RS232 */
	u_char ctrlport;        /* control port */
	u_char progchange;      /* program change enable */
	u_char dummy4[4];       /* XXX */
	u_char pwheelrange;     /* pitch wheel range */
	u_char rs232brate10[2]; /* RS232 baudrate/10 */
	u_char dummy5[2];       /* XXX */
}; /* Note: should be 0x0028 Bytes */

#define AKAI_OVS900_FTYPE		'O' /* file type */
#define AKAI_OVS900_FNAME		"OVERALL SE" /* default file name */



/* AKAI S900 fixup file */
#define AKAI_FIXUP900_FTYPE		'F' /* file type */

/* AKAI S900 memory image file */
#define AKAI_MEMIMG900_FTYPE	'M' /* file type */



/* AKAI S1000 empty file name */
#define AKAI_EMPTY1000_FNAME	"VVVVVVVVVVVV" /* S1000 empty file name */



/* AKAI S1000 generic header */
struct akai_genfilehdr_s{
	u_char blockid;   /* block ID */
	u_char dummy1[2]; /* XXX */
	u_char name[AKAI_NAME_LEN]; /* file name (in RAM) */
}; /* Note: should be 0x000f Bytes */



/* AKAI S1000 sample loop parameters */
struct akai_sample1000loop_s{
	u_char at[4];     /* loop at marker */
	u_char flen[2];   /* loop fine length in 1/65536 sample */
	u_char len[4];    /* loop length in samples */
#define SAMPLE1000LOOP_TIME_NOLOOP	0    /* value for NOLOOP */
#define SAMPLE1000LOOP_TIME_HOLD	9999 /* value for HOLD */
	u_char time[2];   /* loop time in msec */
};

/* AKAI S1000 sample header */
struct akai_sample1000_s{
#define SAMPLE1000_BLOCKID		0x03
	u_char blockid;      /* block ID */
#define SAMPLE1000_BANDW_10KHZ		0x00
#define SAMPLE1000_BANDW_20KHZ		0x01
	u_char bandw;       /* bandwidth select */
	u_char rkey;        /* MIDI root key (typ. 0x3c) */
	u_char name[AKAI_NAME_LEN]; /* sample name (in RAM) */
	u_char dummy1;      /* XXX 0x80 */
	u_char lnum;        /* number of loops */
	u_char lfirst;      /* first active loop-1 */
	u_char dummy2;      /* XXX 0x00 */
#define SAMPLE1000_PMODE_LOOP		0x00
#define SAMPLE1000_PMODE_LOOPNOTREL	0x01
#define SAMPLE1000_PMODE_NOLOOP		0x02
#define SAMPLE1000_PMODE_TOEND		0x03
	u_char pmode;       /* playback mode */
	u_char ctune;       /* cents tune (signed) */
	u_char stune;       /* semitone tune (signed) */
	u_char locat[4];    /* absolute address (updated by sampler) */
	u_char slen[4];     /* number of samples */
	u_char start[4];    /* start marker */
	u_char end[4];      /* end marker */
#define AKAI_SAMPLE1000_LOOPNUM	8
	struct akai_sample1000loop_s loop[AKAI_SAMPLE1000_LOOPNUM];
	u_char dummy3[2];   /* XXX 0x00 0x00 */
#define AKAI_SAMPLE1000_STPAIRA_NONE	0xffff /* value for none */
	u_char stpaira[2];  /* address of sample header of partner in stereo pair (updated by sampler) */
	u_char srate[2];    /* sample rate in Hz */
	u_char hltoff;      /* HOLD loop tune offset */
	u_char dummy4[9];   /* XXX 0x00 ... 0x00 */
}; /* Note: should be 0x0096 Bytes */
/* Note: header is followed by sample data */

#define AKAI_SAMPLE1000_FTYPE	's' /* file type */



/* AKAI S3000 sample header */
struct akai_sample3000_s{
#define SAMPLE3000_BLOCKID		SAMPLE1000_BLOCKID
	struct akai_sample1000_s s1000;
	u_char dummy1[42];  /* XXX 0x00 ... 0x00 */
}; /* Note: should be 0x00c0 Bytes */
/* Note: header is followed by sample data */

/* address multiplicand */
#define SAMPLE3000_STPAIRA_MULT	0x10

#define AKAI_SAMPLE3000_FTYPE	('s'+0x80) /* file type */



/* AKAI CD3000 CD-ROM sample parameters file */
/* Note: contains S3000 sample header */
#define AKAI_CDSAMPLE3000_FTYPE	('h'+0x80) /* file type */



/* AKAI S1000 program header */
struct akai_program1000_s{
#define PROGRAM1000_BLOCKID		0x01
	u_char blockid;      /* block ID */
#define PROGRAM1000_KGA_NONE	0x0000 /* value for none */
	u_char kg1a[2];      /* address of keygroup 1 (updated by sampler) */
	u_char name[AKAI_NAME_LEN]; /* program name (in RAM) */
	u_char dummy1;       /* XXX */
#define PROGRAM1000_MIDICH1_OMNI	0xff /* code for OMNI */
	u_char midich1;      /* MIDI channel number-1 or code */
	u_char dummy2[2];    /* XXX */
	u_char keylo;        /* low MIDI key */
	u_char keyhi;        /* high MIDI key */
	u_char oct;          /* octave offset (signed) */
#define PROGRAM1000_AUXCH1_OFF		0xff /* code for OFF */
	u_char auxch1;        /* aux output channel number-1 or code */
	u_char dummy3[18];   /* XXX */
	u_char kgxf;         /* keygroup crossfade enable */
	u_char kgnum;        /* number of keygroups */
	u_char dummy4[107];  /* XXX */
}; /* Note: should be 0x0096 Bytes */
/* Note: header is followed by keygroups */

/* AKAI S1000 program keygroup: velocity zone */
struct akai_program1000kgvelzone_s{
	u_char sname[AKAI_NAME_LEN]; /* sample name (in RAM) */
	u_char vello;       /* low MIDI velocity */
	u_char velhi;       /* high MIDI velocity */
	u_char ctune;       /* cents tune offset (signed) */
	u_char stune;       /* semitone tune offset (signed) */
	u_char loud;        /* loudness offset (signed) */
	u_char filter;      /* filter offset (signed) */
	u_char pan;         /* pan offset (signed) */
#define PROGRAM1000_PMODE_SAMPLE		0x00
#define PROGRAM1000_PMODE_LOOP			0x01
#define PROGRAM1000_PMODE_LOOPNOTREL	0x02
#define PROGRAM1000_PMODE_NOLOOP		0x03
#define PROGRAM1000_PMODE_TOEND			0x04
	u_char pmode;       /* playback mode */
	u_char dummy1[2];   /* XXX */
#define PROGRAM1000KG_SHDRA_NONE		0xffff /* value for none */
	u_char shdra[2];	/* address of sample header (updated by sampler) */
}; /* Note: should be 0x0018 Bytes */

/* AKAI S1000 program: keygroup */
struct akai_program1000kg_s{
#define PROGRAM1000KG_BLOCKID	0x02
	u_char blockid;      /* block ID */
	u_char kgnexta[2];   /* address of next keygroup (updated by sampler) */
	u_char keylo;        /* low MIDI key */
	u_char keyhi;        /* high MIDI key */
	u_char ctune;        /* cents tune offset (signed) */
	u_char stune;        /* semitone tune offset (signed) */
	u_char filter;       /* filter */
	u_char dummy1[22];   /* XXX */
	u_char velxf;        /* velocity crossfade enable */
	u_char dummy2[3];    /* XXX */
#define PROGRAM1000KG_VELZONENUM	4 /* number of velocity zones per keygroup */
	struct akai_program1000kgvelzone_s velzone[PROGRAM1000KG_VELZONENUM]; /* velocity zones */
	u_char dummy3[2];    /* XXX */
	u_char pconst[PROGRAM1000KG_VELZONENUM]; /* constant pitch enable for velocity zones */
	u_char auxchoff[PROGRAM1000KG_VELZONENUM]; /* aux output channel offsets for velocity zones */
	u_char dummy4[10];   /* XXX */
}; /* Note: should be 0x0096 Bytes */

#define AKAI_PROGRAM1000_FTYPE	'p' /* file type */



/* AKAI S3000 program header */
struct akai_program3000_s{
#define PROGRAM3000_BLOCKID		PROGRAM1000_BLOCKID
	struct akai_program1000_s s1000;
	u_char dummy1[42];  /* XXX */
}; /* Note: should be 0x00c0 Bytes */
/* Note: header is followed by keygroups */

/* AKAI S3000 program: keygroup */
struct akai_program3000kg_s{
#define PROGRAM3000KG_BLOCKID	PROGRAM1000KG_BLOCKID
	struct akai_program1000kg_s s1000;
	u_char dummy1[42];  /* XXX */
}; /* Note: should be 0x00c0 Bytes */

/* address multiplicands */
#define PROGRAM3000_KGA_MULT	0x10
#define PROGRAM3000_SHDRA_MULT	0x10

#define AKAI_PROGRAM3000_FTYPE	('p'+0x80) /* file type */



/* AKAI S1000 or S3000 drum settings descriptor */
struct akai_drum1000des_s{
	u_char midich1;  /* MIDI channel-1 */
	u_char midinote; /* MIDI note */
	u_char sens;     /* sensitivity level */
	u_char trig;     /* trigger level */
	u_char vcurv1;   /* "V-curve"-1 */
	u_char captt;    /* capture time in msec */
	u_char recovt;   /* recovery time in msec */
	u_char ontime;   /* on-time in msec */
	u_char dummy1;   /* XXX */
}; /* Note: should be 0x0009 Bytes */

/* AKAI S1000 or S3000 drum settings file */
struct akai_drum1000_s{
#define DRUM1000_BLOCKID		0x01
	u_char blockid;   /* block ID */
	u_char dummy1[2]; /* XXX */
	u_char name[AKAI_NAME_LEN]; /* drum settings name (in RAM) */
#define DRUM1000_UNITNUM		2 /* number of units */
#define DRUM1000_INPUTNUM		8 /* number of drum trigger inputs per unit */
	struct akai_drum1000des_s ddes1[DRUM1000_INPUTNUM]; /* drum setting descriptors for unit 1 */
	u_char dummy2[3]; /* XXX */
	struct akai_drum1000des_s ddes2[DRUM1000_INPUTNUM]; /* drum setting descriptors for unit 2 */
}; /* Note: should be 0x00a2 Bytes */

#define AKAI_DRUM1000_FTYPE		'd' /* file type */
#define AKAI_DRUM1000_FNAME		"DRUM INPUTS " /* default file name */



/* AKAI S1100 or S3000 or S3000XL effects file */
/* Note: effects files of different system types are incompatible */
#define AKAI_FXFILE_FTYPE		'x' /* file type */
#define AKAI_FXFILE_FNAME		"EFFECTS FILE" /* default file name */
#define AKAI_FXFILE_FSIZE		0x1c90 /* default file size */
#define FXFILE1100_BLOCKID		0x00
#define FXFILE3000_BLOCKID		0x00
#define FXFILE3000XL_BLOCKID	0x02



/* AKAI S1100 or S3000 cue/take-list header */
struct akai_cuelist_s{
#define CUELIST_BLOCKID		0x00
	u_char blockid;      /* block ID */
	u_char dummy1[2];    /* XXX */
	u_char name[AKAI_NAME_LEN]; /* cue/take-list name (in RAM) */
	u_char dummy2[7];    /* XXX */
	u_char cuenum;       /* number of cues */
	u_char dummy3[105];  /* XXX */
}; /* Note: should be 0x0080 Bytes */
/* Note: header is followed by cues */

/* AKAI S1100 or S3000 cue */
/* Note: depending on file/cue type, some struct elements are ignored */
struct akai_cue_s{
	u_char name[AKAI_NAME_LEN]; /* program or take name */
	u_char time[5];      /* time (BCD format) */
#define AKAI_CUETYPE_MOFF	0x00 /* program: MIDI note off */
#define AKAI_CUETYPE_MON	0x01 /* program: MIDI note on */
#define AKAI_CUETYPE_TOFF	0x02 /* DD take: off */
#define AKAI_CUETYPE_TON	0x03 /* DD take: on */
	u_char type;         /* cue type */
	u_char midinote;     /* MIDI note */
	u_char midivel;      /* MIDI velocity */
	u_char pan;          /* panning (signed) */
	u_char stlvl;        /* stereo output level */
	u_char midich1;      /* MIDI channel-1 */
	u_char repeat1;      /* repeat count-1 */
	u_char fadein[2];    /* fade in time in ms */
	u_char fadeout[2];   /* fade out time in ms */
	u_char dummy1[4];    /* XXX */
}; /* Note: should be 0x0020 Bytes */

#define AKAI_QLFILE_FTYPE		'q' /* cue-list file type */
#define AKAI_QLFILE_FNAME		"QL1         " /* default cue-list file name */

#define AKAI_TLFILE_FTYPE		't' /* take-list file type */
#define AKAI_TLFILE_FNAME		"TL1         " /* default take-list file name */



/* AKAI S3000 multi file */
#define AKAI_MULTI3000_FTYPE	('m'+0x80) /* file type */
#define AKAI_MULTI3000_FNAME	"MULTI FILE  " /* default file name */
#define MULTI3000_BLOCKID		0x00



/* AKAI S1000 operating system file */
#define AKAI_SYS1000_FTYPE	'c' /* file type */

/* AKAI S3000 operating system file */
#define AKAI_SYS3000_FTYPE	('c'+0x80) /* file type */



/* AKAI CD3000 CD-ROM setup header */
/* Note: generic file header does not apply for this file type! */
struct akai_cdsetup3000_s{
	u_char name[AKAI_NAME_LEN]; /* CD-ROM setup name (in RAM) */
	u_char dummy1[3]; /* XXX */
	u_char cdlabel[AKAI_NAME_LEN]; /* CD-ROM label */
}; /* Note: should be 0x006b Bytes */
/* followed by marked file entries */

/* marked file entry */
struct akai_cdsetup3000_entry_s{
	u_char parti;    /* partition index */
	u_char voli;     /* volume index */
	u_char filei[2]; /* file index */
}; /* Note: should be 4 Bytes */
/* Note: unused entry is 0xff 0xff 0xff 0xff */

#define AKAI_CDSETUP3000_FTYPE	'T' /* file type */
/* Note: AKAI_CDSETUP3000_FTYPE is within range of file types for S900 */
#define AKAI_CDSETUP3000_FNAME	"NEW CD SETUP" /* default file name */



/* Declarations */

extern int akai_file_info(struct file_s *fp,int verbose);
extern void akai_sample_info(struct file_s *fp,u_char *hdrp);
extern int akai_program_info(struct file_s *fp);
extern int akai_cuelist_info(struct file_s *fp);
extern int akai_takelist_info(struct file_s *fp);
extern int akai_drum1000_info(struct file_s *fp);
extern int akai_drum900_info(struct file_s *fp);
extern int akai_ovs900_info(struct file_s *fp);
extern int akai_cdsetup3000_info(struct file_s *fp);

extern int akai_fixramname(struct file_s *fp);

extern int akai_s900comprfile_updateuncompr(struct file_s *fp);

extern u_int akai_sample900_getsamplesize(struct file_s *fp);

extern void akai_sample900noncompr_sample2wav(u_char *sbuf,u_char *wavbuf,u_int samplecountpart);
extern void akai_sample900noncompr_wav2sample(u_char *sbuf,u_char *wavbuf,u_int samplecountpart);

extern u_int akai_sample900compr_getbits(u_char *buf,u_int bitpos,u_int bitnum);
extern u_int akai_sample900compr_sample2wav(u_char *sbuf,u_char *wavbuf,u_int sbufsiz,u_int wavbufsiz);
extern void akai_sample900compr_setbits(u_char *buf,u_int bitpos,u_int bitnum,u_int val);
extern int akai_sample900compr_wav2sample(u_char *sbuf,u_char *wavbuf,u_int samplecountpart);

extern int akai_sample900_noncompr2compr(struct file_s *fp,struct vol_s *volp);
extern int akai_sample900_compr2noncompr(struct file_s *fp,struct vol_s *volp);

#define SAMPLE2WAV_CHECK		1
#define SAMPLE2WAV_EXPORT		2
#define SAMPLE2WAV_CREATE		4
#define SAMPLE2WAV_ALL			0xff
extern int akai_sample2wav(struct file_s *fp,int wavfd,u_int *sizep,char **wavnamep,int what);

#define WAV2SAMPLE_OPEN			1
#define WAV2SAMPLE_OVERWRITE	2
extern int akai_wav2sample(int wavfd,char *wavname,struct vol_s *volp,u_int findex,
						   u_int type,int s9cflag,u_int osver,u_char *tagp,
						   u_int *bcountp,int what);


}
#endif /* !__AKAIUTIL_FILE_H */
