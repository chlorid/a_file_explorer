/*
* Copyright (C) 2008,2019,2020 Klaus Michael Indlekofer. All rights reserved.
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



#ifdef WIN32
/* for low-level floppy I/O, using fdrawcmd.sys */
#include "fdrawcmd.h" /* from http://simonowen.com/fdrawcmd/fdrawcmd.h */

HANDLE fldr_h[FLDRNUM]; /* handle for floppy drive */
u_int fldr_type[FLDRNUM]; /* floppy drive type */
PBYTE fldr_secbuf; /* DMA buffer for sector */

int
fldr_init(void)
{
	u_int i;

	for (i=0;i<FLDRNUM;i++){
		fldr_h[i]=INVALID_HANDLE_VALUE;
		fldr_type[i]=FLDR_TYPE_FLL; /* XXX */
	}

	/* allocate DMA buffer for sector */
	fldr_secbuf=(PBYTE)VirtualAlloc(NULL,AKAI_FL_SECSIZE,MEM_COMMIT,PAGE_READWRITE);
	if (fldr_secbuf==NULL){
		PRINTF_ERR("fldr_init: cannot allocate DMA buffer\n");
		return -1;
	}

	return 0;
}

void
fldr_end(void)
{
	u_int i;

	for (i=0;i<FLDRNUM;i++){
		if (fldr_h[i]!=INVALID_HANDLE_VALUE){
			CloseHandle(fldr_h[i]);
			fldr_h[i]=INVALID_HANDLE_VALUE;
		}
	}
	if (fldr_secbuf!=NULL){
		VirtualFree(fldr_secbuf,0,MEM_RELEASE);
		fldr_secbuf=NULL;
	}
}

int
fldr_open(int fldrn,u_int fldrtype)
{
	static char devname[32]; /* XXX */
	HANDLE h;
	DWORD version;
	BYTE datarate;
	DWORD ret;

	if ((fldrn<0)||(fldrn>=FLDRNUM)){
		PRINTF_ERR("fldr_open: invalid floppy drive number\n");
		return -1;
	}
	if (fldr_h[fldrn]!=INVALID_HANDLE_VALUE){
		PRINTF_ERR("fldr_open: floppy drive already opened\n");
		return -1;
	}

	/* check driver */
	version=0; /* invalid */
	/* Note: must use GENERIC_READ|GENERIC_WRITE for DeviceIoControl() */
	h=CreateFileA("\\\\.\\fdrawcmd",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if (h!=INVALID_HANDLE_VALUE){
		if (!DeviceIoControl(h,IOCTL_FDRAWCMD_GET_VERSION,NULL,0,&version,sizeof(version),&ret,NULL)){
			version=0; /* invalid */
		}
		CloseHandle(h);
	}
	if (version==0){
		PRINTF_ERR("fldr_open: fdrawcmd.sys is not installed, see: http://simonowen.com/fdrawcmd/\n");
		return -1;
	}
	if (HIWORD(version)!=HIWORD(FDRAWCMD_VERSION)){
		PRINTF_ERR("fldr_open: the installed fdrawcmd.sys is not compatible with this program\n");
		return -1;
	}

	/* open and init device */
    sprintf(devname,"\\\\.\\fdraw%u",(u_int)fldrn);
	/* Note: must use GENERIC_READ|GENERIC_WRITE for DeviceIoControl() */
    h=CreateFileA(devname,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if (h==INVALID_HANDLE_VALUE){
		PRINTF_ERR("fldr_open: cannot open floppy drive\n");
		return -1;
	}
	if (fldrtype==FLDR_TYPE_FLH){
		datarate=AKAI_FLH_DISK_DATARATE;
	}else{
		datarate=AKAI_FLL_DISK_DATARATE;
	}
	if (!DeviceIoControl(h,IOCTL_FD_SET_DATA_RATE,&datarate,sizeof(datarate),NULL,0,&ret,NULL)){
		PRINTF_ERR("fldr_open: cannot set floppy data rate\n");
		CloseHandle(h);
		return -1;
	}
	if (!DeviceIoControl(h,IOCTL_FD_RESET,NULL,0,NULL,0,&ret,NULL)){
		PRINTF_ERR("fldr_open: cannot reset floppy controller\n");
		CloseHandle(h);
		return -1;
	}

	/* save handle and type */
	fldr_h[fldrn]=h;
	fldr_type[fldrn]=fldrtype;

	return 0;
}

int
fldr_checkfloppyinserted(int fldrn)
{
	DWORD ret;

	if ((fldrn<0)||(fldrn>=FLDRNUM)||(fldr_h[fldrn]==INVALID_HANDLE_VALUE)){
		return -1;
	}

	/* check if no floppy inserted */
	if (!DeviceIoControl(fldr_h[fldrn],IOCTL_FD_CHECK_DISK,NULL,0,NULL,0,&ret,NULL)){
		return -1;
	}

	return 0; /* floppy is inserted */
}

int
fldr_io_direct(int fldrn,u_int blk,u_char *buf,int mode)
{
	BYTE cyl;
	BYTE head;
	BYTE sec;
	DWORD ret;
    FD_READ_WRITE_PARAMS rwp;
	u_int retry;
	int retval;

	if ((fldrn<0)||(fldrn>=FLDRNUM)||(fldr_h[fldrn]==INVALID_HANDLE_VALUE)||(buf==NULL)){
		return -1;
	}
	if ((mode!=IO_BLKS_READ)&&(mode!=IO_BLKS_WRITE)){
		return -1;
	}

	if (fldr_type[fldrn]==FLDR_TYPE_FLH){
		cyl=blk/(AKAI_FL_DISK_SIDES*AKAI_FLH_DISK_SECTORS);
		head=1&(blk/AKAI_FLH_DISK_SECTORS);
		sec=AKAI_FL_SECTOR_BASE+(blk%AKAI_FLH_DISK_SECTORS);
	}else{
		cyl=blk/(AKAI_FL_DISK_SIDES*AKAI_FLL_DISK_SECTORS);
		head=1&(blk/AKAI_FLL_DISK_SECTORS);
		sec=AKAI_FL_SECTOR_BASE+(blk%AKAI_FLL_DISK_SECTORS);
	}
#ifdef DEBUG
	printf("fldr_io_direct: cyl=%u head=%u sec=%u\n",(u_int)cyl,(u_int)head,(u_int)sec);
#endif

	/* check if no floppy inserted */
	if (!DeviceIoControl(fldr_h[fldrn],IOCTL_FD_CHECK_DISK,NULL,0,NULL,0,&ret,NULL)){
		return -1;
	}

	/* goto track */
	if (!DeviceIoControl(fldr_h[fldrn],IOCTL_FDCMD_SEEK,&cyl,sizeof(cyl),NULL,0,&ret,NULL)){
		return -1;
	}

	/* set parameters */
	rwp.flags=FD_OPTION_MFM;
	rwp.phead=head;
	rwp.cyl=cyl;
	rwp.head=head;
	rwp.sector=sec;
	rwp.size=AKAI_FL_SECTOR_SIZE_CODE;
	rwp.eot=sec+1; /* for IOCTL_FDCMD_READ_DATA,IOCTL_FDCMD_WRITE_DATA: last sector+1 !!! */
	rwp.gap=0x0a;
	rwp.datalen=0xff;

	retval=0; /* no error so far */
	if (mode==IO_BLKS_WRITE){
		/* copy sector from buf to DMA buffer */
		bcopy(buf,(u_char *)fldr_secbuf,AKAI_FL_SECSIZE);

		for (retry=0;retry<FLDR_RETRYMAX;retry++){
			/* write sector */
			if (DeviceIoControl(fldr_h[fldrn],IOCTL_FDCMD_WRITE_DATA,&rwp,sizeof(rwp),(PVOID)fldr_secbuf,AKAI_FL_SECSIZE,&ret,NULL)){
				break;
			}
		}
		if (retry>=FLDR_RETRYMAX){
			retval=-1; /* error */
		}
	}else{
		for (retry=0;retry<FLDR_RETRYMAX;retry++){
			/* read sector */
			if (DeviceIoControl(fldr_h[fldrn],IOCTL_FDCMD_READ_DATA,&rwp,sizeof(rwp),(PVOID)fldr_secbuf,AKAI_FL_SECSIZE,&ret,NULL)){
				break;
			}
		}
		if (retry>=FLDR_RETRYMAX){
			retval=-1; /* error */
		}

		/* copy sector from DMA buffer to buf */
		/* Note: even DeviceIoControl() above returned an error */
		/*       in this case, DMA buffer could contain leftover from last call */
		/*       or could contain correct complete or partial sector */
		/*       -> let caller decide what to do with buf in case of an error */
		bcopy((u_char *)fldr_secbuf,buf,AKAI_FL_SECSIZE);
	}

	return retval;
}

int
fldr_format(int fldrn)
{
	BYTE cyl;
	BYTE head;
	BYTE sec;
	BYTE secnum;
	BYTE secgap3;
	BYTE secoffset;
	BYTE trackskew;
	DWORD ret;
    static BYTE fpbuf[sizeof(FD_FORMAT_PARAMS)+sizeof(FD_ID_HEADER)*AKAI_FLH_DISK_SECTORS]; /* space for max. track size */
    PFD_FORMAT_PARAMS fpp;
	DWORD fpsize;
#ifndef FLDR_FORMAT_NOVERIFY
    FD_READ_WRITE_PARAMS rwp;
#endif
	u_int retry;
	int retval;

	if ((fldrn<0)||(fldrn>=FLDRNUM)||(fldr_h[fldrn]==INVALID_HANDLE_VALUE)){
		return -1;
	}

	if (fldr_type[fldrn]==FLDR_TYPE_FLH){
		secnum=AKAI_FLH_DISK_SECTORS;
		secgap3=AKAI_FLH_SECTOR_GAP3;
		secoffset=AKAI_FLH_SECTOR_OFFSET;
		trackskew=AKAI_FLH_TRACK_SKEW;
	}else{
		secnum=AKAI_FLL_DISK_SECTORS;
		secgap3=AKAI_FLL_SECTOR_GAP3;
		secoffset=AKAI_FLL_SECTOR_OFFSET;
		trackskew=AKAI_FLL_TRACK_SKEW;
	}

	retval=0; /* no error so far */
	fpp=(PFD_FORMAT_PARAMS)fpbuf;
	fpsize=sizeof(FD_FORMAT_PARAMS)+secnum*sizeof(FD_ID_HEADER);
	for (cyl=0;cyl<AKAI_FL_DISK_TRACKS;cyl++){
		/* check if no floppy inserted */
		if (!DeviceIoControl(fldr_h[fldrn],IOCTL_FD_CHECK_DISK,NULL,0,NULL,0,&ret,NULL)){
			PRINTF_OUT("\rno floppy inserted          \n",((u_int)cyl)+1);
			FLUSH_ALL;
			return -1;
		}

		/* goto track */
		if (!DeviceIoControl(fldr_h[fldrn],IOCTL_FDCMD_SEEK,&cyl,sizeof(cyl),NULL,0,&ret,NULL)){
			PRINTF_OUT("\rtrack %u - cannot seek track\n",((u_int)cyl)+1);
			FLUSH_ALL;
			return -1;
		}

		for (head=0;head<AKAI_FL_DISK_SIDES;head++){
			PRINTF_OUT("\rtrack %u side %u",((u_int)cyl)+1,((u_int)head)+1);
			FLUSH_ALL;

			/* set parameters */
			fpp->flags=FD_OPTION_MFM;
			fpp->phead=head;
			fpp->size=AKAI_FL_SECTOR_SIZE_CODE;
			fpp->sectors=secnum;
			fpp->gap=secgap3;
			fpp->fill=AKAI_FL_SECTOR_FILL;
			for (sec=0;sec<secnum;sec++){
				fpp->Header[sec].cyl=cyl;
				fpp->Header[sec].head=head;
				fpp->Header[sec].sector=AKAI_FL_SECTOR_BASE+((secoffset+sec+cyl*(secnum-trackskew))%secnum);
				fpp->Header[sec].size=AKAI_FL_SECTOR_SIZE_CODE;
			}
#ifndef FLDR_FORMAT_NOVERIFY
			rwp.flags=FD_OPTION_MFM;
			rwp.phead=head;
			rwp.cyl=cyl;
			rwp.head=head;
			rwp.sector=AKAI_FL_SECTOR_BASE;
			rwp.size=AKAI_FL_SECTOR_SIZE_CODE;
			rwp.eot=secnum; /* for IOCTL_FDCMD_VERIFY: number of sectors !!! */
			rwp.gap=0x0a;
			rwp.datalen=0xff;
#endif

			for (retry=0;retry<FLDR_RETRYMAX;retry++){
				/* format track */
				if (DeviceIoControl(fldr_h[fldrn],IOCTL_FDCMD_FORMAT_TRACK,fpp,fpsize,NULL,0,&ret,NULL)){
#ifndef FLDR_FORMAT_NOVERIFY
					/* verify track */
					if (DeviceIoControl(fldr_h[fldrn],IOCTL_FDCMD_VERIFY,&rwp,sizeof(rwp),NULL,0,&ret,NULL))
#endif
					{
						break;
					}
				}
			}
			if (retry>=FLDR_RETRYMAX){
				PRINTF_OUT(" - cannot format track\n");
				FLUSH_ALL;
#if 1
				/* XXX ignore error, continue */
				retval=-1;
#else
				return -1;
#endif
			}
		}
	}
	PRINTF_OUT("\rdone                           \n");
	FLUSH_ALL;

	return retval;
}
#endif /* WIN32 */



/* cache */

struct blk_cache_s blk_cache[BLK_CACHE_NUM];



void
init_blk_cache(void)
{
	int i;

	for (i=0;i<BLK_CACHE_NUM;i++){
		blk_cache[i].valid=0; /* free entry */
		blk_cache[i].buf=NULL; /* not allocated yet */
	}
}



void
free_blk_cache(void)
{
	int i;

	for (i=0;i<BLK_CACHE_NUM;i++){
		if (blk_cache[i].buf!=NULL){ /* buffer allocated? */
			/* free buffer */
			free(blk_cache[i].buf);
			blk_cache[i].valid=0; /* free entry */
			blk_cache[i].buf=NULL; /* not allocated yet */
		}
	}
}



void
print_blk_cache(void)
{
	int i;

#ifdef WIN32
	PRINTF_OUT("nr    fd   fldrn start/B       blksize blk     age         mod\n");
	PRINTF_OUT("--------------------------------------------------------------\n");
#else /* !WIN32 */
	PRINTF_OUT("nr    fd   start/B       blksize blk     age         mod\n");
	PRINTF_OUT("--------------------------------------------------------\n");
#endif
	for (i=0;i<BLK_CACHE_NUM;i++){
		if (!blk_cache[i].valid){ /* free entry? */
			continue; /* next */
		}
#ifdef WIN32
		PRINTF_OUT("%4i  %3i  %3i   0x%02x%08x  0x%04x  0x%04x  0x%08x  %i\n",
			i,
			blk_cache[i].fd,
			blk_cache[i].fldrn,
			(u_int)(blk_cache[i].startoff>>32),
			(u_int)(0xffffffff&blk_cache[i].startoff),
			blk_cache[i].blksize,
			blk_cache[i].blk,
			blk_cache[i].age,
			blk_cache[i].modified);
#else /* !WIN32 */
		PRINTF_OUT("%4i  %3i  0x%02x%08x  0x%04x  0x%04x  0x%08x  %i\n",
			i,
			blk_cache[i].fd,
			(u_int)(blk_cache[i].startoff>>32),
			(u_int)(0xffffffff&blk_cache[i].startoff),
			blk_cache[i].blksize,
			blk_cache[i].blk,
			blk_cache[i].age,
			blk_cache[i].modified);
#endif /* !WIN32 */
	}
#ifdef WIN32
	PRINTF_OUT("--------------------------------------------------------------\n");
#else /* !WIN32 */
	PRINTF_OUT("--------------------------------------------------------\n");
#endif
}



int
find_blk_cache(int fd,
#ifdef WIN32
			   int fldrn,
#endif
			   OFF64_T startoff,u_int blk,u_int blksize)
{
	int i;

	/* scan cache */
	for (i=0;i<BLK_CACHE_NUM;i++){
		if (blk_cache[i].buf==NULL){
			continue; /* next */
		}
		if (!blk_cache[i].valid){ /* free? */
			continue; /* next */
		}
		/* check if contained within cache */
		/* Note: must check for blksize as well!!! */
		if ((fd==blk_cache[i].fd)
#ifdef WIN32
			&&(fldrn==blk_cache[i].fldrn)
#endif
			&&(startoff==blk_cache[i].startoff)
			&&(blk==blk_cache[i].blk)
			&&(blksize==blk_cache[i].blksize)){
			break; /* found it */
		}
	}

	if (i==BLK_CACHE_NUM){ /* none found? */
		return -1;
	}

	return i;
}



void
blk_cache_aging(int i)
{
	int j;
	u_int agemin;

	if ((i<0)||(i>=BLK_CACHE_NUM)||(!blk_cache[i].valid)){
		return;
	}

	blk_cache[i].age=0; /* reset age */

	/* find the youngest of the others */
	agemin=BLK_CACHE_AGE_MAX;
	for (j=0;j<BLK_CACHE_NUM;j++){
		if (!blk_cache[j].valid){ /* free? */
			continue; /* next */
		}
		if (j==i){
			continue; /* next */
		}
		if (blk_cache[j].age<agemin){
			agemin=blk_cache[j].age;
		}
	}
	if (agemin==0){ /* somebody else too young? */
		/* increment age of the others */
		for (j=0;j<BLK_CACHE_NUM;j++){
			if (j==i){
				continue; /* next */
			}
			if (blk_cache[j].age<BLK_CACHE_AGE_MAX){
				blk_cache[j].age++; /* increase age */
			}
		}
	}
}



int
io_blks_direct(int fd,
#ifdef WIN32
			   int fldrn,
#endif
			   OFF64_T startoff,u_char *buf,u_int bstart,u_int bsize,u_int blksize,int cachealloc,int mode)
{
	int err;
	int j,jmax;
	u_int agemax;
	u_int blk,blkmin,blkmax,blkchunk;

	if (buf==NULL){
		return -1;
	}
#ifdef WIN32
	if ((fd<0)&&(fldrn<0)){
		return -1;
	}
#else /* !WIN32 */
	if (fd<0){
		return -1;
	}
#endif

	if ((mode!=IO_BLKS_READ)&&(mode!=IO_BLKS_WRITE)){
		return -1;
	}

	/* Note: OFF64_T against overflow */
	if ((((OFF64_T)bstart)+((OFF64_T)bsize))>(OFF64_T)0xffffffff){ /* XXX */
		return -1;
	}

	if (bsize==0){
		return 0; /* done */
	}

	if (cachealloc /* must allocate cache? */
#ifdef WIN32
		||(fldrn>=0) /* or is floppy drive? */
#endif
		){
		/* single blocks */
		blkmin=bstart;
		blkmax=bstart+bsize;
		blkchunk=1;
	}else{
		/* all blocks at once */
		blkmin=bstart;
		blkmax=bstart+1; /* fake value: only once */
		blkchunk=bsize;
	}

	for (blk=blkmin;blk<blkmax;blk++){
#ifdef WIN32
		if (fldrn>=0){ /* is floppy drive? */
			if (fldr_io_direct(fldrn,blk,buf+(blk-blkmin)*blksize,mode)<0){
				return -1;
			}
		}else /* no floppy drive */
#endif
		{
			/* goto blk */
			if (LSEEK64(fd,startoff+(OFF64_T)(blk*blksize),SEEK_SET)<0){
				PERROR("lseek");
				return -1;
			}
			if (mode==IO_BLKS_WRITE){
				/* write block(s) */
				err=WRITE(fd,(void *)(buf+(blk-blkmin)*blksize),blkchunk*blksize);
				if (err<0){
					PERROR("write");
					return -1;
				}
				if (err!=(int)(blkchunk*blksize)){
					PRINTF_ERR("write: incomplete\n");
					return -1;
				}
			}else{
				/* read block(s) */
				err=READ(fd,(void *)(buf+(blk-blkmin)*blksize),blkchunk*blksize);
				if (err<0){
					PERROR("read");
					return -1;
				}
				if (err!=(int)(blkchunk*blksize)){
					PRINTF_ERR("read: incomplete\n");
					return -1;
				}
			}
		}
		if (cachealloc){ /* must allocate cache? */
			/* find free entry or the oldest one */
			agemax=0;
			jmax=0; /* first guess */
			for (j=0;j<BLK_CACHE_NUM;j++){
				if (!blk_cache[j].valid){ /* free? */
					jmax=j;
					break; /* found one */
				}
				if (blk_cache[j].age>=agemax){ /* older or equal? */
					agemax=blk_cache[j].age;
					jmax=j;
				}
			}
			/* old cache entry not free and modified? */
			err=0;
			if ((blk_cache[jmax].valid)&&(blk_cache[jmax].modified)&&(blk_cache[jmax].buf!=NULL)){
				/* must flush this block */
				if (io_blks_direct(blk_cache[jmax].fd,
#ifdef WIN32
									blk_cache[jmax].fldrn,
#endif
									blk_cache[jmax].startoff,
									(u_char *)blk_cache[jmax].buf,
									blk_cache[jmax].blk,
									1,
									blk_cache[jmax].blksize,
									0, /* don't alloc cache */
									IO_BLKS_WRITE)<0){
					PRINTF_ERR("cannot flush cache block 0x%08x of fd %i\n",blk_cache[jmax].blk,blk_cache[jmax].fd);
					err=1; /* cannot allocate below */
				}
			}
			if (!err){
				if ((blk_cache[jmax].buf!=NULL)&&(blk_cache[jmax].blksize!=blksize)){ /* not compatible? */
					/* free buffer */
					free(blk_cache[jmax].buf);
					blk_cache[jmax].buf=NULL;
				}
				if (blk_cache[jmax].buf==NULL){ /* buffer not allocated yet/anymore? */
					/* need to allocate buffer */
					if ((blk_cache[jmax].buf=(u_char *)malloc(blksize))==NULL){
						PERROR("malloc");
						err=1; /* cannot allocate below */
					}
					blk_cache[jmax].valid=0; /* free */
					blk_cache[jmax].blksize=blksize;
				}
			}
			if (!err){
				/* allocate cache entry */
				blk_cache[jmax].valid=1;
				blk_cache[jmax].modified=0;
				blk_cache[jmax].fd=fd;
#ifdef WIN32
				blk_cache[jmax].fldrn=fldrn;
#endif
				blk_cache[jmax].startoff=startoff;
				blk_cache[jmax].blk=blk;
				blk_cache_aging(jmax); /* adjust ages */
				/* copy data */
				bcopy(buf+(blk-blkmin)*blksize,blk_cache[jmax].buf,blksize);
			}
		}
	}

	return 0;
}



/* Note: prior to changing blksize of any device/file, must flush cache first !!! */
int
flush_blk_cache(void)
{
	int ret;
	int i;

	/* scan cache */
	ret=0; /* no error so far */
	for (i=0;i<BLK_CACHE_NUM;i++){
		if (blk_cache[i].buf==NULL){
			continue; /* next */
		}
		if (!blk_cache[i].valid){ /* free? */
			continue; /* next */
		}
		if (!blk_cache[i].modified){ /* not modified? */
			continue; /* next */
		}
		/* must write */
		if (io_blks_direct(blk_cache[i].fd,
#ifdef WIN32
							blk_cache[i].fldrn,
#endif
							blk_cache[i].startoff,
							(u_char *)blk_cache[i].buf,
							blk_cache[i].blk,
							1,
							blk_cache[i].blksize,
							0, /* don't alloc cache */
							IO_BLKS_WRITE)<0){
			PRINTF_ERR("cannot flush cache block 0x%08x of fd %i\n",blk_cache[i].blk,blk_cache[i].fd);
			/* XXX cannot do more now, maybe more luck next time */
			ret=-1;
		}else{
			blk_cache[i].modified=0; /* done */
		}
	}

	return ret;
}



int
io_blks(int fd,
#ifdef WIN32
		int fldrn,
#endif
		OFF64_T startoff,u_char *buf,u_int bstart,u_int bsize,u_int blksize,int cachealloc,int mode)
{
	int i;
	u_int blk;
	u_int chunkstart,chunksize;

	if (buf==NULL){
		return -1;
	}

	if ((mode!=IO_BLKS_READ)&&(mode!=IO_BLKS_WRITE)){
		return -1;
	}

	/* Note: OFF64_T against overflow */
	if ((((OFF64_T)bstart)+((OFF64_T)bsize))>(OFF64_T)0xffffffff){ /* XXX */
		return -1;
	}
#ifdef WIN32
	if ((fd<0)&&(fldrn<0)){
		return -1;
	}
#else /* !WIN32 */
	if (fd<0){
		return -1;
	}
#endif

	/* check every block */
	chunkstart=bstart;
	chunksize=0; /* no chunk of missing blocks so far */
	for (blk=bstart;blk<(bstart+bsize);blk++){
		/* look in cache */
		i=find_blk_cache(fd,
#ifdef WIN32
						 fldrn,
#endif
						 startoff,blk,blksize);
		/* Note: no match for blk if blksize has changed!!! */
		/*       however, these cache entries can be reused if needed */
		if (i<0){ /* not found in cache? */
			if (chunksize==0){ /* new chunk of missing blocks? */
				chunkstart=blk; /* start of chunk */
			}
			chunksize++; /* one more missing block */
		}else{
			/* found in cache */
			/* Note: now, blk_cache[i].buf!=NULL */

			/* Note: must do cache-I/O first, since io_blks_direct could steal i from cache */
			if (mode==IO_BLKS_WRITE){
				/* copy to cache */
				bcopy(buf+(blk-bstart)*blksize,blk_cache[i].buf,blksize);
				blk_cache[i].modified=1; /* modified */
			}else{
				/* copy from cache */
				bcopy(blk_cache[i].buf,buf+(blk-bstart)*blksize,blksize);
			}
			blk_cache_aging(i); /* adjust ages */

			/* chunk of missing blocks pending? */
			if (chunksize>0){
				/* must read or write */
				if (io_blks_direct(fd,
#ifdef WIN32
									fldrn,
#endif
									startoff,
									(u_char *)(buf+(chunkstart-bstart)*blksize),
									chunkstart,
									chunksize,
									blksize,
									cachealloc,
									mode)<0){
					return -1;
				}
				chunksize=0; /* no chunk of missing blocks anymore */
			}
		}
	}
	/* one last time: chunk of missing blocks pending? */
	if (chunksize>0){
		/* must read or write */
		if (io_blks_direct(fd,
#ifdef WIN32
							fldrn,
#endif
							startoff,
							(u_char *)(buf+(chunkstart-bstart)*blksize),
							chunkstart,
							chunksize,
							blksize,
							cachealloc,
							mode)<0){
			return -1;
		}
		chunksize=0; /* no chunk of missing blocks anymore */
	}

	return 0;
}



/* EOF */
