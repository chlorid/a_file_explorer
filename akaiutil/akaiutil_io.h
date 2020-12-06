#ifndef __AKAIUTIL_IO_H
#define __AKAIUTIL_IO_H
/*
* Copyright (C) 2008,2010,2012,2019,2020 Klaus Michael Indlekofer. All rights reserved.
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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
/* for low-level floppy I/O, using fdrawcmd.sys */
#include "fdrawcmd.h" /* from http://simonowen.com/fdrawcmd/fdrawcmd.h */

/* definitions for low-level format of floppy disks for AKAI samplers */
#define AKAI_FL_DISK_SIDES			2				/* sides per disk */
#define AKAI_FL_DISK_TRACKS			80				/* tracks per side */
#define AKAI_FLL_DISK_SECTORS		5				/* sectors per track for low-density floppy */
#define AKAI_FLH_DISK_SECTORS		10				/* sectors per track for high-density floppy */
#define AKAI_FLL_DISK_DATARATE		FD_RATE_250K	/* data rate for low-density floppy */
#define AKAI_FLH_DISK_DATARATE		FD_RATE_500K	/* data rate for high-density floppy */
#define AKAI_FL_SECTOR_SIZE_CODE	3				/* sector size code: 3 -> (128<<3)==1024 */
#define AKAI_FLL_SECTOR_GAP3		0x74			/* gap3 size between sectors for low-density floppy */
#define AKAI_FLH_SECTOR_GAP3		0x74			/* gap3 size between sectors for high-density floppy */
#define AKAI_FL_SECTOR_FILL			0x00			/* fill byte for formatted sectors */
#define AKAI_FL_SECTOR_BASE			1				/* first sector number on track */
#define AKAI_FLL_SECTOR_OFFSET		3				/* sector number offset for low-density floppy */
#define AKAI_FLH_SECTOR_OFFSET		6				/* sector number offset for high-density floppy */
#define AKAI_FLL_TRACK_SKEW			2				/* skew to the same sector on the next track for low-density floppy */
#define AKAI_FLH_TRACK_SKEW			4				/* skew to the same sector on the next track for high-density floppy */

/* low-level floppy sector size */
#define AKAI_FL_SECSIZE		0x0400 /* 1KB */
/* Note: must be == AKAI_FL_BLOCKSIZE (floppy filesystem blocksize, defined in akaiutil.h) */
#if defined(AKAI_FL_BLOCKSIZE)&&(AKAI_FL_SECSIZE!=AKAI_FL_BLOCKSIZE)
#error "AKAI_FL_SECSIZE!=AKAI_FL_BLOCKSIZE"
#endif

#ifndef FLDRNUM
#define FLDRNUM		2 /* XXX max. number of floppy drives */
#endif
extern HANDLE fldr_h[FLDRNUM]; /* handle for floppy drive */
#define FLDR_TYPE_FLL		0 /* low-density floppy */
#define FLDR_TYPE_FLH		1 /* high-density floppy */
extern u_int fldr_type[FLDRNUM]; /* floppy drive type */
extern PBYTE fldr_secbuf; /* DMA buffer for sector */

#ifndef FLDR_RETRYMAX
#define FLDR_RETRYMAX	5 /* XXX max. number of retries for floppy I/O */
#endif
#endif /* WIN32 */



/* cache */

struct blk_cache_s{
	int valid;
	int modified;
	int fd;
#ifdef WIN32
	int fldrn;
#endif /* WIN32 */
	OFF64_T startoff;
	u_int blk;
	u_int blksize;
#define BLK_CACHE_AGE_MAX		0xffffffff /* XXX max. age */
	u_int age; /* age */
	u_char *buf;
};

#ifndef BLK_CACHE_NUM
#define BLK_CACHE_NUM	512 /* XXX */
#endif
extern struct blk_cache_s blk_cache[BLK_CACHE_NUM];



#define IO_BLKS_READ	0
#define IO_BLKS_WRITE	1



/* Declarations */

#ifdef WIN32
extern int fldr_init(void);
extern void fldr_end(void);
extern int fldr_open(int fldrn,u_int fldrtype);
extern int fldr_checkfloppyinserted(int fldrn);
extern int fldr_io_direct(int fldrn,u_int blk,u_char *buf,int mode);
extern int fldr_format(int fldrn);
#endif /* WIN32 */
extern void init_blk_cache(void);
extern void free_blk_cache(void);
extern void print_blk_cache(void);
extern int find_blk_cache(int fd,
#ifdef WIN32
						  int fldrnr,
#endif
						  OFF64_T startoff,u_int blk,u_int blksize);
extern void blk_cache_aging(int i);
extern int io_blks_direct(int fd,
#ifdef WIN32
						  int fldrn,
#endif
						  OFF64_T startoff,u_char *buf,u_int bstart,u_int bsize,u_int blksize,int cachealloc,int mode);
extern int flush_blk_cache(void);
extern int io_blks(int fd,
#ifdef WIN32
				   int fldrn,
#endif
				   OFF64_T startoff,u_char *buf,u_int bstart,u_int bsize,u_int blksize,int cachealloc,int mode);


}
#endif /* !__AKAIUTIL_IO_H */
