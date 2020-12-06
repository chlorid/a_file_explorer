#ifndef __AKAIUTIL_TAR_H
#define __AKAIUTIL_TAR_H
/*
* Copyright (C) 2008,2010,2012,2019 Klaus Michael Indlekofer. All rights reserved.
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



/* tar header */

/* Note: numbers in octal, trailing '\0' */
struct tar_head_s{
#define TAR_NAMELEN 100 /* incl. '\0' */
	char name[TAR_NAMELEN];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[7];
	char space;
	char type;
	char linkname[TAR_NAMELEN];
#define TAR_USTAR "ustar\00000"
	char ustar[8];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
#define TAR_PREFIXLEN 155 /* incl. '\0' */
	char prefix[TAR_PREFIXLEN];
	char zero[12];
};
/* should be 512 bytes */
#define TAR_BLOCKSIZE 512 /* in bytes */

#define TAR_TYPE_REG  '0'  /* regular file */
#define TAR_TYPE_REG0 '\0' /* regular file */
#define TAR_TYPE_DIR  '5'  /* directory */

#define TAR_TAILZERO_BLOCKS	6 /* number or zero blocks at file end */



/* Declarations */

extern u_int tar_checksum(u_char *hp);

#define TAR_EXPORT_PART				0x0001
#define TAR_EXPORT_VOL				0x0002
#define TAR_EXPORT_FILE				0x0004
#define TAR_EXPORT_DDFILE			0x0008
#define TAR_EXPORT_TAGSFILE			0x0010
#define TAR_EXPORT_VOLPARAMFILE		0x0020
#define TAR_EXPORT_ANYFILE			(TAR_EXPORT_FILE|TAR_EXPORT_DDFILE|TAR_EXPORT_TAGSFILE|TAR_EXPORT_VOLPARAMFILE)
#define TAR_EXPORT_WAV				0x0100
extern int tar_export(int fd,struct part_s *pp,struct vol_s *vp,struct file_s *fp,u_int ti,u_int flags,int verbose,u_char *filtertagp);
extern int tar_export_vol(int fd,struct vol_s *vp,u_int flags,int verbose,u_char *filtertagp);
extern int tar_export_part(int fd,struct part_s *pp,u_int flags,int verbose,u_char *filtertagp);
extern int tar_export_disk(int fd,struct disk_s *dp,u_int flags,int verbose,u_char *filtertagp);
extern int tar_export_curdir(int fd,int verbose,u_int flags);
extern int tar_export_tailzero(int fd);

#define TAR_IMPORT_WAV				0x0100
#define TAR_IMPORT_WAVS9			0x1000
#define TAR_IMPORT_WAVS9C			0x2000
#define TAR_IMPORT_WAVS1			0x4000
#define TAR_IMPORT_WAVS3			0x8000
extern int tar_import_curdir(int fd,u_int vtype0,int verbose,u_int flags);



#endif /* !__AKAIUTIL_TAR_H */
