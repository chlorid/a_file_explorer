#ifndef __AKAIUTIL_TAKE_H
#define __AKAIUTIL_TAKE_H
/*
* Copyright (C) 2012,2018,2019 Klaus Michael Indlekofer. All rights reserved.
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



/* AKAI DD takes */

/* Note: all data types are little endian */



/* S1100/S3000 DD take header (directory entry in DD partition header) */
/* XXX should be defined in akaiutil_take.h, but must be in akaiutil.h due to dependencies */
struct akai_ddtake_s;



/* file name ending for exported DD take */
#define AKAI_DDTAKE_FNAMEEND	".TK"



/* Declarations */

extern int akai_ddtake_info(struct part_s *pp,u_int ti,int verbose);

extern int akai_import_take(int inpfd,struct part_s *pp,struct akai_ddtake_s *tp,u_int ti,u_int csizes,u_int csizee);

extern int akai_export_take(int outfd,struct part_s *pp,struct akai_ddtake_s *tp,u_int csizes,u_int csizee,u_int cstarts,u_int cstarte);

#define TAKE2WAV_CHECK		1
#define TAKE2WAV_EXPORT		2
#define TAKE2WAV_CREATE		4
#define TAKE2WAV_ALL		0xff
extern int akai_take2wav(struct part_s *pp,u_int ti,int wavfd,u_int *sizep,char **wavnamep,int what);

#define WAV2TAKE_OPEN		1
extern int akai_wav2take(int wavfd,char *wavname,struct part_s *pp,u_int ti,u_int *bcountp,int what);

extern int akai_take_setenv(struct part_s *pp,u_int cstarts,u_int samplesize,u_char *envbuf,u_int envsiz);



#endif /* !__AKAIUTIL_TAKE_H */
