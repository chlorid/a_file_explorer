#ifndef __AKAIUTIL_H
#define __AKAIUTIL_H

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

//TODO: This is ugly and has to go. it seems not to be enough to wrap the includes in C** into the extern C :-(
#ifdef __cplusplus
extern "C" {
#endif



/* AKAI S900/S1000/S3000 filesystems */

/* Note: all data types are little endian */



/* floppy */

/* floppy filesystem blocksize */
#define AKAI_FL_BLOCKSIZE	0x0400 /* 1KB */
/* Note: must be == AKAI_FL_SECSIZE (low-level floppy sector size, defined in akaiutil_io.h) */
#if defined(AKAI_FL_SECSIZE)&&(AKAI_FL_BLOCKSIZE!=AKAI_FL_SECSIZE)
#error "AKAI_FL_BLOCKSIZE!=AKAI_FL_SECSIZE"
#endif

#define AKAI_FLL_SIZE		0x0320 /* low-density floppy size in floppy blocks (800KB) */
#define AKAI_FLH_SIZE		0x0640 /* high-density floppy size in floppy blocks (1.6MB) */



/* harddisk */

/* harddisk filesystem blocksize */
#define AKAI_HD_BLOCKSIZE	0x2000 /* 8KB */

/* S900 harddisk */
#define AKAI_HD9_MAXSIZE	0x1fff /* max. harddisk size in harddisk blocks (approx. 64MB) */
#define AKAI_HD9_DEFSIZE	0x09c4 /* default harddisk size in harddisk blocks (approx. 19.5MB) */

/* S1000/S3000 harddisk */
#define AKAI_HD_MAXSIZE		0xffff /* max. harddisk size in harddisk blocks (approx. 512MB, Note: for 16bit block numbers) */

/* S1000/S3000 harddisk sampler partition */
#define AKAI_PART_MAXSIZE	0x1e00 /* max. partition size in harddisk blocks (60MB) */
#define AKAI_PART_NUM		18 /* max. number of sampler partitions per harddisk */

/* S1100/S3000 harddisk DD partition */
#define AKAI_DDPART_CBLKS	0x20 /* number of blocks per DD partition cluster */
#define AKAI_DDPART_NUM		18 /* max. number of DD partitions per harddisk */



/* max. disk size in bytes, Note: fits into 32bit */
#define AKAI_DISKSIZE_MAX	(AKAI_HD_MAXSIZE*AKAI_HD_BLOCKSIZE)



/* names */

#define AKAI_NAME_LEN_S900	10 /* for S900 */
#define AKAI_NAME_LEN		12 /* for S1000/S3000 */



/* FAT (for floppies and harddisk sampler partitions) */

#define AKAI_FAT_CODE_FREE			0x0000 /* free block */
#define AKAI_FAT_CODE_BAD			0x2000 /* bad block */
#define AKAI_FAT_CODE_SYS900FL		0x0000 /* block reserved for system (S900 floppy), warning: same as for free block!!! */
#define AKAI_FAT_CODE_SYS900HD		0xffff /* block reserved for system (S900 harddisk) */
#define AKAI_FAT_CODE_SYS			0x4000 /* block reserved for system (S1000/S3000) */
#define AKAI_FAT_CODE_DIREND900HD	0x8000 /* end of chain for volume directory (S900 harddisk) */
#define AKAI_FAT_CODE_DIREND1000HD	0x4000 /* end of chain for volume directory (S1000 harddisk), warning: same as for system block!!! */
#define AKAI_FAT_CODE_DIREND3000	0x8000 /* end of chain for volume directory (S3000) */
#define AKAI_FAT_CODE_FILEEND900	0x8000 /* end of chain for file (S900) */
#define AKAI_FAT_CODE_FILEEND		0xc000 /* end of chain for file (S1000/S3000) */



/* DD FAT (for S1100/S3000 harddisk DD partition) */

#define AKAI_DDFAT_CODE_FREE		0x0000 /* free cluster */
#define AKAI_DDFAT_CODE_SYS			0x8000 /* cluster reserved for system (header cluster) */
#define AKAI_DDFAT_CODE_END			0xffff /* end of chain */



/* OS versions for volumes and files */

#define AKAI_OSVER_S900VOL			0x0000 /* OS version of S900/S950 volume (zero) */
#define AKAI_OSVER_S1000MAX			0x0428 /* max. OS version of S1000 ("4.40") */
#define AKAI_OSVER_S1100MAX			0x091e /* max. OS version of S1100 ("9.30") */
#define AKAI_OSVER_S3000MAX			0x1100 /* max. OS version of S3000 ("17.00") */



/* file */

#define AKAI_FILE_SIZEMAX	0xffffff /* max. file size in bytes (approx. 16MB, Note: for 24bit size in volume directory entry for file) */

/* entry in volume directory for file */
struct akai_voldir_entry_s{
	/* Note: S900 uses first AKAI_NAME_LEN900 chars in name, rest is zero */
	u_char name[AKAI_NAME_LEN];  /* file name */
#define AKAI_FILE_TAGNUM	0x04 /* number of tags in volume directory entry for file */
#define AKAI_FILE_TAGFREE	0x00 /* invalid tag number, means: free tag entry for S3000 */
#define AKAI_FILE_TAGS1000	0x20 /* invalid tag number, default for S1000 */
	/* Note: valid tag numbers are 1, ..., AKAI_PARTHEAD_TAGNUM */
	/* Note: S900 has no tags, all zero */
	u_char tag[AKAI_FILE_TAGNUM]; /* tags */
#define AKAI_FTYPE_FREE		0x00 /* invalid file type, means: free entry in volume directory */
	u_char type; /* file type */
	u_char size[3];  /* file size in bytes (Note: 24bit) */
	u_char start[2]; /* start block within partition */
	u_char osver[2]; /* if S1000/S3000: OS version */
					 /* if S900 compressed file: number of un-compressed floppy blocks */
					 /* else: zero */
}; /* Note: should be 0x0018 Bytes */



/* volume parameters */
/* Note: S900 has no volume parameters */
struct akai_volparam_s{
	u_char progselmidich1; /* program select MIDI channel-1 */
	u_char progselomni;    /* MIDI OMNI for program select */
	u_char progselenab;    /* program select enable */
	u_char prognr1;        /* selected program number-1 */
	u_char playinominovrr; /* MIDI OMNI override for play input */
	u_char sysexch1;       /* MIDI system exclusive channel-1 */
	u_char dummy1[42];     /* XXX */
}; /* Note: should be 0x0030 Bytes */



/* harddisk volumes */

/* harddisk volume directory of files */
#define AKAI_VOLDIR_ENTRIES_S900HD		128 /* total number of volume directory entries for S900 */
#define AKAI_VOLDIR_ENTRIES_S1000HD		126 /* total number of volume directory entries for S1000 */
#define AKAI_VOLDIR_ENTRIES_S3000HD		510 /* total number of volume directory entries for S3000 */
#define AKAI_VOLDIR_ENTRIES_1BLKHD		341 /* max. number of volume directory entries in 1 harddisk block */

/* S900 harddisk volume */
struct akai_voldir900hd_s{
	struct akai_voldir_entry_s file[AKAI_VOLDIR_ENTRIES_S900HD]; /* volume directory entries for files */
	u_char dummy1[0x1400]; /* XXX */
}; /* Note: should be 1 harddisk block */
#define AKAI_VOLDIR900HD_BLKS		1

/* S1000 harddisk volume */
struct akai_voldir1000hd_s{
	struct akai_voldir_entry_s file[AKAI_VOLDIR_ENTRIES_S1000HD]; /* volume directory entries for files */
	struct akai_volparam_s param; /* volume parameters */
	u_char dummy1[0x1400]; /* XXX */
}; /* Note: should be 1 harddisk block */
#define AKAI_VOLDIR1000HD_BLKS		1

/* S3000 harddisk volume */
struct akai_voldir3000hd_s{
	struct akai_voldir_entry_s file[AKAI_VOLDIR_ENTRIES_S3000HD]; /* volume directory entries for files */
	struct akai_volparam_s param; /* volume parameters */
	u_char dummy1[0x1000]; /* XXX */
}; /* Note: should be 2 harddisk blocks */
#define AKAI_VOLDIR3000HD_BLKS		2



/* floppy volumes */

/* floppy volume directory of files */
#define AKAI_VOLDIR_ENTRIES_S1000FL		64  /* number of volume directory entries total for S900 and S1000 floppy */
#define AKAI_VOLDIR_ENTRIES_S3000FL		510 /* number of volume directory entries total for S3000 floppy */

/* floppy volume label */
/* Note: S900 has no floppy volume label, all zero */
struct akai_flvol_label_s{
	u_char name[AKAI_NAME_LEN];   /* volume name */
	u_char dummy1[2];             /* XXX */
	u_char osver[2];              /* OS version */
	struct akai_volparam_s param; /* volume parameters */
}; /* Note: should be 0x0040 Bytes */

/* low-density floppy header */
/* Note: S900 and S1000 floppy volume directory is within floppy header */
struct akai_fllhead_s{
	struct akai_voldir_entry_s file[AKAI_VOLDIR_ENTRIES_S1000FL]; /* volume directory entries for files */
#define AKAI_FAT_ENTRIES_FLL	AKAI_FLL_SIZE /* number of FAT entries */
	u_char fatblk[AKAI_FAT_ENTRIES_FLL][2]; /* FAT entries for floppy blocks: next block or special code */
	/* Note: S900 has no floppy volume label, all zero */
	struct akai_flvol_label_s label; /* label */
	u_char dummy1[0x0380]; /* XXX */
}; /* Note: should be 4 floppy blocks */
#define AKAI_FLLHEAD_BLKS	4

/* high-density floppy header */
/* Note: S900 and S1000 floppy volume directory is within floppy header */
/* Note: low- and high-density floppy headers are identical */
/*       up to first AKAI_FAT_ENTRIES_FLL FAT-entries */
struct akai_flhhead_s{
	struct akai_voldir_entry_s file[AKAI_VOLDIR_ENTRIES_S1000FL]; /* volume directory entries for files */
#define AKAI_FAT_ENTRIES_FLH	AKAI_FLH_SIZE /* number of FAT entries */
	u_char fatblk[AKAI_FAT_ENTRIES_FLH][2]; /* FAT entries for floppy blocks: next block or special code */
	/* Note: S900 has no floppy volume label, all zero */
	struct akai_flvol_label_s label; /* label */
	u_char dummy1[0x0140]; /* XXX */
}; /* Note: should be 5 floppy blocks */
#define AKAI_FLHHEAD_BLKS	5

/* S3000 floppy volume directory (behind header) */
struct akai_voldir3000fl_s{
	struct akai_voldir_entry_s file[AKAI_VOLDIR_ENTRIES_S3000FL]; /* volume directory entries for files */
	u_char dummy1[0x0030]; /* Note: volume parameters are in floppy header */
}; /* Note: should be 12 floppy blocks */
#define AKAI_VOLDIR3000FL_BLKS		12
#define AKAI_VOLDIR3000FLL_BSTART	4 /* start block for low-density */
#define AKAI_VOLDIR3000FLH_BSTART	5 /* start block for high-density */
/* flag for S3000 floppy: invalid type of first file in floppy header */
#define AKAI_VOLDIR3000FL_FTYPE		0xff /* invalid file type, used as flag for S3000 floppy volume */



/* generic volume */
/* Note: first file starts at byte 0 in first block */
union akai_voldir_u{
	struct akai_voldir900hd_s s900hd;
	struct akai_voldir1000hd_s s1000hd;
	struct akai_voldir3000hd_s s3000hd;
	/* Note: S900 and S1000 floppy volume is within floppy header */
	struct akai_voldir3000fl_s s3000fl;
};



/* entry in root directory for volume on S900 harddisk */
struct akai_hd9rootdir_entry_s{
	u_char name[AKAI_NAME_LEN_S900]; /* volume name */
#define AKAI_VOL_START_INACT		0x0000 /* inactive */
	u_char start[2]; /* start block on harddisk */
}; /* Note: should be 0x000c Bytes */



/* S900 harddisk header */
struct akai_hd9head_s{
#define AKAI_HD9ROOTDIR_ENTRIES		128 /* number of root directory entries for volumes */
	struct akai_hd9rootdir_entry_s vol[AKAI_HD9ROOTDIR_ENTRIES]; /* root directory for volumes */
	u_char size[2]; /* harddisk size in harddisk blocks */
#define AKAI_HD9FLAG1_SIZEVALID		0xff /* if size[] is valid */
	u_char flag1; /* XXX */
	u_char flag2; /* XXX */
#define AKAI_HD9FAT0_ENTRIES	0x0cfe /* number of copied FAT entries */
	u_char fatblk0[AKAI_HD9FAT0_ENTRIES][2]; /* copy of first FAT entries */
#define AKAI_HD9FAT_ENTRIES		AKAI_HD9_MAXSIZE /* max. number of FAT entries */
	u_char fatblk[AKAI_HD9FAT_ENTRIES][2]; /* FAT entries for harddisk blocks: next block or special code */
	u_char dummy1[0x6000-AKAI_HD9FAT_ENTRIES*2]; /* XXX */
}; /* Note: should be 4 harddisk blocks */
#define AKAI_HD9HEAD_BLKS	4



/* entry in root directory for volume in S1000/S3000 harddisk sampler partition */
struct akai_rootdir_entry_s{
	u_char name[AKAI_NAME_LEN]; /* volume name */
#define AKAI_VOL_TYPE_INACT		0x00 /* inactive */
#define AKAI_VOL_TYPE_S1000		0x01 /* S1000 */
#define AKAI_VOL_TYPE_S3000		0x03 /* S3000 or CD3000 */
#define AKAI_VOL_TYPE_CD3000	0x07 /* CD3000 CD-ROM (compatible with AKAI_VOL_TYPE_S3000) */
	u_char type; /* volume type */
#define AKAI_VOL_LNUM_OFF	0
#define AKAI_VOL_LNUM_MIN	1
#define AKAI_VOL_LNUM_MAX	128
	u_char lnum; /* load number */
	u_char start[2]; /* start block within partition */
}; /* Note: should be 0x0010 Bytes */



/* S1000/S3000 harddisk sampler and DD partition table (only in first harddisk sampler partition) */
struct akai_parttab_s{
#define AKAI_PARTTAB_MAGICNUM	128 /* number of fields */
#define AKAI_PARTTAB_MAGICVAL	9999 /* magic base value */
	u_char magic[AKAI_PARTTAB_MAGICNUM][2]; /* magic fields, indicate partition table, only in first partition */
	u_char partnum;   /* number of sampler partitions, only in first sampler partition */
	u_char ddpartnum; /* number of DD partitions */
	u_char part[AKAI_PART_NUM+1][2];     /* sampler partition table, only in first sampler partition */
	u_char ddpart[AKAI_DDPART_NUM+1][2]; /* DD partition table, only in first sampler partition */
	u_char dummy1[0x0100-2-2*(AKAI_PART_NUM+1)-2*(AKAI_DDPART_NUM+1)]; /* XXX */
}; /* Note: should be 0x0202 Bytes */



/* S1000/S3000 harddisk sampler partition header */
struct akai_parthead_s{
	u_char size[2]; /* partition size in harddisk blocks */
#define AKAI_PARTHEAD_MAGICNUM	98 /* number of fields */
#define AKAI_PARTHEAD_MAGICVAL	3333 /* magic base value */
	u_char magic[AKAI_PARTHEAD_MAGICNUM][2]; /* magic fields, indicate harddisk */
	u_char chksum[4]; /* checksum of all above */
#define AKAI_ROOTDIR_ENTRIES	100 /* number of root directory entries for volumes */
	struct akai_rootdir_entry_s vol[AKAI_ROOTDIR_ENTRIES]; /* root directory for volumes */
#define AKAI_FAT_ENTRIES	AKAI_PART_MAXSIZE /* max. number of FAT entries */
	u_char fatblk[AKAI_FAT_ENTRIES][2]; /* FAT entries for harddisk partition blocks: next block or special code */
	u_char dummy1[0x00f6]; /* XXX */
	struct akai_parttab_s parttab; /* partition table */
#define AKAI_PARTHEAD_TAGSMAGIC	"TAGS" /* magic for tags */
	u_char tagsmagic[4]; /* magic for tags */
#define AKAI_PARTHEAD_TAGNUM	26 /* number of tags */
	u_char tag[AKAI_PARTHEAD_TAGNUM][AKAI_NAME_LEN]; /* tag names */
	u_char dummy2[0x1a00-4-AKAI_PARTHEAD_TAGNUM*AKAI_NAME_LEN]; /* XXX */
}; /* Note: should be 3 harddisk blocks */
#define AKAI_PARTHEAD_BLKS	3



/* S1100/S3000 DD take header (directory entry in DD partition header) */
/* XXX should be defined in akaiutil_take.h, but must be in akaiutil.h due to dependencies */
struct akai_ddtake_s{
	u_char name[AKAI_NAME_LEN]; /* name */
	u_char cstarts[2];  /* start cluster for sample */
	u_char cstarte[2];  /* start cluster for envelope */
	u_char wstart[4];   /* sample start word (16bit) */
	u_char wend[4];     /* sample end word (16bit) */
#define AKAI_DDTAKESTAT_FREE		0x00 /* free directory entry */
#define AKAI_DDTAKESTAT_USED		0x01 /* used directory entry */
	u_char stat;        /* directory entry status */
#define AKAI_DDTAKESTYPE_MONO		0x00 /* sample type mono */
#define AKAI_DDTAKESTYPE_STEREO		0x01 /* sample type stereo */
	u_char stype;       /* sample type */
	u_char srate[2];    /* sample rate in Hz */
	u_char vspeed[2];   /* varispeed (signed) */
	u_char finerate[2]; /* fine rate (signed) */
	u_char wstartm[4];  /* edit marker sample start word (16bit) */
	u_char wendm[4];    /* edit marker sample end word (16bit) */
	u_char fadein[2];   /* fade in time in ms */
	u_char fadeout[2];  /* fade out time in ms */
	u_char stlvl;       /* stereo output level */
	u_char pan;         /* panning (signed) */
	u_char stmix;       /* stereo mix (ON=0x01, OFF=0x00) */
	u_char midich1;     /* MIDI channel-1 */
	u_char midinote;    /* MIDI note */
	u_char startm;      /* start mode */
	u_char deemph;      /* de-emphasis (ON=0x01, OFF=0x00) */
	u_char dummy1[5];   /* XXX */
	u_char predel[2];   /* pre-delay time in ms */
	u_char outlvl;      /* output level */
	u_char outch;       /* output channel pair */
	u_char fxbus;       /* FX bus */
	u_char sendlvl;     /* send level */
	u_char dummy2[2];   /* XXX */
}; /* Note: should be 0x0040 Bytes */



/* S1100/S3000 harddisk DD partition header */
struct akai_ddparthead_s{
#define AKAI_DDFAT_ENTRIES	0x07ff /* max. number of DD FAT entries (AKAI_HD_MAXSIZE/AKAI_DDPART_CBLKS) */
	u_char fatcl[AKAI_DDFAT_ENTRIES][2]; /* FAT entries for harddisk DD partition clusters: next cluster or special code */
	u_char dummy1[0x2000-AKAI_DDFAT_ENTRIES*2]; /* XXX */
#define AKAI_DDTAKE_MAXNUM	256 /* max. number of DD takes */
	struct akai_ddtake_s take[AKAI_DDTAKE_MAXNUM]; /* directory of DD takes */
}; /* Note: should be 3 harddisk blocks (rest of cluster is junk) */
#define AKAI_DDPARTHEAD_BLKS	3

#define AKAI_DDTAKE_ENVBLKSIZW	128 /* envelope block size in words */
#define AKAI_DDTAKE_ENVMAXVAL	0x20 /* max. envelope value */



/* generic header */
union akai_head_u{
	struct akai_fllhead_s fll;   /* low-density floppy header */
	struct akai_flhhead_s flh;   /* high-density floppy header */
	struct akai_hd9head_s hd9;   /* S900 harddisk header */
	struct akai_parthead_s hd;   /* S1000/S3000 harddisk sampler partition header */
	struct akai_ddparthead_s dd; /* S1000/S3000 harddisk DD partition header */
};
/* Note: see comment above: */
/*       may access files and first blocks in FAT via */
/*       high-density floppy header also for low-density floppy case */



/* CD-ROM info header in S1000/S3000 sampler partition */
struct akai_cdinfohead_s{
	u_char fnum[2]; /* number of files */
	u_char volesiz[AKAI_ROOTDIR_ENTRIES][2]; /* size of volume directory entries for files in bytes */
	u_char cdlabel[AKAI_NAME_LEN]; /* CD-ROM label */
}; /* Note: should be 0x00d6 Bytes */
/* Note: followed by voldir entries of files */
#define AKAI_CDINFO_BLK			AKAI_PARTHEAD_BLKS /* start block (following sampler partition header) */
#define AKAI_CDINFO_MINSIZB		3 /* min. size in harddisk blocks */
#define AKAI_CDINFO_DEFLABEL	"CDROM" /* XXX default label */



/* private data structures */

/* disk */
struct disk_s{
	u_int index; /* disk index */
	int fd=-1; /* disk file descriptor */
#ifdef WIN32
	int fldrn; /* floppy drive number (or -1 if none) */
#endif /* WIN32 */
	int readonly; /* read-only flag */
	OFF64_T startoff; /* start offset in bytes */
	u_int totsize; /* size in bytes */
#define DISK_TYPE_FLL	1 /* low-density floppy */
#define DISK_TYPE_FLH	2 /* high-density floppy */
#define DISK_TYPE_HD9	9 /* S900 harddisk */
#define DISK_TYPE_HD	3 /* S1000/S3000 harddisk */
	u_int type; /* disk type */
	u_int blksize; /* blocksize in bytes */
	u_int bsize; /* size in blocks */
};

/* partition */
struct part_s{
	struct disk_s *diskp; /* pointer to disk */
	int valid; /* check passed */
#define PART_TYPE_FLL	DISK_TYPE_FLL /* low-density floppy */
#define PART_TYPE_FLH	DISK_TYPE_FLH /* high-density floppy */
#define PART_TYPE_HD9	DISK_TYPE_HD9 /* S900 harddisk */
#define PART_TYPE_HD	DISK_TYPE_HD  /* S1000/S3000 harddisk sampler partion */
#define PART_TYPE_DD	4             /* S1100/S3000 harddisk DD partition (at end of harddisk) */
	u_int type; /* partition type */
	u_int index; /* index on disk */
	u_int blksize; /* blocksize in bytes */
	u_int bstart; /* start block on disk */
	u_int bsize; /* size in blocks */
	u_int csize; /* if DD partition: size in clusters */
	u_int bsyssize; /* if not DD partition: reserved blocks for system (partition header or floppy header) */
	u_int bfree; /* free blocks */
	u_int bbad; /* bad blocks */
	u_char (*fat)[2]; /* start of FAT */
	union akai_head_u head; /* whole header */
	u_int volnummax; /* if not DD partition: max. number of volumes */
	char letter; /* letter (ASCII) */
};
/* Note: fat points into original part_s head => must not use(!) a copy of a part_s !!! */

/* volume */
struct vol_s{
	struct part_s *partp; /* pointer to partition */
	u_int index; /* index in root directory */
#define AKAI_VOL_TYPE_S900		0xf9 /* fake type for S900, not valid for type in S1000/S3000 root directory */
	u_int type; /* volume type */
	u_int lnum; /* load number */
	u_int osver; /* OS version */
#define VOL_DIRBLKS	AKAI_VOLDIR3000FL_BLKS /* XXX max. number of volume blocks */
	u_int dirblk[VOL_DIRBLKS]; /* directory blocks */
	u_int fimax; /* max. file entries */
	struct akai_voldir_entry_s *file; /* pointer to start of files */
	struct akai_volparam_s *param; /* pointer to volume parameters */
	union akai_voldir_u dir; /* whole volume directory (except for S900 and S1000 floppy) */
	char name[AKAI_NAME_LEN+1]; /* name (ASCII), +1 for '\0' */
};
/* Note: file, param point into original vol_s dir => must not use(!) a copy of a vol_s !!! */
/*       => use akai_copy_vol() to obtain a self-contained copy of a vol_s */
/* Note: must be careful with partp if part_s behind it has changed/disappeared !!! */
/*       this should never be a problem since system-wide unique and static part[] */

/* sampler file */
struct file_s{
	struct vol_s *volp; /* pointer to volume */
	u_int index; /* index in volume directory */
	u_int bstart; /* start block */
	u_int size; /* size in bytes */
	u_int type; /* type */
	u_int osver; /* OS version */
	u_char tag[AKAI_FILE_TAGNUM]; /* tags */
	char name[AKAI_NAME_LEN+4+1]; /* name (ASCII), +4 for ".<type>", +1 for '\0' */
};
/* Note: must be careful with volp if vol_s behind it has changed/disappeared !!! */
/*       this could be a problem with non-unique curvolp/curvol_buf */



/* disks */
#ifndef DISK_NUM_MAX
#define DISK_NUM_MAX	64 /* XXX */
#endif
extern struct disk_s disk[DISK_NUM_MAX]; /* disks, Note: one for each disk, system-wide */
extern u_int disk_num; /* number of disks */

/* partitions */
#ifndef PART_NUM_MAX
#define PART_NUM_MAX	512 /* XXX */
#endif
extern struct part_s part[PART_NUM_MAX]; /* partitions, Note: one for each partition, system-wide */
extern u_int part_num; /* number of partitions */



/* current directory */
extern struct disk_s *curdiskp; /* current disk pointer into disk[], NULL if system-level */
								/* Note: unique, fixed mapping to each file */
extern struct part_s *curpartp; /* current partition pointer into part[], NULL if disk-level */
								/* Note: unique, fixed mapping to each partition */
extern struct vol_s *curvolp; /* current volume pointer into curvol_buf, NULL if disk- or partition-level */
							  /* Note: neither fixed nor unique mapping to each volume!!! */
extern struct vol_s curvol_buf; /* current volume buffer, Note: only one buffer, might change volume identity!!! */

/* for operations with current directory, Note: one instance only, not recursive!!! */
extern int savecurfd;
extern struct part_s *savecurpartp;
extern struct vol_s *savecurvolp;
extern struct vol_s savecurvol_buf;
extern int savecurvolmodflag; /* modifier flag */
extern char savecurvolname[AKAI_NAME_LEN+1]; /* +1 for '\0' */

#define DIRNAMEBUF_LEN	256 /* XXX */
extern char dirnamebuf[DIRNAMEBUF_LEN+1]; /* +1 for '\0' */

extern u_char curfiltertag[AKAI_FILE_TAGNUM];



/* Declarations */

#ifndef PSEUDODISK_NUM_MAX
#define PSEUDODISK_NUM_MAX		8 /* XXX max. number of pseudo-disks per file descriptor */
#endif


	

extern int open_disk(char *name,int readonly,u_int pseudodisksize);

	
extern void close_alldisks(void);

#ifndef AKAI_DISKSIZE_GRAN
#define AKAI_DISKSIZE_GRAN		2048 /* XXX disk size granularity in bytes, compatible with CD-ROM */
#endif
extern u_int akai_disksize(int fd,OFF64_T startoff,u_int disksizemax);

extern int akai_io_blks(struct part_s *pp,u_char *buf,u_int bstart,u_int bsize,int cachealloc,int mode);

extern int akai_openreadonly_extfile(char *name);

extern void akai_countfree_part(struct part_s *pp);
extern int akai_check_fatblk(u_int blk,u_int bsize,u_int bsyssize);
extern int print_fatchain(struct part_s *pp,u_int blk);
extern int akai_free_fatchain(struct part_s *pp,u_int bstart,int writeflag);
extern int akai_allocate_fatchain(struct part_s *pp,u_int bsize,u_int *bstartp,u_int bcont0,u_int endcode);

extern int akai_read_file(int outfd,u_char *outbuf,struct file_s *fp,u_int begin,u_int end);
extern int akai_write_file(int inpfd,u_char *inpbuf,struct file_s *fp,u_int begin,u_int end);

extern int print_ddfatchain(struct part_s *pp,u_int cstart);
extern u_int akai_count_ddfatchain(struct part_s *pp,u_int cstart);
extern int akai_free_ddfatchain(struct part_s *pp,u_int cstart,int writeflag);
extern int akai_allocate_ddfatchain(struct part_s *pp,u_int csize,u_int *cstartp,u_int ccont0);

extern int akai_export_ddfatchain(struct part_s *pp,u_int cstart,u_int bstart,u_int bsize,int outfd,u_char *outbuf);
extern int akai_import_ddfatchain(struct part_s *pp,u_int cstart,u_int bstart,u_int bsize,int inpfd,u_char *inpbuf);

extern char akai2ascii(u_char c);
extern char akai2ascii900(u_char c);
extern void akai2ascii_name(u_char *aname,char *name,int s900flag);
extern void akai2ascii_filename(u_char *aname,u_char ft,u_int osver,char *name,int s900flag);
extern u_char ascii2akai(char a);
extern u_char ascii2akai900(char a);
extern void ascii2akai_name(char *name,u_char *aname,int s900flag);
extern u_char ascii2akai_filename(char *name,u_char *aname,u_int *osverp,int s900flag);

extern void akai_vol_info(struct vol_s *vp,u_int ai,int verbose);
extern void akai_list_vol(struct vol_s *vp,u_char *filtertagp);
extern int akai_read_voldir(struct vol_s *vp);
extern int akai_write_voldir(struct vol_s *vp,u_int fi);
extern void akai_copy_structvol(struct vol_s *srcvp,struct vol_s *dstvp);
extern int akai_get_vol(struct part_s *pp,struct vol_s *vp,u_int vi);
extern int akai_find_vol(struct part_s *pp,struct vol_s *vp,char *name);
extern int akai_rename_vol(struct vol_s *vp,char *name,u_int lnum,u_int osver,struct akai_volparam_s *parp);
#define AKAI_CREATE_VOL_NOINDEX	((u_int)-1) /* no user-supplied index */
extern int akai_create_vol(struct part_s *pp,struct vol_s *vp,u_int type,u_int index,char *name,u_int lnum,struct akai_volparam_s *parp);
extern int akai_wipe_vol(struct vol_s *vp,int delflag);

extern void akai_list_volparam(struct akai_volparam_s *parp,int listflag);
extern void akai_init_volparam(struct akai_volparam_s *parp,int s3000flag);

extern void akai_print_lnum(u_int lnum);
extern u_int akai_get_lnum(char *name);

extern int copy_tags(struct part_s *srcpp,struct part_s *dstpp);
extern void akai_list_tags(struct part_s *pp);
extern int akai_rename_tag(struct part_s *pp,char *name,u_int ti,int wipeflag);

extern void akai_print_cdinfo(struct part_s *pp,int verbose);
extern int akai_set_cdinfo(struct part_s *pp,char *cdlabel);

extern void akai_part_info(struct part_s *pp,int verbose);
extern void akai_list_part(struct part_s *pp,int recflag,u_char *filtertagp);
extern struct part_s *akai_find_part(struct disk_s *dp,char *name);
extern void print_fat(struct part_s *pp);

extern void akai_disk_info(struct disk_s *dp,int verbose);
extern void akai_list_disk(struct disk_s *dp,int recflag,u_char *filtertagp);
extern void akai_list_alldisks(int recflag,u_char *filtertagp);

extern void akai_sort_filetags(u_char *tagp);
extern int akai_set_filetag(u_char *tagp,u_int ti);
extern int akai_clear_filetag(u_char *tagp,u_int ti);
extern int akai_match_filetags(u_char *filtertagp,u_char *testtagp);
extern int akai_get_file(struct vol_s *vp,struct file_s *fp,u_int fi);
extern int akai_find_file(struct vol_s *vp,struct file_s *fp,char *name);
extern int akai_rename_file(struct file_s *fp,char *name,struct vol_s *vp,u_int dstindex,u_char *tagp,u_int osver);
#define AKAI_CREATE_FILE_NOINDEX	((u_int)-1) /* no user-supplied index */
extern int akai_create_file(struct vol_s *vp,struct file_s *fp,u_int size,u_int index,char *name,u_int osver,u_char *tagp);
extern void akai_fvol1000_initfile(struct akai_voldir_entry_s *ep,u_int osver,u_int tag);
extern int akai_delete_file(struct file_s *fp);

extern int akai_rename_ddtake(struct part_s *pp,u_int ti,char *name);
extern int akai_delete_ddtake(struct part_s *pp,u_int ti);

extern void akai_fix_partheadmagic(struct part_s *pp);
extern int akai_check_partheadmagic(struct part_s *pp);
extern void akai_fix_parttabmagic(struct akai_parttab_s *ptp);
extern int akai_check_parttabmagic(struct akai_parttab_s *ptp);

extern int akai_scan_floppy(struct disk_s *dp);
extern int akai_scan_harddisk9(struct disk_s *dp);
extern int akai_scan_ddpart(struct disk_s *dp,struct akai_parttab_s *ptp,u_int bstart,u_int pi);
extern int akai_scan_disk(struct disk_s *dp,int floppyenable);

extern int akai_wipe_part(struct part_s *pp,int wipeflag,struct part_s *plistp,u_int pimax,int cdromflag);
extern int akai_wipe_harddisk(struct disk_s *dp,u_int bsize,u_int totb,int s3000flag,int cdromflag);
extern int akai_wipe_harddisk9(struct disk_s *dp,u_int totb);
extern int akai_wipe_floppy(struct disk_s *dp,int lodensflag,int s3000flag,int s900flag,int scanbadflag);

extern int change_curdir(char *name,u_int vi,char *lastname,int checklast);
extern int change_curdir_home(void);
extern void save_curdir(int modflag);
extern void restore_curdir(void);
extern void curdir_info(int verbose);
extern void list_curdir(int recflag);
extern void list_curfiltertags(void);

extern int copy_file(struct file_s *srcfp,struct vol_s *dstvp,struct file_s *dstfp,u_int dstindex,char *dstname,int delflag);
extern int copy_vol_allfiles(struct vol_s *srcvp,struct part_s *dstpp,char *dstname,int delflag,int verbose);
extern int copy_part_allvols(struct part_s *srcpp,struct part_s *dstpp,int delflag,int verbose);

extern int check_curnosamplervol(void);
extern int check_curnosamplerpart(void);
extern int check_curnoddpart(void);
extern int check_curnopart(void);

//extern void printsock(char *format,...);
//extern void printsock2( char* f, ... );
extern int createSocket(int port) ;

#ifdef __cplusplus
}
#endif
#endif /* !__AKAIUTIL_H */
