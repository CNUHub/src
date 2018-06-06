/***********************************************************************
*	NAME
*		write_ecat2 - c interface to write_ecat2.pro
*
*	SYNOPSIS
*		write_ecat2 file frame,plane -debug
*
*	PARAMETERS
*		file - file to write
*		bnum - list of block numbers to write
*
*	DESCRIPTION
*		'C' interface to write_ecat2.pro.  Allows ecat format images
*		to be read with the data reformated to Sun format.  The
*		reformating of the data is done on the basis of the DATA_TYPE
*		field in the individual matrix sub-headers.  The headers (both
*		main and sub-headers) are reformated according to a list of
*		offsets,one for short words, one for long words, and one for
*		floats.
*
*		The reformatting needed is:
*			2 byte integer:  0,1 -> 1,0
*			4 byte integer:  0,1,2,3 -> 3,2,1,0
*			4 byte float:    0,1,2,3 -> 1,0,3,2
*				also increase exponent by 2 where the location of the
*				fields in the float number are:
*					(s=sign bit, e=exponent, m=mantissa)
*					seee eeee   emmm mmmm   mmmm mmmm   mmmm mmmm
*
*		All headers in a ecat file are 1, 512 byte block.  The structure
*		of a ecat file is:
*			main header
*			foreach frame
*				matrix directory
*				foreach plane
*					matrix subheader
*					matrix data
*		(TBD - the impact of gates,beds,data)
*
*		The matrix directory consists of one or more blocks.  Each block
*		consists of 4 byte integers which must be reformated as above.
*		Each directory block contains a 4 word header, and then up to
*		31 frame/plane entries.  The 4 word header contains:
*			(0) - the number of free frame/plane entries in this block
*			(1) - the block number of the next directory block
*			(2) - the block number of the previous directory block
*			(3) - the number of frame/plane entries used in this directory block
*		Each frame/plane entry contains:
*			(0) - the matrix number for this frame/plane
*			(1) - the starting block number (actually the block for the sub-header)
*			(2) - the ending block number
*			(3) - the matrix status (TBD - what this field means)
*		(The logic for decoding the matrix directories is contained in the
*		function buildMatrixDirectory.)
*
*		The matrix number has encoded in it, the frame number and plane
*		number for this frame/plane.  This encoding is done as follows:
*			ddgg gggg   pppp pppp   bbbb DPPf   ffff ffff
*			where:
*				frame = fffffffff (9 bits)
*				plane = PPpppppppp (10 bits)
*				gate = gggggg (6 bits)
*				bed = bbbb (4 bits)
*				data = Ddd (3 bits)
*		(The logic for decoding the matrix number is in the function decodeMatnum.)
*
*	HISTORY
*		04/15/93 - kirt - original code
*		4/16/93 - kas - embedded the flip locations (both short and long)
*		4/21/93 - kas - added 3D capability and matrixDirectory logic
*		4/22/93 - kas - added doc and decodeMatnum routine
*
************************************************************************
*	$Id$
*
*	$Log$
***********************************************************************/

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include "read_ecat2.h"

/* don't know where this system routine is declared */
//extern void swab(char *from, char *to, int nbytes);


/*---------------------------------------*/
/*   U T I L I T Y   F U N C T I O N S   */
/*---------------------------------------*/

/*-----------------------------------------------------*/
/* routines to copy bytes from character pointers */
/*-----------------------------------------------------*/

void copy_bytes(unsigned char *inptr, unsigned char *outptr, int nbytes)
{
     while(nbytes-- > 0)*outptr++ = *inptr++;
}


/*-----------------------------*/
/* routines to rearrange bytes */
/*-----------------------------*/

/* swap bytes and bit patterns for a float */
void ifswaper(unsigned char *data)
{
	unsigned char tmp1, tmp2;
	unsigned short exp;

	/* zero check */
	if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) && (data[3] == 0))
	{
		return;
	}

	/* adjust exponent */
	/* get exponent */
	tmp1 = data[0] & 0x7f;   tmp2 = data[1] & 0x80;
	tmp1 = (tmp1 << 1);   tmp2 = (tmp2 >> 7);
	exp = tmp1 | tmp2;
	/* increment exponent */
	exp++;	/* vms is excess 128, sun is excess 127 */
	exp++;	/* ??? */
	/* return exponent */
	tmp1 = (exp & 0x00fe) >> 1;
	data[0] = (data[0] & 0x80) | tmp1;
	tmp2 = (exp & 0x0001) << 7;
	data[1] = (data[1] & 0x7f) | tmp2;

	/* rearrange bytes */
	lswaper2(data);
}


void write_header(FILE *fp, int blk, unsigned char header[], 
	    int *soff, int *loff, int *foff)
{
  unsigned char block[MatBLKSIZE];
  int *ptr;
  int i;
  for(i=0; i<MatBLKSIZE; i++)block[i]=header[i];
#ifndef USES_LITTLE_ENDIAN
  for (ptr = (int *) soff; *ptr > 0; ptr++)
    swaper(&block[*ptr]);
  for (ptr = (int *) loff; *ptr > 0; ptr++)
    lswaper(&block[*ptr]);
#endif
  for (ptr = (int *) foff; *ptr > 0; ptr++) {
#ifdef USES_LITTLE_ENDIAN
    /* swap little endian to big */
    lswaper(&block[*ptr]);
#endif
    /* fix floating point to vax format while converting to little_endian */
    ifswaper(&block[*ptr]);
  }
  write_block(fp, blk, (char *)block, 1);
}
/*--------------*/

void write_plane(FILE *fp, MatDir *mlist, int nmat, int frame, int plane,
		 char *inbuff, int inbuffbytes,
		 char *header, int *soff, int *loff, int *foff)
{
  int blk0, nblk;
  unsigned char block[512];
  //  int nitem;
  int bnum;
  int loop;
  char *ptr;
  int dataType;
  char *endinbuff;

  char *routine="write_plane";

  endinbuff = inbuff + inbuffbytes;
  dataType = get_short(&header[DATA_TYPE]);
  if (findBlocks(mlist,nmat, frame,plane, &blk0,&nblk))
    {
      fprintf(stderr, "%s: error locating frame %d, plane %d\n", 
	      routine,frame,plane);
      return;
    }
  /* write the header */
  write_header(fp, blk0, (unsigned char *)header, soff, loff, foff);
  /* write the data */
  for (bnum = blk0+1; bnum < blk0+nblk; bnum++) {
    for(loop=0, ptr=(char *)block; loop<512; loop++) {
      if(inbuff < endinbuff)*ptr++ = *inbuff++;
      else *ptr++=0;
    }
     /* do the appropriate byte swapping/data conversion */
    switch (dataType) {
    case GENERIC:
    case BYTE_TYPE:
    case SUN_R4:
    case SUN_I4:
      /* leave big endian alone */
#ifdef USES_LITTLE_ENDIAN
      /* swap little endian to big */
      for (loop = 0; loop < 512; loop += 4)
	lswaper(&block[loop]);
#endif
      break;
    case SUN_I2:
      /* leave big endian alone */
#ifdef USES_LITTLE_ENDIAN
      /* swap little endian to big */
      for (loop = 0; loop < 512; loop += 2)
	swaper(&block[loop]);
#endif
      break;
    case VAX_I2:
      /* leave little endian alone */
#ifndef USES_LITTLE_ENDIAN
      /* swap big endian to little */
      for (loop = 0; loop < 512; loop += 2)
	swaper(&block[loop]);
#endif
      break;
    case VAX_I4:
      /* leave little endian alone */
#ifndef USES_LITTLE_ENDIAN
      /* swap big endian to little */
      for (loop = 0; loop < 512; loop += 4)
	lswaper(&block[loop]);
#endif
      break;
    case VAX_R4:
      for (loop = 0; loop < 512; loop += 4) {
#ifdef USES_LITTLE_ENDIAN
	/* swap little endian to big */
	lswaper(&block[loop]);
#endif
	/* fix floating point to vax format while converting to little_endian */
	ifswaper(&block[loop]);
      }
      break;
    }
    write_block(fp,bnum,(char *)block,1);
  }
}

/* write a given block */
int write_block(FILE *fptr, int blkno, char *bufr, int nblks)
{
	int err;
	err = fseek(fptr, (blkno-1)*MatBLKSIZE, 0);
	if (err) return (err);
	err = fwrite(bufr, 1, nblks*MatBLKSIZE, fptr);
	if (err != nblks*MatBLKSIZE) return (-1);
	return (0);
}

/*-----------------------------------------------------------------*/

/* encode the matrix number */
void encodeMatnum(int frame, int plane, int gate, int data, int bed,
		  int *matnum)
{
	*matnum = (frame & 0x1FF) |
	  ((plane & 0xFF) << 16) | ((plane & 0x300) << 1) |
	    ((gate & 0x3f) << 24) |
	      ((data & 0x3) << 30) | ((data & 0x4) << 9) |
		((bed & 0xF) << 12);
}

/*-----------------------------------------------------------------*/

int writeMatrixDirectory(MatDir mlist[], int lmax, int nframes, int nplanes,
		      int bytesperplane, int gate, int data, int bed, FILE *fptr)
{
  int ml_indice;
  int frame;
  int plane;
  int matblk, nxtblk, prvblk;
  int nfree, nused;
  int strtblk, endblk;
  int wordsperblkframedir;
  int blksperframedir;
  int blksperplanedata;
  int matnum;
  int blkspersubheader=1;
  int dirbufr[MatBLKSIZE/4];
  int db_indice;
#ifndef USES_LITTLE_ENDIAN
  char bytebufr[MatBLKSIZE];
#endif
  int i;

/* initialize dirbufr */
  for(i=0; i<MatBLKSIZE/4; i++) dirbufr[i] = 0;

  /* calculate the number of full and partial blocks to hold frame matrix dir */
  wordsperblkframedir = (MatBLKSIZE/4) - 4; /* 4 word header on each blk */
  blksperframedir =
    (nplanes*4 + wordsperblkframedir - 1)/wordsperblkframedir;
  /* calculate the number of full and partial blocks needed to hold plane data */
  blksperplanedata = (bytesperplane + MatBLKSIZE - 1)/MatBLKSIZE;

  prvblk=matblk=MatFirstDirBlk;
  db_indice=4;
  nused=0;
  ml_indice=0;
  for(frame=1; frame<=nframes; frame++) {
    /* skip matrix directory for each frame */
    strtblk = matblk + blksperframedir;
    for(plane=1; ((plane<=nplanes)&&(ml_indice < lmax)); plane++) {
      endblk = strtblk + blkspersubheader + blksperplanedata - 1;
      encodeMatnum(frame, plane, gate, data, bed, &matnum);
      mlist[ml_indice].matnum = dirbufr[db_indice++] = matnum;
      mlist[ml_indice].strtblk = dirbufr[db_indice++] = strtblk;
      mlist[ml_indice].endblk = dirbufr[db_indice++] = endblk;
      mlist[ml_indice].matstat = dirbufr[db_indice++] = 0;
      strtblk=endblk+1;
      ml_indice++;
      nused++;
      if(db_indice >= MatBLKSIZE/4 || plane == nplanes) {
	if(plane == nplanes) {
/*	  nused = db_indice; */
	  if(frame==nframes)nxtblk = MatFirstDirBlk;
	  else nxtblk = strtblk;
	}
	else {
/*	  nused=db_indice; */
	  nxtblk=matblk + 1;
	}
	nfree = MatBLKSIZE/4 - nused;
	dirbufr[0]=nfree; dirbufr[1]=nxtblk;
	dirbufr[2]=prvblk; dirbufr[3]=nused;
/* swap bytes in words (0,1,2,3 -> 3,2,1,0) */
#ifndef USES_LITTLE_ENDIAN
	swab((char *)dirbufr, bytebufr, MatBLKSIZE);
	swaw((short *)bytebufr, (short *)dirbufr, MatBLKSIZE/2);
#endif
	write_block(fptr, matblk, (char *)dirbufr, 1);
	prvblk = matblk; matblk=nxtblk;
	db_indice=4;
	nused = 0;
/* re-initialize dirbufr */
	for(i=0; i<MatBLKSIZE/4; i++) dirbufr[i] = 0;
      }

    }
  }
  if(ml_indice == lmax) {
    fprintf(stderr, "createMatrixDirectory: mlist size limit reached\n");
  }
  return(ml_indice);
}
