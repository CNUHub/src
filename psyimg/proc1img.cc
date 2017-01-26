#include "psyhdr.h"

void proc1img::copyblock(char *outbuff, int xorig, int yorig, int zorig,
		       int iorig, int xend, int yend, int zend, int iend,
		       int out_xinc, int out_yinc,
		       int out_zinc, int out_iinc,
		       psytype pixeltype)
{
  char_or_largest_pixel inpixel;
  int x, y, z, i;
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psytype inputpsyimgtype=inputpsyimg->gettype();
// select most efficient loop to use
  ioptr=outbuff;
  if(xorig != orig.x || xend != end.x) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  inputpsyimg->initgetpixel(inputpsyimgtype, xorig, y, z, i);
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            inputpsyimg->getnextpixel(inpixel.c);
            convertpixel(inpixel.c, inputpsyimgtype, xoptr, pixeltype);
	  }//end for(x=
	  inputpsyimg->freegetpixel();
        }//end for(y=
      }//end for(z=
    }//end for(i=
  } else if(yorig != orig.y || yend != end.y) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	inputpsyimg->initgetpixel(inputpsyimgtype, xorig, yorig, z, i);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            inputpsyimg->getnextpixel(inpixel.c);
            convertpixel(inpixel.c, inputpsyimgtype, xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
        inputpsyimg->freegetpixel();
      }//end for(z=
    }//end for(i=
  } else if(zorig != orig.z || zend != end.z) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      inputpsyimg->initgetpixel(inputpsyimgtype, xorig, yorig, zorig, i);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            inputpsyimg->getnextpixel(inpixel.c);
            convertpixel(inpixel.c, inputpsyimgtype, xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
      inputpsyimg->freegetpixel();
    }//end for(i=
  } else {
    inputpsyimg->initgetpixel(inputpsyimgtype, xorig, yorig, zorig, iorig);
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            inputpsyimg->getnextpixel(inpixel.c);
            convertpixel(inpixel.c, inputpsyimgtype, xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
    }//end for(i=
    inputpsyimg->freegetpixel();
  }
}

void proc1img::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
// allows any request type even if not initialized to that type
  type=pixeltype;
  inputpsyimg->initgetpixel(inputpsyimg->gettype(), x, y, z, i);
}

void proc1img::getpixel(char *outpixel, psytype pixeltype,
			  int x, int y, int z, int i)
{
  char_or_largest_pixel inpixel;
// allows any request type even if not initialized to that type
  type=pixeltype;
  psytype inputpsyimgtype=inputpsyimg->gettype();
  inputpsyimg->getpixel(inpixel.c, inputpsyimgtype, x, y, z, i);
  convertpixel(inpixel.c, inputpsyimgtype, outpixel, pixeltype);
}

void proc1img::getnextpixel(char *outpixel)
{
  char_or_largest_pixel inpixel;
  inputpsyimg->getnextpixel(inpixel.c);
  convertpixel(inpixel.c, inputpsyimg->gettype(), outpixel, type);
}
