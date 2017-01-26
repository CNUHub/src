#include "psyhdr.h"

class psyaccum : public psyimglnk {
  psybuff buffimage;
 public:
  psyaccum();
  void initpsyaccum(psyimg *psyimgptr, psytype pixeltype=psydouble);
  void output_tree(ostream *out) {psyimglnk::output_tree(out);*out<<"::psyaccum";};
  void add2accum(psyimg *psyimgptr, psytype pixeltype=psydouble);
  void addsqr2accum(psyimg *psyimgptr, psytype pixeltype=psydouble);
  void incaccum(psyimg *psyimgptr, psytype pixeltype=psyushort, double threshold=1e-16);
  void inc_ge_thresholdimg(psyimg *psyimgptr, psyimg *threshimgptr, psytype pixeltype=psyushort);
  void inc_le_thresholdimg(psyimg *psyimgptr, psyimg *threshimgptr, psytype pixeltype=psyushort);
  void inc_mag_ge_thresholdimg(psyimg *psyimgptr, psyimg *threshimgptr, psytype pixeltype=psyushort);
  void minaccum(psyimg *psyimgptr, psytype pixeltype=psyushort);
  void maxaccum(psyimg *psyimgptr, psytype pixeltype=psyushort);
  void max_mag_accum(psyimg *psyimgptr, psytype pixeltype);
  void chknclearbuff();
  void getpixel(char *pixel, psytype pixeltype, int x, int y, int z, int i);
  void initgetpixel(psytype pixeltype, int x, int y, int z, int i);
  void freegetpixel() {buffimage.freegetpixel();};
  void getnextpixel(char *pixel);
  void copyblock(char *outbuff, int xorig, int yorig, int zorig,
		 int iorig, int xend, int yend, int zend, int iend,
		 int xinc, int yinc, int zinc, int iinc,
		 psytype pixeltype=psyuchar);
};

psyaccum::psyaccum() : psyimglnk()
{
}

void psyaccum::initpsyaccum(psyimg *psyimgptr, psytype pixeltype)
{
  initpsyimglnk(psyimgptr, pixeltype);
  chknclearbuff();
// after initialization we don't need the input image any more
  inputpsyimg=NULL;
}

void psyaccum::chknclearbuff() {
  if(buffimage.getbuff() == NULL) {
// initialize buffer to full image size
    buffimage.initbuff(size.x, size.y, size.z, size.i, type);
  }
// clear the buffer
  int length=buffimage.getlength();
  for(char *ptr=buffimage.getbuff(); length>0; length--, ptr++)*ptr=0;
}

void psyaccum::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			 int iorig, int xend, int yend, int zend, int iend,
			 int xinc, int yinc, int zinc, int iinc,
			 psytype pixeltype)
{
// check pixel size
  if(type != pixeltype) {
    output_tree(&cerr);
    cerr << ":psyfullbuff::copyblock - pixel types must be the same\n";
    exit(1);
  }
  if(!inside(xorig, yorig, zorig, iorig) || 
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr << ":psyfullbuff::copyblock - request outside of buffer\n";
    cerr<<"orig=("<<xorig<<","<<yorig<<","<<zorig<<","<<iorig<<")\n";
    cerr<<"end=("<<xend<<","<<yend<<","<<zend<<","<<iend<<")\n";

    exit(1);
  }

// transfer from buffer to output buffer
  buffimage.copyblock(outbuff, xorig, yorig, zorig, iorig,
		      xend, yend, zend, iend, xinc, yinc, zinc, iinc,
		      pixeltype);
}

void psyaccum::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  buffimage.initgetpixel(pixeltype,x,y,z,i);
}

void psyaccum::getpixel(char *pixel, psytype pixeltype,
			   int x, int y, int z, int i)
{
  buffimage.getpixel(pixel,pixeltype,x,y,z,i);
}

void psyaccum::getnextpixel(char *pixel)
{
// no checking - hopefully user called initgetpixel first
  buffimage.getnextpixel(pixel);
}

void psyaccum::add2accum(psyimg *psyimgptr, psytype pixeltype)
{
  if(length == 0) {
    initpsyimglnk(psyimgptr, pixeltype);
    chknclearbuff();
// after initialization we don't need psyimglnk pointer anymore
    inputpsyimg=NULL;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::add2accum - image size differs from accumulator\n";
    exit(1);
  }
  double invalue;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
// process with fastest method
  if((accumtype == psydouble) && (intype == psydouble)) {
    double *sumptr;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel((char *)&invalue);
	    sumptr=(double *)xoptr;
	    *sumptr += invalue;
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  } // end if(accumtype == psydouble && intype == psydouble
  else if(accumtype == psydouble) {
    char_or_largest_pixel inpixel;
    double *sumptr;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    sumptr=(double *)xoptr;
	    *sumptr += invalue;
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  } // end else if(accumtype == psydouble && intype != psydouble
  else {
    char_or_largest_pixel inpixel;
    double sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    type2double(xoptr, accumtype, (char *)&sum);
	    sum += invalue;
	    pixeltypechg((char *)&sum, psydouble, xoptr, accumtype);
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }// end else accumtype != psydouble
  psyimgptr->freegetpixel();
}

void psyaccum::addsqr2accum(psyimg *psyimgptr, psytype pixeltype)
{
  if(length == 0) {
    initpsyimglnk(psyimgptr, pixeltype);
    chknclearbuff();
// after initialization we don't need psyimglnk pointer anymore
    inputpsyimg=NULL;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::addsqr2accum - image size differs from accumulator\n";
    exit(1);
  }
  double invalue;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
// process with fastest method
  if((accumtype == psydouble) && (intype == psydouble)) {
    double *sumptr;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel((char *)&invalue);
	    sumptr=(double *)xoptr;
	    *sumptr += invalue*invalue;
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  } // end if(accumtype == psydouble && intype == psydouble
  else if(accumtype == psydouble) {
    char_or_largest_pixel inpixel;
    double *sumptr;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    sumptr=(double *)xoptr;
	    *sumptr += invalue*invalue;
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  } // end else if(accumtype == psydouble && intype != psydouble
  else {
    char_or_largest_pixel inpixel;
    double sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    type2double(xoptr, accumtype, (char *)&sum);
	    sum += invalue*invalue;
	    pixeltypechg((char *)&sum, psydouble, xoptr, accumtype);
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }// end else accumtype != psydouble
  psyimgptr->freegetpixel();
}

void psyaccum::incaccum(psyimg *psyimgptr, psytype pixeltype, double threshold)
{
  if(length == 0) {
    initpsyimglnk(psyimgptr, pixeltype);
    chknclearbuff();
// after initialization we don't need psyimglnk pointer anymore
    inputpsyimg=NULL;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::incaccum - image size differs from accumulator\n";
    exit(1);
  }
  double invalue;
  char_or_largest_pixel inpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  if(accumtype == psyushort) {
    unsigned short *sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    if(invalue > threshold) {
	      sum=(unsigned short *)xoptr;
	      (*sum)++;
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }
  else {
    double sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    if(invalue > threshold) {
	      type2double(xoptr, accumtype, (char *)&sum);
	      sum++;
	      pixeltypechg((char *)&sum, psydouble, xoptr, accumtype);
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }// end else
  psyimgptr->freegetpixel();
}

void  psyaccum::inc_ge_thresholdimg(psyimg *psyimgptr, psyimg *threshimgptr, psytype pixeltype) {
  if(length == 0) {
    initpsyimglnk(psyimgptr, pixeltype);
    chknclearbuff();
// after initialization we don't need psyimglnk pointer anymore
    inputpsyimg=NULL;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::inc_ge_thresholdimg - image size differs from accumulator\n";
    exit(1);
  }
  if(!samedim(threshimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::inc_ge_thresholdimg - threshold image size differs from accumulator\n";
    exit(1);
  }

  double invalue, threshvalue;
  char_or_largest_pixel inpixel, threshpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  psytype threshtype=threshimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  threshimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  if(accumtype == psyushort) {
    unsigned short *sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    threshimgptr->getnextpixel(threshpixel.c);
	    type2double(threshpixel.c, threshtype, (char *)&threshvalue);
	    if(invalue >= threshvalue) {
	      sum=(unsigned short *)xoptr;
	      (*sum)++;
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }
  else {
    double sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    threshimgptr->getnextpixel(threshpixel.c);
	    type2double(threshpixel.c, threshtype, (char *)&threshvalue);
	    if(invalue >= threshvalue) {
	      type2double(xoptr, accumtype, (char *)&sum);
	      sum++;
	      pixeltypechg((char *)&sum, psydouble, xoptr, accumtype);
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }// end else
  psyimgptr->freegetpixel();
  threshimgptr->freegetpixel();
}

void  psyaccum::inc_le_thresholdimg(psyimg *psyimgptr, psyimg *threshimgptr, psytype pixeltype) {
  if(length == 0) {
    initpsyimglnk(psyimgptr, pixeltype);
    chknclearbuff();
// after initialization we don't need psyimglnk pointer anymore
    inputpsyimg=NULL;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::inc_ge_thresholdimg - image size differs from accumulator\n";
    exit(1);
  }
  if(!samedim(threshimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::inc_ge_thresholdimg - threshold image size differs from accumulator\n";
    exit(1);
  }

  double invalue, threshvalue;
  char_or_largest_pixel inpixel, threshpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  psytype threshtype=threshimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  threshimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  if(accumtype == psyushort) {
    unsigned short *sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    threshimgptr->getnextpixel(threshpixel.c);
	    type2double(threshpixel.c, threshtype, (char *)&threshvalue);
	    if(invalue <= threshvalue) {
	      sum=(unsigned short *)xoptr;
	      (*sum)++;
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }
  else {
    double sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    threshimgptr->getnextpixel(threshpixel.c);
	    type2double(threshpixel.c, threshtype, (char *)&threshvalue);
	    if(invalue <= threshvalue) {
	      type2double(xoptr, accumtype, (char *)&sum);
	      sum++;
	      pixeltypechg((char *)&sum, psydouble, xoptr, accumtype);
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }// end else
  psyimgptr->freegetpixel();
  threshimgptr->freegetpixel();
}

void  psyaccum::inc_mag_ge_thresholdimg(psyimg *psyimgptr, psyimg *threshimgptr, psytype pixeltype) {
  if(length == 0) {
    initpsyimglnk(psyimgptr, pixeltype);
    chknclearbuff();
// after initialization we don't need psyimglnk pointer anymore
    inputpsyimg=NULL;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::inc_ge_thresholdimg - image size differs from accumulator\n";
    exit(1);
  }
  if(!samedim(threshimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::inc_ge_thresholdimg - threshold image size differs from accumulator\n";
    exit(1);
  }

  double invalue, threshvalue;
  char_or_largest_pixel inpixel, threshpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  psytype threshtype=threshimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  threshimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  if(accumtype == psyushort) {
    unsigned short *sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    threshimgptr->getnextpixel(threshpixel.c);
	    type2double(threshpixel.c, threshtype, (char *)&threshvalue);
	    if(fabs(invalue) >= fabs(threshvalue)) {
	      sum=(unsigned short *)xoptr;
	      (*sum)++;
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }
  else {
    double sum;
    for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
      for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
	for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	  for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	    psyimgptr->getnextpixel(inpixel.c);
	    type2double(inpixel.c, intype, (char *)&invalue);
	    threshimgptr->getnextpixel(threshpixel.c);
	    type2double(threshpixel.c, threshtype, (char *)&threshvalue);
	    if(fabs(invalue) >= fabs(threshvalue)) {
	      type2double(xoptr, accumtype, (char *)&sum);
	      sum++;
	      pixeltypechg((char *)&sum, psydouble, xoptr, accumtype);
	    }
	  }//end for(x=
	}//end for(y=
      }//end for(z=
    }//end for(i=
  }// end else
  psyimgptr->freegetpixel();
  threshimgptr->freegetpixel();
}

// set accumlated voxels to minimum of previously accumlated new image voxels
void psyaccum::minaccum(psyimg *psyimgptr, psytype pixeltype)
{
  if(length == 0) {
    add2accum(psyimgptr, pixeltype); // initialize min values to current psyimgptr
    return;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::minaccum - image size differs from accumulator\n";
    exit(1);
  }
  double invalue;
  char_or_largest_pixel inpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  double currentvalue;
  for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
    for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
      for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	  psyimgptr->getnextpixel(inpixel.c);
	  type2double(inpixel.c, intype, (char *)&invalue);
	  type2double(xoptr, accumtype, (char *)&currentvalue);
	  if(invalue < currentvalue) pixeltypechg((char *)&invalue, psydouble, xoptr, accumtype);
	}//end for(x=
      }//end for(y=
    }//end for(z=
  }//end for(i=
  psyimgptr->freegetpixel();
}

// set accumlated voxels to maximum of previously accumlated new image voxels
void psyaccum::maxaccum(psyimg *psyimgptr, psytype pixeltype)
{
  if(length == 0) {
    add2accum(psyimgptr, pixeltype); // initialize max values to current psyimgptr
    return;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::maxaccum - image size differs from accumulator\n";
    exit(1);
  }
  double invalue;
  char_or_largest_pixel inpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  double currentvalue;
  for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
    for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
      for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	  psyimgptr->getnextpixel(inpixel.c);
	  type2double(inpixel.c, intype, (char *)&invalue);
	  type2double(xoptr, accumtype, (char *)&currentvalue);
	  if(invalue > currentvalue) pixeltypechg((char *)&invalue, psydouble, xoptr, accumtype);
	}//end for(x=
      }//end for(y=
    }//end for(z=
  }//end for(i=
  psyimgptr->freegetpixel();
}

// set accumlated voxels to maximum (positive or negative) of previously accumlated and new image voxels based on magnitude
void psyaccum::max_mag_accum(psyimg *psyimgptr, psytype pixeltype)
{
  if(length == 0) {
    add2accum(psyimgptr, pixeltype); // initialize max values to current psyimgptr
    return;
  }
  if(!samedim(psyimgptr)) {
    output_tree(&cerr);
    cerr << ":psyaccum::maxaccum - image size differs from accumulator\n";
    exit(1);
  }
  double invalue;
  char_or_largest_pixel inpixel;
  int x, y, z, i;
  psydims orig=getorig();
  psydims end=getend();
  psydims inc=getinc();
  psytype accumtype=gettype();
  psytype intype=psyimgptr->gettype();
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psyimgptr->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  ioptr=buffimage.getbuff();
  double currentvalue;
  for(i=orig.i; i<=end.i; i++, ioptr+=inc.i) {
    for(z=orig.z, zoptr=ioptr; z<=end.z; z++, zoptr+=inc.z) {
      for(y=orig.y, yoptr=zoptr; y<=end.y; y++, yoptr+=inc.y) {
	for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
	  psyimgptr->getnextpixel(inpixel.c);
	  type2double(inpixel.c, intype, (char *)&invalue);
	  type2double(xoptr, accumtype, (char *)&currentvalue);
	  if(fabs(invalue) > fabs(currentvalue)) pixeltypechg((char *)&invalue, psydouble, xoptr, accumtype);
	}//end for(x=
      }//end for(y=
    }//end for(z=
  }//end for(i=
  psyimgptr->freegetpixel();
}

class sqrsum2std : public proc2img {
  int mincontributions;
  void proc2pixels(char *in1, psytype in1type,
		   char *in2, psytype in2type,
		   char *out, psytype outtype);
 public:
  sqrsum2std(psyimg *sqrsumimgptr, psyimg *cntimgptr,
	     int mincontributions=2, psytype pixeltype=psynotype);
  void output_tree(ostream *out) {proc2img::output_tree(out);
				 *out<<"::sqrsum2std";};
};

sqrsum2std::sqrsum2std(psyimg *sqrsumimgptr, psyimg *cntimgptr,
		       int mincontributions,
		       psytype pixeltype) :
	    proc2img(sqrsumimgptr, cntimgptr, pixeltype)
{
  if(mincontributions < 2) {
    output_tree(&cerr);
    cerr << ":sqrsum2std::sqrsum2std - standardiviation requires at least 2 contributions\n";
    exit(1);
  }
  sqrsum2std::mincontributions=mincontributions;
}

void sqrsum2std::proc2pixels(char *in1, psytype in1type,
			     char *in2, psytype in2type,
			     char *out, psytype outtype)
{
  double std_pixel, sqr_sum_pixel, cnt_pixel;
// convert to double to simplify math
  type2double(in2, in2type, (char *)&cnt_pixel);
// do actual processing
  if(cnt_pixel < mincontributions)std_pixel=0;
  else {
    type2double(in1, in1type, (char *)&sqr_sum_pixel);
    if(sqr_sum_pixel < 0)std_pixel=0;
    else std_pixel = sqrt(sqr_sum_pixel/(cnt_pixel-1));
  }
// convert back to desired output type
  pixeltypechg((char *)&std_pixel, psydouble, out, outtype);
}

char **read_file_list(char *name, int *numberoffiles, int *numberoftemplates)
{
  char linebuff[1024], c, *cptr;
  fstream fd;
  fd.open(name, ios::in);
  if(fd.fail() || fd.bad()) {
    cerr<<"read_file_list - unable to open file "<<name<<'\n';
    exit(1);
  }
// first count number of files
  int numfiles=0;
  while(! fd.eof() && fd.good()) {
    //remove left over carriage return
    if(fd.peek() == '\n')fd.get(c);
    linebuff[0] = '\0';
    fd.get(linebuff, 1024-1);
    if(fd.good()) {
      // parse line
      if(linebuff[0] == '#') ; //ignore comment lines
      else if(linebuff[0] == '\0') ; //ignore blank lines
      else numfiles++;
    }// end if(fd.good())
  }// end while

// next build pointer array and read values
  char **namelist=new char *[2*numfiles+1];
  int i=0;
  for(i=0; i<2*numfiles+1; i++)namelist[i]=NULL;
  i=0;
  int numtemplates=0;
  //  fd.close();
  fd.clear();
  //  fd.open(name, ios::in);
  fd.seekg(ios::beg);
  if(fd.fail() || fd.bad()) {
    cerr<<"read_file_list - unable to seek to beginning of file "<<name<<'\n';
    exit(1);
  }
  while(! fd.eof() && fd.good()) {
    //remove left over carriage return
    if(fd.peek() == '\n')fd.get(c);
    linebuff[0] = '\0';
    fd.get(linebuff, 1024-1);
    if(fd.good()) {
      // parse line
      if(linebuff[0] == '#') ; //ignore comment lines
      else if(linebuff[0] == '\0') ; //ignore blank lines
      else {
	char *token;
	token=strtok(linebuff," =\t\n\0");
	namelist[i]=new char[strlen(token) + 1];
	strcpy(namelist[i], token);
	token=strtok(NULL," =\t\n\0");
	if(token != NULL) {
	  namelist[numfiles+numtemplates]=new char[strlen(token) + 1];
	  strcpy(namelist[numfiles+numtemplates], token);
	  numtemplates++;
	}
	i++;
      }// end else linebuff[0] != '#'
    }// end if(fd.good())
  }// end while

  fd.close();
  if(numberoffiles != NULL)*numberoffiles=numfiles;
  if(numberoftemplates != NULL)*numberoftemplates=numtemplates;
  return(namelist);
}

void free_read_file_list(char **ptr)
{
  int i=0;
  while(ptr[i] != NULL)delete[] ptr[i++];
  delete[] ptr;
}

int main(int argc, char *argv[])
{
  char *avgfile=NULL;
  char *stdfile=NULL;
  char *cntfile=NULL;
  char *minfile=NULL;
  char *maxfile=NULL;
  char *maxmagfile=NULL;
  char *listfile=NULL;

  char *imgtorank=NULL;
  char *rank_cnt_file=NULL;
  char *rankprobfile=NULL;
  int rankmode=0;

  double min, max, mean;
  psytype outtype=psynotype; // will default to largest input type
  psyfileclass outfileclass=psynoclass; // defaults to first input class
  int template_flag=0;
  int divide_template_flag=0;
// allocate a name list large enough to point to all input file names //
  char *commandlinelist[argc];
  char **namelist=commandlinelist;
  int numinputfiles=0;
  int numtemplates=0;
  int minavgcount=1;
  int minstdcount=2;
  int norm_with_template=0;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  int i;
  for(i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" ( [-t|-td [-nt]] -list list_file\n";
      cout<<" | infile1 [infile2 [infile3 [...]]]\n";
      cout<<"   [-t|-td [-nt] templatefile1 [templatefile2 [...]]] )"<<'\n';
      cout<<" [-avg avgfile [-mac min_avg_count]]\n";
      cout<<" [-std stdfile [-msc min_std_count]]\n";
      cout<<" [-min minfile] [-max maxfile] [-maxmag maxmagfile]\n";
      cout<<" [-cnt cntfile]\n";
      cout<<" [-rank imgtorank [-r2tail|-rnegtail] [-rcnt rank_cnt_outfile] [-rprob rankproboutfile]]\n";
      cout<<" ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout<<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-t", argv[i])==0)template_flag=1;
    else if(strcmp("-td", argv[i])==0)divide_template_flag=1;
    else if(strcmp("-nt", argv[i])==0)norm_with_template=1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if((strcmp("-list", argv[i])==0) && ((i+1)<argc))listfile=argv[++i];
    else if((strcmp("-avg", argv[i])==0) && ((i+1)<argc))avgfile=argv[++i];
    else if((strcmp(argv[i],"-mac")==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&minavgcount);
    else if((strcmp("-std", argv[i])==0) && ((i+1)<argc))stdfile=argv[++i];
    else if((strcmp(argv[i],"-msc")==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&minstdcount);
    else if((strcmp("-cnt", argv[i])==0) && ((i+1)<argc))cntfile=argv[++i];
    else if((strcmp("-min", argv[i])==0) && ((i+1)<argc))minfile=argv[++i];
    else if((strcmp("-max", argv[i])==0) && ((i+1)<argc))maxfile=argv[++i];
    else if((strcmp("-maxmag", argv[i])==0) && ((i+1)<argc))maxmagfile=argv[++i];
    else if((strcmp("-rank", argv[i])==0) && ((i+1)<argc))imgtorank=argv[++i];
    else if((strcmp("-r2tail", argv[i])==0))rankmode=1;
    else if((strcmp("-rnegtail", argv[i])==0))rankmode=2;
    else if((strcmp("-rcnt", argv[i])==0) && ((i+1)<argc))rank_cnt_file=argv[++i];
    else if((strcmp("-rprob", argv[i])==0) && ((i+1)<argc))rankprobfile=argv[++i];
    else if(strncmp("-", argv[i], 1) == 0) {
      cerr<<argv[0]<<": error invalid parameter="<<argv[i]<<'\n';
      exit(1);
    }
    else {
// keep a pointer to this file name
      if(!(template_flag || divide_template_flag)){
	namelist[numinputfiles]=argv[i];
	numinputfiles++;
      }
      else {
// keep a pointer to this file name but as a template
	namelist[numinputfiles+numtemplates]=argv[i];
	numtemplates++;
      }
    }
  }

// verify input files
  if(numinputfiles < 1 && listfile==NULL) {
    cerr<<argv[0]<<" - no input files specified\n";
    exit(1);
  }
  if(numinputfiles != 0 && listfile != NULL) {
    cerr<<argv[0]<<" - input files not allowed with listfile specified\n";
    exit(1);
  }
  if(numtemplates != 0 && listfile != NULL) {
    cerr<<argv[0]<<" - template files not allowed with listfile specified\n";
    exit(1);
  }
// verify output file name set
  if(avgfile == NULL && stdfile == NULL && cntfile == NULL && minfile == NULL && maxfile == NULL && maxmagfile == NULL &&
     rank_cnt_file == NULL && rankprobfile == NULL) {
    cerr<<argv[0]<<" - no output file(s) specified\n";
    exit(1);
  }
// verify templating and averaging flags
  if(template_flag && divide_template_flag) {
    cerr<<argv[0]<<" - -t option not allowed with -td option\n";
    exit(1);
  }
  // verify norm with template flag
  if(norm_with_template && !divide_template_flag) {
    cerr<<argv[0]<<" - -nt requires -td option\n";
    exit(1);
  }
// verify image to rank with rank count file or prob file
  if(imgtorank == NULL) {
    if(rank_cnt_file != NULL) {
      cerr<<argv[0]<<" - -rcnt option requires -rank imgtorank\n";
      exit(1);
    }
    if(rankprobfile != NULL) {
      cerr<<argv[0]<<" - -rprob option requires -rank imgtorank\n";
      exit(1);
    }
  }
  else if(rank_cnt_file == NULL && rankprobfile == NULL) {
    cerr<<argv[0]<<" - -rank option needs an output file specified with -rcnt and/or -rprob\n";
    exit(1);
  }

// verify min counts
  if(minavgcount < 1) {
    cerr<<argv[0]<<" - -mac min_avg_count must be >= 1\n";
    exit(1);
  }
  if(minstdcount < 2) {
    cerr<<argv[0]<<" - -msc min_std_count must be >= 2\n";
    exit(1);
  }
  if(minstdcount < minavgcount) {
    cerr<<argv[0]<<" - min_std_count must be >= min_avg_count\n";
    exit(1);
  }

// now we need to read the list file before some more verification
  if(listfile != NULL) {
// read input list
    namelist=read_file_list(listfile, &numinputfiles, &numtemplates);
  }

// more verification
  if(divide_template_flag && (numtemplates != numinputfiles)) {
    cerr<<argv[0]<<" - -td option requires equal number of template files to input files\n";
    exit(1);
  }
  if(template_flag && (numtemplates < 1)) {
    cerr<<argv[0]<<" - -t option requires at least one template file\n";
    exit(1);
  }

// build accumulated images
  psytype biggestintype=psynotype;
  psyimg *infileimgptr=NULL;
  psypgbuff infilepgbuffered;
  psyimg *psyimgptr=NULL;
  psyimg *templatefileimgptr=NULL;
  psypgbuff templatepgbuffered;
  psyaccum accumulated_image;
  psyaccum accumulate_templates;
  psyaccum *accum_max_image=NULL;
  psyaccum *accum_min_image=NULL;
  psyaccum *accum_max_mag_image=NULL;
  psyimg *cntimage=NULL;
  templateimg templatedimg;
  psyfileclass infileclass;
  psytype intype;
  scaleimg *normedimgptr=NULL;
  double scalefactors[numinputfiles];

  psyaccum *rank_cnt_img=NULL;
  psyimg *rank_img_ptr=NULL;

  if(imgtorank != NULL) {
    rank_img_ptr=psynewinfile(imgtorank, &infileclass, &intype);
    if(biggestintype == psynotype)biggestintype=intype;
    else if(gettypebytes(biggestintype) < gettypebytes(intype))
      biggestintype=intype;
    if(outfileclass == psynoclass)outfileclass=infileclass;
    rank_img_ptr=new psypgbuff(rank_img_ptr, 1);
    rank_cnt_img=new psyaccum();
  }
  if(minfile != NULL) accum_min_image=new psyaccum();
  if(maxfile != NULL) accum_max_image=new psyaccum();
  if(maxmagfile != NULL) accum_max_mag_image=new psyaccum();

  for(i=0; i<numinputfiles; i++) {
// in file
//    cout<<i+1<<" infile="<<namelist[i];
    infileimgptr=psynewinfile(namelist[i], &infileclass, &intype);
    psyimgptr=infileimgptr;
// buffer the input file because ecat files don't implement getnextpixel
    if(i==0)infilepgbuffered.initpgbuff(psyimgptr, 1);
    else infilepgbuffered.reset(psyimgptr);
    psyimgptr=(psyimg *)&infilepgbuffered;
// note largest type and outfileclass
    if(biggestintype == psynotype)biggestintype=intype;
    else if(gettypebytes(biggestintype) < gettypebytes(intype))
      biggestintype=intype;
    if(outfileclass == psynoclass)outfileclass=infileclass;
//
// template
    templatefileimgptr=NULL;
    normedimgptr=NULL;
    if(i < numtemplates) {
//      cout<<" template="<<namelist[numinputfiles+i];
      templatefileimgptr=psynewinfile(namelist[numinputfiles+i],
				      &infileclass, &intype);
      if(i==0)templatepgbuffered.initpgbuff((psyimg *)templatefileimgptr, 1);
      else templatepgbuffered.reset((psyimg *)templatefileimgptr);
// increment pixels in the template accumulator
      accumulate_templates.incaccum((psyimg *)&templatepgbuffered, psyshort);
// template the current input image
      templatedimg.reset(psyimgptr, (psyimg *)&templatepgbuffered);
      psyimgptr=(psyimg *)&templatedimg;
      if(norm_with_template) {
	double mean;
	templatedimg.gettemplatedstats(NULL, NULL, &mean, NULL);
	scalefactors[i]=1000.0/mean;
// normalize input file
	normedimgptr= new scaleimg(psyimgptr, psydouble,
				   scalefactors[i]);
// set unitary word resolution to maintain normalized values
        normedimgptr->setwordres(1.0);
	psyimgptr=(psyimg *)normedimgptr;
      }
    }
//    cout<<'\n';
// add image to the image accumulator
// save a little work if no average or standard deviation file generated
    if(avgfile != NULL || stdfile != NULL || i==0)
      accumulated_image.add2accum(psyimgptr, psydouble);

    if(minfile != NULL) {
      if(i < numtemplates) {
// set templated areas to values greater then possible minimums
// **note** should work even with normalizing because scaleimg doesn't buffer
// **note** other intermediate processing between templatedimg set up and here may invalidate
	templatedimg.setthresholdvalues(0,max_value_for_type(outtype));
      }
      accum_min_image->minaccum(psyimgptr, psydouble);
    }

    if(maxfile != NULL) {
      if(i < numtemplates) {
// set templated areas to values less then possible maximums
// **note** should work even with normalizing because scaleimg doesn't buffer
// **note** other intermediate processing between templatedimg set up and here may invalidate
	templatedimg.setthresholdvalues(0,min_value_for_type(outtype));
      }
      accum_max_image->maxaccum(psyimgptr, psydouble);
    }

    if(maxmagfile != NULL) {
      if(i < numtemplates) {
// set templated areas to 0 which is less then possible maximum magnitude
// **note** should work even with normalizing because scaleimg doesn't buffer
// **note** other intermediate processing between templatedimg set up and here may invalidate
	templatedimg.setthresholdvalues(0,0);
      }
      accum_max_mag_image->max_mag_accum(psyimgptr, psydouble);
    }

// accumulate number of images greater then or equal to rank image at each voxel
    if(rank_cnt_img != NULL) {
      switch(rankmode) {
      case 0:
	rank_cnt_img->inc_ge_thresholdimg(psyimgptr, rank_img_ptr);
	break;
      case 1:
	rank_cnt_img->inc_mag_ge_thresholdimg(psyimgptr, rank_img_ptr);
	break;
      case 2:
	rank_cnt_img->inc_le_thresholdimg(psyimgptr, rank_img_ptr);
	break;
      }
    }

// we may now close the files
    delete infileimgptr;
    if(templatefileimgptr != NULL)delete templatefileimgptr;
    if(normedimgptr != NULL)delete normedimgptr;
  }// end for(i=0; i<numinputfiles ...
// note output type
  if(outtype==psynotype)outtype=biggestintype;

// set up count image
  templateimg *template_cntimage=NULL;
  psyimgconstant *numinputfilesimage=NULL;

  if(divide_template_flag) {
// all areas combining one or more templates used
    cntimage=(psyimg *)&accumulate_templates;
  }
  else {
// count file based on numinputfiles
    numinputfilesimage=
      new psyimgconstant((psyimg *)&accumulated_image,
			 (double) numinputfiles, psyshort);
    cntimage= (psyimg *) numinputfilesimage;
    if(template_flag) {
// threshold count so only areas combining all templates will be used
      template_cntimage=new templateimg(cntimage,
					(psyimg *)&accumulate_templates,
					1, numtemplates-.5);
      cntimage=(psyimg *)template_cntimage;
    }
  }

  if(avgfile != NULL || stdfile != NULL) {
// now get average
// divide image by number of accumulated template values
// note - it also sets values to zero where no pixels contribute
    divideimgs sumdivided((psyimg *)&accumulated_image, cntimage, 1.0,
			  0.0, minavgcount);
// produces the averaged image
// lets buffer this
    psypgbuff avgbuffered((psyimg *)&sumdivided, 1);
    psyimg *avgimgptr=(psyimg *)&avgbuffered;

// now work on standard deviation
    psyaccum accumvar_image;
    psyimg *stdimgptr=NULL;
// initialize deviation_img at top level allowing later linked usage and
// automatic destruction - note avgimgptr just used for dimensions
    addimgs deviation_img(avgimgptr, avgimgptr, 1.0, -1.0);
    if(stdfile != NULL) {
      psypgbuff *cntimagebuffered=NULL;
      if(template_flag) {
// need to buffer cntimage because it is used to template and in avgimgptr
	cntimagebuffered=new psypgbuff((psyimg *)cntimage, 1);
      }
// accumulate variance
      for(i=0; i<numinputfiles; i++) {
// in file
	infileimgptr=psynewinfile(namelist[i], &infileclass, &intype);
	psyimgptr=infileimgptr;
	infilepgbuffered.reset(psyimgptr);
	psyimgptr=(psyimg *)&infilepgbuffered;

	normedimgptr=NULL;
	if(norm_with_template) {
// normalize input file
	  normedimgptr= new scaleimg(psyimgptr, psydouble,
				     scalefactors[i]);
// set unitary word resolution to maintain normalized values
	  normedimgptr->setwordres(1.0);
	  psyimgptr=(psyimg *)normedimgptr;
	}

// subtract mean image
	deviation_img.initproc2img(psyimgptr, avgimgptr);
	psyimgptr=(psyimg *)&deviation_img;
// template
	templatefileimgptr=NULL;
	if(template_flag) {
// template based on combined templates
	  templatedimg.reset(psyimgptr, (psyimg *)cntimagebuffered);
	  psyimgptr=(psyimg *)&templatedimg;
	}
	else if(i < numtemplates) {
// template based on template for this image only
	  templatefileimgptr=psynewinfile(namelist[numinputfiles+i],
					  &infileclass, &intype);
	  templatepgbuffered.reset((psyimg *)templatefileimgptr);
	  templatedimg.reset(psyimgptr, (psyimg *)&templatepgbuffered);
	  psyimgptr=(psyimg *)&templatedimg;
	}
// accumulate variance
	accumvar_image.addsqr2accum(psyimgptr, psydouble);
// we may now close the files
	delete infileimgptr;
	if(templatefileimgptr != NULL)delete templatefileimgptr;
      }
// convert variance sum to standard deviation
      stdimgptr=(psyimg *)new sqrsum2std((psyimg *)&accumvar_image,
					 cntimage, minstdcount);
      if(cntimagebuffered != NULL)delete cntimagebuffered;
    }


// output results to files

    if(avgfile != NULL) {
// set date and time to current date and time
      avgimgptr->settime();
      avgimgptr->setdate();
// set description for output image
      if(template_flag)
	avgimgptr->setdescription("average - only where templates overlap\n");
      else if(divide_template_flag)
	avgimgptr->setdescription("average - images templated individually\n");
      else avgimgptr->setdescription("average - no templating\n");

      psyimg *outavgimgptr=psynewoutfile(avgfile, avgimgptr,
					 outfileclass, outtype);
// log
      logfile log(avgfile, argc, argv);
// print out images stats
      outavgimgptr->getstats(&min, &max, &mean);
      cout<<"average image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
      delete outavgimgptr;
    }

    if(stdfile != NULL) {
// set date and time to current date and time
      stdimgptr->settime();
      stdimgptr->setdate();
// set description for output image
      if(template_flag)
	stdimgptr->setdescription("standard deviation - only where templates overlap\n");
      else if(divide_template_flag)
	stdimgptr->setdescription("standard deviation - images templated individually");
      else stdimgptr->setdescription("standard deviation - no templating\n");

      psyimg *outstdimgptr=psynewoutfile(stdfile, stdimgptr,
				       outfileclass, outtype);
// log
      logfile log(stdfile, argc, argv);
// print out images stats
      outstdimgptr->getstats(&min, &max, &mean);
      cout<<"standard deviation image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
      delete outstdimgptr;
    }

    if(stdimgptr!=NULL)delete stdimgptr;
  }

  if(rank_cnt_file != NULL) {
    psyimgptr=rank_cnt_img;
    if(template_flag) {
      templatedimg.reset(psyimgptr, (psyimg *)cntimage);
      psyimgptr=(psyimg *)&templatedimg;
    }
// set date and time to current date and time
    psyimgptr->settime();
    psyimgptr->setdate();
// set description for output image
    if(template_flag)
      psyimgptr->setdescription("image rank count - only where templates overlap\n");
    else if(numtemplates > 0)
      psyimgptr->setdescription("image rank count - images templated individually");
    else cntimage->setdescription("image rank count - no templating\n");

    psyimg *outcntimage=psynewoutfile(rank_cnt_file, psyimgptr, outfileclass);
// log
    logfile log(rank_cnt_file, argc, argv);
// print out images stats
    outcntimage->getstats(&min, &max, &mean);
    cout<<"rank count image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outcntimage;
    outcntimage=NULL;
  }

  if(rankprobfile != NULL) {
    divideimgs *rankprobimg=new divideimgs(rank_cnt_img, cntimage, 1.0,0, 1e-16, outtype);
// set date and time to current date and time
    rankprobimg->settime();
    rankprobimg->setdate();
// set description for output image
    if(template_flag)
      rankprobimg->setdescription("image rank prob - only where templates overlap\n");
    else if(numtemplates > 0)
      rankprobimg->setdescription("image rank prob - images templated individually");
    else rankprobimg->setdescription("image rank percnet - no templating\n");

    psyimg *outcntimage=psynewoutfile(rankprobfile, rankprobimg, outfileclass);
// log
    logfile log(rankprobfile, argc, argv);
// print out images stats
    outcntimage->getstats(&min, &max, &mean);
    cout<<"rank prob file min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outcntimage;
    outcntimage=NULL;
  }

  if(cntfile != NULL) {
// set date and time to current date and time
    cntimage->settime();
    cntimage->setdate();
// set description for output image
    if(template_flag)
      cntimage->setdescription("pixel count - only where templates overlap\n");
    else if(numtemplates > 0)
      cntimage->setdescription("pixel count - images templated individually");
    else cntimage->setdescription("pixel count - no templating\n");

    psyimg *outcntimage=psynewoutfile(cntfile, cntimage,
				      outfileclass);
// log
    logfile log(cntfile, argc, argv);
// print out images stats
    outcntimage->getstats(&min, &max, &mean);
    cout<<"count image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outcntimage;
  }

  if(minfile != NULL) {
    psyimgptr=accum_min_image;
    if(template_flag) {
      templatedimg.reset(psyimgptr, (psyimg *)cntimage);
      psyimgptr=(psyimg *)&templatedimg;
    }
// set date and time to current date and time
    psyimgptr->settime();
    psyimgptr->setdate();
// set description for output image
    if(template_flag)
      psyimgptr->setdescription("voxel mins - only where templates overlap\n");
    else if(numtemplates > 0)
      psyimgptr->setdescription("pixel count - images templated individually");
    else psyimgptr->setdescription("pixel count - no templating\n");

    psyimg *outstdimgptr=psynewoutfile(minfile, psyimgptr,
				        outfileclass, outtype);
// log
    logfile log(minfile, argc, argv);
// print out images stats
    outstdimgptr->getstats(&min, &max, &mean);
    cout<<"minimum image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outstdimgptr;
  }

  if(maxfile != NULL) {
    psyimgptr=accum_max_image;
    if(template_flag) {
      templatedimg.reset(psyimgptr, (psyimg *)cntimage);
      psyimgptr=(psyimg *)&templatedimg;
    }

// set date and time to current date and time
    psyimgptr->settime();
    psyimgptr->setdate();
// set description for output image
    if(template_flag)
      psyimgptr->setdescription("voxel maxs - only where templates overlap\n");
    else if(numtemplates > 0)
      psyimgptr->setdescription("pixel count - images templated individually");
    else psyimgptr->setdescription("pixel count - no templating\n");

    psyimg *outstdimgptr=psynewoutfile(maxfile, psyimgptr,
				        outfileclass, outtype);
// log
    logfile log(maxfile, argc, argv);
// print out images stats
    outstdimgptr->getstats(&min, &max, &mean);
    cout<<"maximum image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outstdimgptr;
  }

  if(maxmagfile != NULL) {
    psyimgptr=accum_max_mag_image;
    if(template_flag) {
      templatedimg.reset(psyimgptr, (psyimg *)cntimage);
      psyimgptr=(psyimg *)&templatedimg;
    }

// set date and time to current date and time
    psyimgptr->settime();
    psyimgptr->setdate();
// set description for output image
    if(template_flag)
      psyimgptr->setdescription("voxel magnitude maxs - only where templates overlap\n");
    else if(numtemplates > 0)
      psyimgptr->setdescription("pixel count - images templated individually");
    else psyimgptr->setdescription("pixel count - no templating\n");

    psyimg *outstdimgptr=psynewoutfile(maxmagfile, psyimgptr,
				        outfileclass, outtype);
// log
    logfile log(maxmagfile, argc, argv);
// print out images stats
    outstdimgptr->getstats(&min, &max, &mean);
    cout<<"maximum image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outstdimgptr;
  }

// clean up
  if(namelist != commandlinelist) free_read_file_list(namelist);
  if(numinputfilesimage!=NULL)delete numinputfilesimage;
  if(template_cntimage!=NULL)delete template_cntimage;
  if(accum_max_image!=NULL)delete accum_max_image;
  if(accum_min_image!=NULL)delete accum_min_image;
  if(rank_img_ptr!=NULL)delete rank_img_ptr;
  if(rank_cnt_img!=NULL)delete rank_cnt_img;
}
