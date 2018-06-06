/***********************************************************************
*	NAME
*		read_ecat2 - c interface to read_ecat2.pro
*
*	SYNOPSIS
*		read_ecat2 file frame,plane -debug
*
*	PARAMETERS
*		file - file to read
*		bnum - list of block numbers to read
*
*	DESCRIPTION
*		'C' interface to read_ecat2.pro.  Allows ecat format images
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
*				also decrease exponent by 2 where the location of the
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
// extern void swab(char *from, char *to, int nbytes);

int debug=0;

int main_soff[] = { 48,50,52,54,66,68,70,72,74,76,134,136,138,
	148,150,152,158,160,350,352,354,376,378,380,382,452,454,456, -1 };
int main_loff[] = { -1 };
int main_foff[] = { 86,122,126,130,140,144,154,384,388,392,396,
	400,404,408,412,416,420,424,428,432,436,440,444,448,458, -1 };

int scan_soff[] = { 126,132,134,136,138,170,192,194, -1 };
int scan_loff[] = { 172,176,196,200,204,208, 452,456, 460, -1 };
int scan_foff[] = { 146,166,182,316,320,324,328,332,336,340,344,
	348,352,356,360,364,368,372,376,380,384,388,392,396,400,404,408,
	412,416,420,424,428,432,436,440,444,448,464, -1 };

int norm_soff[] = { 126,132,134,186,188,190,192,194,196, -1 };
int norm_loff[] = { -1 };
int norm_foff[] = { 182,198, -1 };

int attn_soff[] = { 126,128,132,134, -1 };
int attn_loff[] = { -1 };
int attn_foff[] = { 182,186,190,194,198,202,206,210, -1 };

int image_soff[] = { 126,128,132,134,176,178,200,202,204,206,236,
	376,380,382,384,386, -1 };
int image_loff[] = { 192,196,208,238,242,246, -1 };
int image_foff[] = { 160,164,168,172,184,188,296,300,304,308,388,
	392,396,400,404,408,412,416, -1 };


/*---------------------------------------*/
/*   U T I L I T Y   F U N C T I O N S   */
/*---------------------------------------*/

/* dump a block as hex values */
void dump_block(const unsigned char *ptr)
{
  int i,j;
  for (i = 0; i < 512; i+=32) {
    for (j = 0; j < 32; j+=4)
      fprintf(stderr,"%-8d ",i+j); 
    fprintf(stderr,"\n");
    for (j = 0; j < 32; j+=4)
      fprintf(stderr,"%2.2x%2.2x%2.2x%2.2x ", 
	      ptr[i+j],ptr[i+j+1],ptr[i+j+2],ptr[i+j+3]);
    fprintf(stderr,"\n");
  }
}

/*-----------------------------------------------------*/
/* routines to extract numbers from character pointers */
/*-----------------------------------------------------*/

short int get_short(const unsigned char *ptr)
{
  int i;
  union { unsigned short int i2; unsigned char byte[2]; } conv;
  for (i=0; i<2; i++) conv.byte[i] = ptr[i];
  return(conv.i2);
}

/*--------------*/

int get_int(const unsigned char *ptr)
{
  int i;
  union { int i4; unsigned char byte[4]; } conv;
  for (i=0; i<4; i++) conv.byte[i] = ptr[i];
  return(conv.i4);
}

/*--------------*/
float get_float(const unsigned char *ptr)
{
  int i;
  union { float f4; unsigned char byte[4]; } conv;
  for (i=0; i<4; i++) conv.byte[i] = ptr[i];
  return(conv.f4);
}

/*-----------------------------*/
/* routines to rearrange bytes */
/*-----------------------------*/

/* swap bytes in a word (0,1,2,3 -> 2,3,0,1) */
void swaw (short int from[], short int to[], int length)
{
  short int temp;
  int i;

  length--; /* if odd dont swap last words */
  for (i=0; i<length; i+=2) {
    temp = from[i+1];
    to[i+1] = from[i];
    to[i] = temp;
  }
}

/*--------------*/

/* swap bytes in a short int (0,1 -> 1,0) */
void swaper(unsigned char *data)
{
  unsigned char tmp;
	
  tmp = data[0];   data[0] = data[1];   data[1] = tmp;
}

/*--------------*/

/* swap bytes in a word (0,1,2,3 -> 3,2,1,0) */
void lswaper(unsigned char *data)
{
  unsigned char tmp;

  tmp = data[0];   data[0] = data[3];   data[3] = tmp;
  tmp = data[1];   data[1] = data[2];   data[2] = tmp;
}

/*--------------*/

/* swap bytes in a word (0,1,2,3 -> 1,0,3,2) */
void lswaper2(unsigned char *data)
{
  unsigned char tmp;

  tmp = data[0];   data[0] = data[1];   data[1] = tmp;
  tmp = data[2];   data[2] = data[3];   data[3] = tmp;
}

/*--------------*/

/* swap bytes and bit patterns for a float */
void fswaper(unsigned char *data)
{
  unsigned char tmp1, tmp2;
  unsigned short exp;

  /* zero check */
  if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) && (data[3] == 0)) {
    return;
  }
  if (debug > 0) fprintf(stderr, "  0x%2.2x%2.2x%2.2x%2.2x",
			 data[0],data[1],data[2],data[3]);

  /* rearrange bytes */
  lswaper2(data);
  if (debug > 0) fprintf(stderr, "  0x%2.2x%2.2x%2.2x%2.2x",
			 data[0],data[1],data[2],data[3]);

  /* adjust exponent */
  tmp1 = data[0] & 0x7f;   tmp2 = data[1] & 0x80;
  if (debug > 0) fprintf(stderr,"   (%2.2x,%2.2x)", tmp1,tmp2);
  tmp1 = (tmp1 << 1);   tmp2 = (tmp2 >> 7);
  exp = tmp1 | tmp2;
  if (debug > 0) fprintf(stderr,"   (%d, %2.2x,%2.2x)", exp,tmp1,tmp2);
  exp--;	/* vms is excess 128, sun is excess 127 */
  exp--;	/* ??? */
  tmp1 = (exp & 0x00fe) >> 1;
  data[0] = (data[0] & 0x80) | tmp1;
  tmp2 = (exp & 0x0001) << 7;
  data[1] = (data[1] & 0x7f) | tmp2;
  if (debug > 0)
    fprintf(stderr, "  (%d, %2.2x,%2.2x)   0x%2.2x%2.2x%2.2x%2.2x (%f)\n",
	    exp, tmp1,tmp2, data[0],data[1],data[2],data[3], get_float(data));

}

/*--------------*/

void read_plane(FILE *fp, int file_type, MatDir *mlist, int nmat,
		int frame, int plane, int doScale, char *outbuff,
		int outbuffbytes, double *planesquantscale)
{
  int blk0, nblk;
  unsigned char block[512];
  unsigned char *out_block;
  int outwords=0;
  int outwordsize=1;
  unsigned char scaled_block[512*4];
  int nitem;
  float quantScale;	/* scaling applied to image files */
  int dataType;		/* matrix data type */
  int bnum;
  int loop;
  char *endoutbuff;

  char *routine="read_plane";
  endoutbuff = outbuff + outbuffbytes;

  if (findBlocks(mlist,nmat, frame,plane, &blk0,&nblk)) {
    fprintf(stderr, "%s: error locating frame %d, plane %d\n", 
	    routine,frame,plane);
    return;
  }
  /* always need dataType, need quantScale for images */
  read_block(fp, blk0, (char *)block, 1);
#ifndef USES_LITTLE_ENDIAN
  swaper(&block[DATA_TYPE]);
#endif
  dataType = get_short(&block[DATA_TYPE]);
  if(file_type == IMAGE_FILE) {
    fswaper(&block[QUANT_SCALE]);
#ifdef USES_LITTLE_ENDIAN
    lswaper(&block[QUANT_SCALE]);
#endif
    quantScale = get_float(&block[QUANT_SCALE]);
  }
  else {
    fswaper(&block[SCALE_FACTOR]);
#ifdef USES_LITTLE_ENDIAN
    lswaper(&block[SCALE_FACTOR]);
#endif
    quantScale = get_float(&block[SCALE_FACTOR]);
  }
  *planesquantscale=quantScale;
  if (debug > 2) fprintf(stderr,"%s: plane=%d, dataType=%d, quantScale=%f\n",
			 routine,plane,dataType,quantScale);
  else if (debug) fprintf(stderr,".");
  /* read the data */
  for (bnum = blk0+1; bnum < blk0+nblk; bnum++) {
    read_block(fp,bnum,(char *)block,1);
    /* do the appropriate byte swapping/data conversion */
    switch (dataType) {
    case GENERIC:
    case BYTE_TYPE:
      outwordsize=1;
      outwords=512;
      break;
    case SUN_I2:
      outwordsize=2;
      outwords=256;
#ifdef USES_LITTLE_ENDIAN
      for (loop = 0; loop < 512; loop += 2)
	swaper(&block[loop]);
#endif
      break;
    case SUN_R4:
    case SUN_I4:
      outwordsize=4;
      outwords=128;
#ifdef USES_LITTLE_ENDIAN
      for (loop = 0; loop < 512; loop += 4)
	lswaper(&block[loop]);
#endif
      break;
    case VAX_I2:
      outwordsize=2;
      outwords=256;
#ifndef USES_LITTLE_ENDIAN
      for (loop = 0; loop < 512; loop += 2)
	swaper(&block[loop]);
#endif
      break;
    case VAX_I4:
      outwordsize=4;
      outwords=128;
#ifndef USES_LITTLE_ENDIAN
      for (loop = 0; loop < 512; loop += 4)
	lswaper(&block[loop]);
#endif
      break;
    case VAX_R4:
      outwordsize=4;
      outwords=128;
      for (loop = 0; loop < 512; loop += 4) {
	fswaper(&block[loop]);
#ifdef USES_LITTLE_ENDIAN
	lswaper(&block[loop]);
#endif
      }
      break;
    }
    /* scale images by quantScale */
    if (doScale) {
      unsigned char *cptr;
      short *sptr;
      int *iptr;
      float *fptr;
      float *fptrSB;
      if ((debug > 2) && (bnum == blk0+1))
	fprintf(stderr, "%s: scaling data\n", routine);
      fptrSB = (float *) scaled_block;
      outwordsize=4;
      switch (dataType) {
      case GENERIC:
      case BYTE_TYPE:
	cptr = (unsigned char *) block;
	for (loop = 0; loop < outwords; loop+=1)
	  *fptrSB++ = quantScale * *cptr++;
	break;
      case SUN_I2:
      case VAX_I2:
	sptr = (short *) block;
	for (loop = 0; loop < outwords; loop+=1)
	  *fptrSB++ = quantScale * *sptr++;
	break;
      case SUN_I4:
      case VAX_I4:
	iptr = (int *) block;
	for (loop = 0; loop < outwords; loop+=1)
	  *fptrSB++ = quantScale * *iptr++;
	break;
      case SUN_R4:
      case VAX_R4:
	fptr = (float *) block;
	for (loop = 0; loop < outwords; loop+=1)
	  *fptrSB++ = quantScale * *fptr++;
	break;
      }
      out_block=(unsigned char *)scaled_block;
    }
    else {
      out_block=(unsigned char *)block;
    }

    if(outbuff==NULL){
      nitem = fwrite(out_block, outwordsize, outwords, stdout);
      if (debug > 1) fprintf(stderr,"%d ", nitem*1);
    }
    else {
      char *ptr;
      int outbytes=outwords*outwordsize;
      for(loop=0, ptr=(char *)out_block;
	  (loop<outbytes) && (outbuff<endoutbuff);
	  loop++) *outbuff++ = *ptr++;
    }
  }
}

int read_header(FILE *fp, int blk, unsigned char block[], 
		int *soff, int *loff, int *foff)
{
  int *ptr;
  int err;
  err=read_block(fp, blk, (char *)block, 1);
#ifndef USES_LITTLE_ENDIAN
  for (ptr = (int *) soff; *ptr > -1; ptr++)
    swaper(&block[*ptr]);
  for (ptr = (int *) loff; *ptr > -1; ptr++)
    lswaper(&block[*ptr]);
#endif
  for (ptr = (int *) foff; *ptr > -1; ptr++) {
    fswaper(&block[*ptr]);
#ifdef USES_LITTLE_ENDIAN
    lswaper(&block[*ptr]);
#endif
  }
  return(err);
}

/* read a given block into a buffer */
int read_block(FILE *fptr, int blkno, char *bufr, int nblks)
{
  int err;
/*	int fseek(), fread();
*/
  err = fseek(fptr, (blkno-1)*MatBLKSIZE, 0);
  if (err) return (err);
  err = fread(bufr, 1, nblks*MatBLKSIZE, fptr);
  if (err != nblks*MatBLKSIZE)
    return (-1);
  return (0);
}

/*-----------------------------------------------------------------*/

/* decode the matrix number */
void decodeMatnum(int matnum, int *frame, int *plane, int *gate,
		 int *data, int *bed)
{
  int loPlane;
  int hiPlane;
  int loData;
  int hiData;

  *frame = matnum & 0x1FF;
  loPlane = (matnum >> 16) & 0xFF;
  hiPlane = (matnum >> 1) & 0x300;

  *plane = loPlane | hiPlane;

  *gate = (matnum >> 24) & 0x3F;

  loData = (matnum >> 30) & 0x3;
  hiData = (matnum >> 9) & 0x4;
  *data = loData | hiData;

  *bed = (matnum >> 12) & 0xF;

    if(debug > 4) fprintf(stderr,
	  "decodeMatnum: matnum=%d, frame,plane=%d,%d, gate,data,bed=%d,%d,%d\n",
	  matnum, *frame, *plane, *gate, *data, *bed);

}

/*-----------------------------------------------------------------*/

/* return blocks for a frame/plane */
int findBlocks(MatDir mlist[],int nmat, int frame, int plane,
	       int *blk0, int *nblk)
#if 0
MatDir mlist[];	       /* matrix directory */
int nmat;	       /* number of matrix entries */
int frame, plane;      /* frame,plane desired */
int *blk0, *nblk;      /* starting block and number of blocks in frame/plane */
#endif
{
  MatDir *ptr;
  int num;
  int thisFrame, thisPlane, thisGate, thisData, thisBed;

  for(num = 0, ptr = mlist; num < nmat; num++, ptr++) {
    /* check frame/plane number */
    decodeMatnum(ptr->matnum, &thisFrame, &thisPlane, &thisGate,
		 &thisData, &thisBed);
    if(debug > 4) fprintf(stderr,
      "findBlocks: num=%d, frame,plane=%d,%d, gate,data,bed=%d,%d,%d\n",
			  num, thisFrame, thisPlane, thisGate, 
			  thisData, thisBed);
    if((thisFrame == frame) && (thisPlane == plane)) {
      *blk0 = ptr->strtblk;
      *nblk = ptr->endblk - ptr->strtblk + 1;
      return(0);
    }
  }
  /* not found */
  fprintf(stderr,
  "findBlocks: cannot find entry for frame %d, plane %d\n", frame,plane);
  return(-1);
}

/*-----------------------------------------------------------------*/

/* return largest plane number for a given frame */
int maxPlaneNumber(MatDir mlist[], int nmat, int frame)
{
  MatDir *ptr;
  int num;
  int thisFrame, thisPlane, thisGate, thisData, thisBed;
  int maxPlane;
  
  maxPlane = 0;
  for (num = 0, ptr = mlist; num < nmat; num++, ptr++) {
    /* check frame/plane number */
    decodeMatnum(ptr->matnum, &thisFrame, &thisPlane, &thisGate,
		 &thisData, &thisBed);
    if(frame > 0) {
      if((frame == thisFrame) && (thisPlane > maxPlane))
	maxPlane = thisPlane;
    } else if(thisPlane > maxPlane) maxPlane = thisPlane;
  }
  return(maxPlane);
}

/*-----------------------------------------------------------------*/

/* return largest frame number */
int maxFrameNumber(MatDir mlist[], int nmat)
{
  MatDir *ptr;
  int num;
  int thisFrame, thisPlane, thisGate, thisData, thisBed;
  int maxFrame;
  
  maxFrame = 0;
  for (num = 0, ptr = mlist; num < nmat; num++, ptr++) {
    /* check frame/plane number */
    decodeMatnum(ptr->matnum, &thisFrame, &thisPlane, &thisGate,
		 &thisData, &thisBed);

    if(thisFrame > maxFrame) maxFrame = thisFrame;
  }
  return(maxFrame);
}

/*-----------------------------------------------------------------*/

/* build directory of matrix locations */
int buildMatrixDirectory(FILE *fptr, MatDir mlist[], int lmax)
{
  int blk, num_entry, num_stored, i, err;
  int nfree, nxtblk, prvblk, nused, matnum, strtblk, endblk, matstat;
  int dirbufr[MatBLKSIZE/4];
#ifndef USES_LITTLE_ENDIAN
  char bytebufr[MatBLKSIZE];
#endif

  blk = MatFirstDirBlk;
  num_entry = 0;
  num_stored = 0;
  while (1) {
    err = read_block(fptr, blk, (char *) dirbufr,1);
#ifndef USES_LITTLE_ENDIAN
/* swap bytes in words (0,1,2,3 -> 3,2,1,0) */
    swab((char *)dirbufr, bytebufr, MatBLKSIZE);
    swaw((short*)bytebufr, (short*)dirbufr, MatBLKSIZE/2);
#endif
    nfree  = dirbufr[0];
    nxtblk = dirbufr[1];
    prvblk = dirbufr[2];
    nused  = dirbufr[3];
    if (debug) fprintf(stderr,
     "buildMatrixDirectory: blk=%d, nfree=%d, nxtblk,prvblk=%d,%d, nused=%d\n",
		       blk,nfree,nxtblk,prvblk,nused);
    for (i=4; i<MatBLKSIZE/4; i+=4) {
      matnum  = dirbufr[i];
      strtblk = dirbufr[i+1];
      endblk	= dirbufr[i+2];
      matstat = dirbufr[i+3];
      if (debug > 4) fprintf(stderr,
"buildMatrixDirectory: matnum=%d (0x%8.8x), strtblk,endblk=%d,%d, matstat=%d\n",
			     matnum,matnum,strtblk,endblk,matstat);
      if (matnum && num_stored < lmax) {
	mlist[num_stored].matnum  = matnum;
	mlist[num_stored].strtblk = strtblk;
	mlist[num_stored].endblk  = endblk;
	mlist[num_stored].matstat = matstat;
	num_stored++;
      }
      if (matnum) num_entry++;
    }
    blk = nxtblk;
    if (blk == MatFirstDirBlk) break;
  }
  return (num_entry);
}
