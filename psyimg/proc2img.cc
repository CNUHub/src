#include "psyhdr.h"

string combine_comments(string s1, string s2)
{
  string combinedstr;
  if(s2.length() == 0) return s1;
  else if(s1.length() == 0) return s2;
  else if(s1.compare(s2) == 0) return s1;
  else {
// combine them
    return s1 + " & " + s2;
  }
}

psy2imglnk::psy2imglnk() : psyimg()
{
  in1psyimg=NULL;
  in2psyimg=NULL;
}

psy2imglnk::psy2imglnk(psyimg *psy1imgptr, psyimg *psy2imgptr)
{
  initpsy2imglnk(psy1imgptr, psy2imgptr);
}

void psy2imglnk::initpsy2imglnk(psyimg *psy1imgptr, psyimg *psy2imgptr,
				psytype pixeltype,
				psydims *indims, xyziint *inorig,
				psyres *inres)
{
  psydims dims;
  xyziint orig;
  psyres res;

  if(psy1imgptr == NULL || psy2imgptr == NULL) {
    output_tree(&cerr);
    cerr<<":psy2imglnk::initpsy2imglnk - error null psyimgptr\n";
    exit(1);
  }
  in1psyimg=psy1imgptr;
  in2psyimg=psy2imgptr;
  if(inorig == NULL)orig=in1psyimg->getorig();
  else orig= *inorig;
  if(inres == NULL)res=in1psyimg->getres();
  else res= *inres;
  if(indims != NULL)dims= *indims;
  else {
// get input image sizes
    dims=in1psyimg->getsize();
// make sure input images are the same size
    if(!psy2imgptr->samedim(dims.x, dims.y, dims.z, dims.i)) {
      output_tree(&cerr);
      cerr<<":psy2imglnk::initpsy2imglnk - images must be the same dimensions\n";
      exit(1);
    }
  }
// use requested output pixel type or default to input 1 image type
  if(pixeltype==psynotype)pixeltype=in1psyimg->gettype();
// set extents to same as input images
  initpsyimg(dims.x,dims.y,dims.z,dims.i,pixeltype,orig.x,orig.y,orig.z,orig.i,
	     0,res.x,res.y,res.z,res.i,in1psyimg->getwordres());
// set date and time from first input image
  setdate(in1psyimg->getdate());
  settime(in1psyimg->gettime());
// set orientation form first input image
  setorient(in1psyimg->getorient());
// combine inputs to set patientid
  setpatientid(combine_comments(in1psyimg->getpatientid(),
				in2psyimg->getpatientid()));
// combine inputs to set description
  setdescription(combine_comments(in1psyimg->getdescription(),
				  in2psyimg->getdescription()));
}

psyimg *psy2imglnk::getlink1()
{
  return(in1psyimg);
}

psyimg *psy2imglnk::getlink2()
{
  return(in2psyimg);
}

proc2img::proc2img(psyimg *psy1imgptr, psyimg *psy2imgptr,
		   psytype pixeltype) : psy2imglnk()
{
  initproc2img(psy1imgptr, psy2imgptr, pixeltype);
}

void proc2img::initproc2img(psyimg *psy1imgptr, psyimg *psy2imgptr,
		    psytype pixeltype)
{
  initpsy2imglnk(psy1imgptr, psy2imgptr, pixeltype);
}

void proc2img::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			 int iorig, int xend, int yend, int zend, int iend,
			 int out_xinc, int out_yinc,
			 int out_zinc, int out_iinc,
			 psytype pixeltype)
{
  char_or_largest_pixel in1pixel, in2pixel;// insure size and allignment
                                           // correct for largest pixel
  int x, y, z, i;
  char *xoptr, *yoptr, *zoptr, *ioptr;
  psytype in1type=in1psyimg->gettype();
  psytype in2type=in2psyimg->gettype();
// select most efficient loop to use
  ioptr=outbuff;
  if(xorig != orig.x || xend != end.x) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  in1psyimg->initgetpixel(in1type, xorig, y, z, i);
	  in2psyimg->initgetpixel(in2type, xorig, y, z, i);
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
            in1psyimg->getnextpixel(in1pixel.c);
            in2psyimg->getnextpixel(in2pixel.c);
            proc2pixels(in1pixel.c, in1type, in2pixel.c, in2type,
			xoptr, pixeltype);
	  }//end for(x=
	  in1psyimg->freegetpixel();
	  in2psyimg->freegetpixel();
        }//end for(y=
      }//end for(z=
    }//end for(i=
  } else if(yorig != orig.y || yend != end.y) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	in1psyimg->initgetpixel(in1type, xorig, yorig, z, i);
	in2psyimg->initgetpixel(in2type, xorig, yorig, z, i);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
            in1psyimg->getnextpixel(in1pixel.c);
            in2psyimg->getnextpixel(in2pixel.c);
            proc2pixels(in1pixel.c, in1type, in2pixel.c, in2type,
			xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
	in1psyimg->freegetpixel();
	in2psyimg->freegetpixel();
      }//end for(z=
    }//end for(i=
  } else if(zorig != orig.z || zend != end.z) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      in1psyimg->initgetpixel(in1type, xorig, yorig, zorig, i);
      in2psyimg->initgetpixel(in2type, xorig, yorig, zorig, i);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
            in1psyimg->getnextpixel(in1pixel.c);
            in2psyimg->getnextpixel(in2pixel.c);
            proc2pixels(in1pixel.c, in1type, in2pixel.c, in2type,
			xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
      in1psyimg->freegetpixel();
      in2psyimg->freegetpixel();
    }//end for(i=
  } else {
    in1psyimg->initgetpixel(in1type, xorig, yorig, zorig, iorig);
    in2psyimg->initgetpixel(in2type, xorig, yorig, zorig, iorig);
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
            in1psyimg->getnextpixel(in1pixel.c);
            in2psyimg->getnextpixel(in2pixel.c);
            proc2pixels(in1pixel.c, in1type, in2pixel.c, in2type,
			xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
    }//end for(i=
    in1psyimg->freegetpixel();
    in2psyimg->freegetpixel();
  }
}

void proc2img::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
// sequential get pixel will use the same type
  type=pixeltype;
  in1psyimg->initgetpixel(in1psyimg->gettype(), x, y, z, i);
  in2psyimg->initgetpixel(in2psyimg->gettype(), x, y, z, i);
}

void proc2img::freegetpixel(){
  in1psyimg->freegetpixel();
  in2psyimg->freegetpixel();
};

void proc2img::getpixel(char *outpixel, psytype pixeltype,
			int x, int y, int z, int i)
{
  char_or_largest_pixel in1pixel, in2pixel;// insure size and allignment
                                           // correct for largest pixel
// sequential get pixel will use the same type
  type=pixeltype;
  in1psyimg->getpixel(in1pixel.c, in1psyimg->gettype(), x, y, z, i);
  in2psyimg->getpixel(in2pixel.c, in2psyimg->gettype(), x, y, z, i);
  proc2pixels(in1pixel.c, in1psyimg->gettype(),
	      in2pixel.c, in2psyimg->gettype(),
	      outpixel, pixeltype);
}

void proc2img::getnextpixel(char *outpixel)
{
  char_or_largest_pixel in1pixel, in2pixel;// insure size and allignment
                                           // correct for largest pixel
  in1psyimg->getnextpixel(in1pixel.c);
  in2psyimg->getnextpixel(in2pixel.c);
  proc2pixels(in1pixel.c, in1psyimg->gettype(),
	      in2pixel.c, in2psyimg->gettype(),
	      outpixel, type);
}
