#include <float.h>
#include <limits.h>
#include "psyhdr.h"

double scale_factor_for_max_res(double dmin, double dmax, psytype outtype)
{
  double scale_factor;
  double dmaxfabs;
  if(dmax < dmin) {
    double tmp=dmin; dmin=dmax; dmax=tmp;
  }
  if(dmax <= 0){
    dmaxfabs = fabs(dmin); //fabs(dmin) must be >= fabs(dmax)
    dmax=dmin; //all negative numbers - reverse sign for unsigned types
  }
  else {
    dmin = fabs(dmin);
    dmaxfabs = (dmin > dmax)? dmin : dmax;
  }
  if(dmaxfabs < 1e-16)scale_factor=0; //avoid divide by zero
  else {
    switch(outtype) {
    case psychar:
      scale_factor = 127.0/dmaxfabs;
      break;
    case psyuchar:
      scale_factor = 255.0/dmax;
      break;
    case psyshort:
      scale_factor = 32767.0/dmaxfabs;
      break;
    case psyushort:
      scale_factor = 65535.0/dmax;
      break;
    case psyint:
      if(dmaxfabs<65535)scale_factor=65535.0/dmaxfabs; //no real reason
      else scale_factor=1;
      break;
    default:
      scale_factor=1;
    }//end switch
  }//end else
  return(scale_factor);
}

void calc_image_centroid(psyimg *psyimgptr, double *x, double *y,
			 double *z, double *i)
{
  xyzidouble moment;
  xyziint loc;
  double localsum=0;
  double dpixel;
  char_or_largest_pixel pixel;

  xyziint orig=psyimgptr->getorig();
  xyziint end=psyimgptr->getend();
  psytype type=psyimgptr->gettype();

// loop through remaining pixels
  int first_time=1;
  for(loc.i=orig.i; loc.i<=end.i; loc.i++) {
    for(loc.z=orig.z; loc.z<=end.z; loc.z++) {
      for(loc.y=orig.y; loc.y<=end.y; loc.y++) {
	for(loc.x=orig.x; loc.x<=end.x; loc.x++) {
	  if(first_time) {
	    first_time=0;
	    // retrieve and initialize Values using first pixel
	    psyimgptr->initgetpixel(type, loc.x, loc.y, loc.z, loc.i);
	    psyimgptr->getnextpixel(pixel.c);
	    type2double(pixel.c, type, (char *)&dpixel);
	    localsum=dpixel;
	    moment = xyziint2double(loc) * dpixel;
	  }
	  else {
	    psyimgptr->getnextpixel(pixel.c);
	    type2double(pixel.c, type, (char *)&dpixel);
	    localsum += dpixel;
	    moment = moment + (xyziint2double(loc) * dpixel);
	  }
	} // end for(loc.x
      } // end for(loc.y
    } // end for(loc.z
  } // end for(loc.i
  psyimgptr->freegetpixel();
  xyzidouble centroid;
  if(fabs(localsum) > 1e-19)centroid=moment * (1.0/localsum);
  else centroid.x=centroid.y=centroid.z=centroid.i=0.0;
  if(x != NULL)*x=centroid.x;
  if(y != NULL)*y=centroid.y;
  if(z != NULL)*z=centroid.z;
  if(i != NULL)*i=centroid.i;
}

scaleimg::scaleimg(psyimg *psyimgptr, psytype pixeltype,
		   double scale_factor, double translate)
{
  initscaleimg(psyimgptr, pixeltype, scale_factor, translate);
}

void scaleimg::initscaleimg(psyimg *psyimgptr, psytype pixeltype,
			    double scale_factor, double translate)
{
  initproc1img(psyimgptr, pixeltype);
  set_scale_factor(scale_factor);
  translation=translate;
  threshminflag=threshmaxflag=0;
  threshmin=threshminvalue=-DBL_MAX;
  threshmax=threshmaxvalue=DBL_MAX;
}

void scaleimg::set_scale_factor(double scale_factor)
{
  scale=scale_factor;
  if(fabs(scale_factor) < 1e-16)setwordres(0.0);
  else setwordres(inputpsyimg->getwordres()/scale_factor);
}

void scaleimg::set_scale_factor_for_max_res()
{
  double min, max; //, maxfabs;
// scale image by largest absolute value of input image
  if(!threshminflag || !threshmaxflag)inputpsyimg->getstats(&min, &max, NULL);
  if(threshminflag)min=threshmin;
  if(threshmaxflag)max=threshmax;
  min += translation;
  max += translation;
  set_scale_factor(scale_factor_for_max_res(min, max, gettype()));
}

void scaleimg::set_min_thresh(double min_thresh, double min_thresh_value=0)
{
  threshminflag=1;
  threshmin=min_thresh;
  threshminvalue=min_thresh_value;
}

void scaleimg::set_max_thresh(double max_thresh, double max_thresh_value)
{
  threshmaxflag=1;
  threshmax=max_thresh;
  threshmaxvalue=max_thresh_value;
}

void scaleimg::convertpixel(char *in, psytype intype, char *out,
			    psytype outtype)
{
  double dpixel;
  type2double(in, intype, (char *)&dpixel);
  if(threshminflag && (dpixel < threshmin))dpixel=threshminvalue;
  else if(threshmaxflag && (dpixel > threshmax))dpixel=threshmaxvalue;
  else dpixel = (dpixel+translation)*scale;
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

addimgs::addimgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
		 double scalefactor1, double scalefactor2,
		 psytype pixeltype) :
       proc2img(psy1imgptr, psy2imgptr, pixeltype)
{
  setvalues(scalefactor1, scalefactor2);
}

void addimgs::setvalues(double scalefactor1, double scalefactor2)
{
  addimgs::scalefactor1=scalefactor1;
  addimgs::scalefactor2=scalefactor2;
}

void addimgs::proc2pixels(char *in1, psytype in1type,
			  char *in2, psytype in2type,
			  char *out, psytype outtype)
{
  double dpixel, dpixel1, dpixel2;
// convert to double to simplify math
  type2double(in1, in1type, (char *)&dpixel1);
  type2double(in2, in2type, (char *)&dpixel2);
// do actual processing - multiply and add
  dpixel = dpixel1*scalefactor1 + dpixel2*scalefactor2;
// convert back to desired output type
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

multiplyimgs::multiplyimgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
			   double factor, psytype pixeltype) :
       proc2img(psy1imgptr, psy2imgptr, pixeltype)
{
  setvalues(factor);
}

void multiplyimgs::setvalues(double factor)
{
  multiplyimgs::factor=factor;
}

void multiplyimgs::proc2pixels(char *in1, psytype in1type,
			  char *in2, psytype in2type,
			  char *out, psytype outtype)
{
  double dpixel, dpixel1, dpixel2;
// convert to double to simplify math
  type2double(in1, in1type, (char *)&dpixel1);
  type2double(in2, in2type, (char *)&dpixel2);
// do actual processing - multiply
  dpixel = factor*dpixel1*dpixel2;
// convert back to desired output type
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

divideimgs::divideimgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
		       double factor,
		       double below_minimum_default,
		       double minimum_divisor_magnitude,
		       psytype pixeltype) :
       proc2img(psy1imgptr, psy2imgptr, pixeltype)
{
  setvalues(factor, below_minimum_default,
	    minimum_divisor_magnitude);
}

void divideimgs::setvalues(double factor,
			   double below_minimum_default,
			   double minimum_divisor_magnitude)
{
  divideimgs::factor=factor;
  divideimgs::minimum_divisor_magnitude=minimum_divisor_magnitude;
  divideimgs::below_minimum_default=below_minimum_default;
}

void divideimgs::proc2pixels(char *in1, psytype in1type,
			  char *in2, psytype in2type,
			  char *out, psytype outtype)
{
  double dpixel, dpixel1, dpixel2;
// convert to double to simplify math
  type2double(in1, in1type, (char *)&dpixel1);
  type2double(in2, in2type, (char *)&dpixel2);
// do actual processing - divide
  if(fabs(dpixel2) < minimum_divisor_magnitude)dpixel=below_minimum_default;
  else dpixel = factor*dpixel1/dpixel2;
// convert back to desired output type
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

accumulateimgs::accumulateimgs(psytype pixeltype) : psypgbuff()
{
  accumulatetype=pixeltype; 
  accumulatedpsyimgptr=NULL;
  first_factor=1.0;
}

accumulateimgs::accumulateimgs(psyimg *psyimgptr, double factor,
			       psytype pixeltype)
{
  accumulatetype=pixeltype;
  accumulatedpsyimgptr=NULL;
  first_factor=1.0;
  initpgbuff(psyimgptr, 1,  accumulatetype);
  addimg(psyimgptr, factor);
}

accumulateimgs::~accumulateimgs()
{
  addimgs *tmpaddimgptr;

  while((accumulatedpsyimgptr!=NULL) && (accumulatedpsyimgptr!=inputpsyimg))
    {
      tmpaddimgptr=(addimgs *)accumulatedpsyimgptr;
      accumulatedpsyimgptr=tmpaddimgptr->getlink1();
      delete tmpaddimgptr;
    }
}

void accumulateimgs::addimg(psyimg *psyimgptr, double factor)
{
  if(accumulatedpsyimgptr==NULL) {
    if(inputpsyimg == NULL)initpgbuff(psyimgptr, 1, accumulatetype);
    else if(inputpsyimg != psyimgptr) {
      cerr<<"accumulateimgs::addimg - initialization error\n";
      exit(1);
    }
    accumulatedpsyimgptr=psyimgptr;
    first_factor=factor;
  }
  else if(accumulatedpsyimgptr == inputpsyimg) {
    accumulatedpsyimgptr=(psyimg*)new addimgs(accumulatedpsyimgptr, psyimgptr,
					      first_factor,factor,
					      accumulatetype);
  }
  else {
    accumulatedpsyimgptr=(psyimg*)new addimgs(accumulatedpsyimgptr, psyimgptr,
					      1.0,factor,accumulatetype);
  }
// reset page buffer
  reset();
}

void accumulateimgs::setscale(double scale)
{
  addimgs *tmpaddimgptr;
  psyimg *tmppsyimgptr;
  int scaleset=0;

  tmppsyimgptr=accumulatedpsyimgptr;
  while(tmppsyimgptr != NULL && tmppsyimgptr != inputpsyimg) {
    tmpaddimgptr=(addimgs *)tmppsyimgptr;
    tmppsyimgptr=tmpaddimgptr->getlink1();
    if(tmppsyimgptr == inputpsyimg) tmpaddimgptr->setvalues(scale,scale);
    else tmpaddimgptr->setvalues(1.0,scale);
    scaleset=1;
  }
  if(!scaleset && scale != 1.0) {
    cerr<<"accumulateimgs::setscale - can't scale single or no image\n";
    exit(1);
  }
  reset();
}

void accumulateimgs::fillpage(char *buff, int z, int i)
{
  if(accumulatedpsyimgptr == NULL) {
    cerr<<"accumulatedimgs::fillpage - not initialized\n";
    exit(1);
  }
  else if(accumulatedpsyimgptr == inputpsyimg) {
// only one input image act like a page buffer
    if(first_factor != 1.0) {
      cerr<<"accumulateimgs::fillpage - can't handle scaling single image\n";
      exit(1);
    }
    psypgbuff::fillpage(buff, z, i);
  }
  else {
    ((addimgs *)accumulatedpsyimgptr)->copyblock(buff,
						 orig.x, orig.y, z, i,
						 end.x, end.y, z, i,
						 inc.x, inc.y, inc.z, inc.i,
						 type);
  }
}

psyshiftimg::psyshiftimg(psyimg *psyimgptr, psydims shift)
     : psypgbuff(psyimgptr)
{
  psyshiftimg::shift=shift;
  // fix spatial transforms
  xyzidouble changeinvoxorig;
  changeinvoxorig.x = -shift.x; changeinvoxorig.y = -shift.y; changeinvoxorig.z = -shift.z;
  changeinvoxorig.i = 1.0l;
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * changeinvoxorig;
    matrix4X4 newmatrix = oldmatrix;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform(new threeDtransform(newmatrix), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * changeinvoxorig;
    matrix4X4 newmatrix = oldmatrix;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform2(new threeDtransform(newmatrix), getspatialtransformcode2());
  }
}

void psyshiftimg::fillpage(char *buff, int z, int i)
{
  char *buffptrx, *buffptry;
  int x, y;
  int xf, xp, yp, zp, ip;
// calculate current plane with shifted z and i locations
// x, y, z, i correspond to the shifted locations in the output image
// xp, yp, zp, ip correspond location in the input image
  ip = i - shift.i;
  while(ip < orig.i)ip+=size.i;
  while(ip > end.i)ip-=size.i;
  zp = z - shift.z;
  while(zp < orig.z)zp+=size.z;
  while(zp > end.z)zp-=size.z;
// now fill page looping thru input image to use sequential gets
  y = orig.y + shift.y;
  while(y < orig.y)y+=size.y;
  while(y > end.y)y-=size.y;
  xf = orig.x + shift.x;
  while(xf < orig.x)xf+=size.x;
  while(xf > end.x)xf-=size.x;
  inputpsyimg->initgetpixel(type, orig.x, orig.y, zp, ip);
  buffptry=buff + ((y-orig.y) * inc.y);
  for(yp=orig.y; yp<=end.y; y++, yp++, buffptry+=inc.y){
    if(y > end.y) {
      y -= size.y;
      buffptry -= inc.z;
    }
    x=xf;
    buffptrx=buffptry + ((x-orig.x) * inc.x);
    for(xp=orig.x; xp<=end.x; x++, xp++, buffptrx+=inc.x){
      if(x > end.x){
	x -= size.x;
	buffptrx -= inc.y;
      }
      inputpsyimg->getnextpixel(buffptrx);
    }
  }
  inputpsyimg->freegetpixel();
  return;
}

padimage::padimage(psyimg *psyimgptr,
		   xyziint prepad, xyziint postpad,
		   double padvalue)
{
  xyziint dim;
  xyziint orig;
  int skippixels=0;
  xyzidouble res;
  double wres;
  int maxnumpages=1;

  padimage::padvalue=padvalue;
// get values from input image
  psyimgptr->getsize(&dim.x, &dim.y, &dim.z, &dim.i);
  psyimgptr->getres(&res.x, &res.y, &res.z, &res.i);
  wres=psyimgptr->getwordres();
// the original image will maintain it's origin and end
  image_beg=psyimgptr->getorig();
  image_end=psyimgptr->getend();
// then padded image may have a negative origin
  orig = image_beg - prepad;
  dim = prepad + postpad + dim;
  initpgbuff(psyimgptr,
	     dim.x, dim.y, dim.z, dim.i, psynotype,
	     orig.x, orig.y, orig.z, orig.i, skippixels,
	     res.x, res.y, res.z, res.i, wres,
	     maxnumpages);
  // fix spatial transforms
  xyzidouble changeinvoxorig;
  changeinvoxorig.x = -prepad.x; changeinvoxorig.y = -prepad.y; changeinvoxorig.z = -prepad.z;
  changeinvoxorig.i = 1.0l;
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * changeinvoxorig;
    matrix4X4 newmatrix = oldmatrix;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform(new threeDtransform(newmatrix), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * changeinvoxorig;
    matrix4X4 newmatrix = oldmatrix;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform2(new threeDtransform(newmatrix), getspatialtransformcode2());
  }
}

void padimage::fillpage(char *buff, int z, int i)
{
  char *buff_ptr, *buff_end;
  char_or_largest_pixel tmpvalue;
  int bytesize;
  char *tmpptr, *tmpbuffptr;
  int k;
// for simplicity fill whole buffer with pad value
  pixeltypechg((char *)&padvalue, psydouble, tmpvalue.c, type);
  bytesize = gettypebytes(type);
  buff_end = buff + offset(end.x, end.y, z, i) - offset(orig.x, orig.y, z, i);
  for(buff_ptr=buff; buff_ptr < buff_end; buff_ptr += inc.x)
    for(k=0, tmpptr=tmpvalue.c, tmpbuffptr=buff_ptr;
	k<bytesize; k++) *tmpbuffptr++ = *tmpptr++;
// now fill in image
  if(z >= image_beg.z && z <= image_end.z &&
     i >= image_beg.i && i <= image_end.i) {
    buff_ptr=buff + offset(image_beg.x, image_beg.y, z, i) -
      offset(orig.x, orig.y, z, i);
    inputpsyimg->copyblock(buff_ptr, image_beg.x, image_beg.y, z, i,
			   image_end.x, image_end.y, z, i,
			   inc.x, inc.y, inc.z, inc.i,
			   type);
  }
  return;
}

concatimgs::concatimgs() : psy2imglnk()
{
}

concatimgs::concatimgs(psyimg *psy1imgptr, psyimg *psy2imgptr, int catdim)
{
  dim_res_set = 0;
  initconcatimgs(psy1imgptr, psy2imgptr, catdim);
}

concatimgs::concatimgs(psyimg *psy1imgptr, psyimg *psy2imgptr, int catdim, double dimres)
{
  initconcatimgs(psy1imgptr, psy2imgptr, catdim, dimres);
}

void concatimgs::initconcatimgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
				int catdim, double dimres)
{
  dim_res_set = 1;
  dim_res = dimres;
  initconcatimgs(psy1imgptr, psy2imgptr, catdim);
}

void concatimgs::initconcatimgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
				int catdim)
{
  psydims dim1, dim2, newdim;
  psyres newres;
  //  char *ptr, *ptr1, *ptr2;
  if(psy1imgptr == NULL || psy2imgptr == NULL) {
    output_tree(&cerr);
    cerr<<":concatimgs::initconcatimgs - error null psyimgptr\n";
    exit(1);
  }
  in1psyimg=psy1imgptr;
  in2psyimg=psy2imgptr;
// make sure input images have correct word sizes
  if(in1psyimg->gettype() != in2psyimg->gettype()) {
    output_tree(&cerr);
    cerr<<":concatimgs::initconcatimgs - images must have the same pixel type\n";
    exit(1);
  }
// get input image sizes
  dim1=in1psyimg->getsize();
  dim2=in2psyimg->getsize();

// get input image resolution
  newres=in1psyimg->getres();

  concatimgs::catdim=catdim;
  switch(catdim) {
  case 0:
    // make sure input images have correct sizes
    if(dim1.y != dim2.y || dim1.z != dim2.z || dim1.i != dim2.i) {
      output_tree(&cerr);
      cerr<<":concatimgs::initconcatimgs - images must have same y, z & i dim\n";
      exit(1);
    }
    // new dim combines z planes
    first_img_2_size=dim1.x;
    newdim=dim1; newdim.x += dim2.x;
    if(dim_res_set) newres.x = dim_res;
    break;
  case 1:
    // make sure input images have correct sizes
    if(dim1.x != dim2.x || dim1.z != dim2.z || dim1.i != dim2.i) {
      output_tree(&cerr);
      cerr<<":concatimgs::initconcatimgs - images must have same x, z & i dim\n";
      exit(1);
    }
    // new dim combines z planes
    first_img_2_size=dim1.y;
    newdim=dim1; newdim.y += dim2.y;
    if(dim_res_set) newres.y = dim_res;
    break;
  case 2:
  default:
    concatimgs::catdim=2;
    // make sure input images have correct sizes
    if(dim1.x != dim2.x || dim1.y != dim2.y || dim1.i != dim2.i) {
      output_tree(&cerr);
      cerr<<":concatimgs::initconcatimgs - images must have same x, y & i dim\n";
      exit(1);
    }
    // new dim combines z planes
    first_img_2_size=dim1.z;
    newdim=dim1; newdim.z += dim2.z;
    if(dim_res_set) newres.z = dim_res;
    break;
  case 3:
    // make sure input images have correct sizes
    if(dim1.x != dim2.x || dim1.y != dim2.y || dim1.z != dim2.z) {
      output_tree(&cerr);
      cerr<<":concatimgs::initconcatimgs - images must have same x, y & z dim\n";
      exit(1);
    }
    // new dim combines z planes
    first_img_2_size=dim1.i;
    newdim=dim1; newdim.i += dim2.i;
    if(dim_res_set) newres.i = dim_res;
    break;
  }
// let initialize psy2imglnk do remaining set up
  initpsy2imglnk(in1psyimg, in2psyimg, in1psyimg->gettype(),
		 &newdim, NULL, &newres);
// avoid concatenating too many descriptions
  setdescription("concatimgs");
}

void concatimgs::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			  int iorig, int xend, int yend, int zend, int iend,
			  int out_xinc, int out_yinc,
			  int out_zinc, int out_iinc,
			  psytype pixeltype)
{
  int y, i, z;
  char *xoptr, *yoptr, *zoptr, *ioptr;
  switch(catdim) {
  case 0:
    ioptr = outbuff;
    for(i=iorig; i<=iend; i++, ioptr += out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr += out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr += out_yinc) {
	  xoptr = yoptr;
	  if(xorig < first_img_2_size) {
	    if(xend < first_img_2_size) {
	      in1psyimg->copyblock(xoptr, xorig, y, z, i,
				   xend, y, z, i,
				   out_xinc, out_yinc, out_zinc, out_iinc,
				   pixeltype);
	    }
	    else {
	      in1psyimg->copyblock(xoptr, xorig, y, z, i,
				   first_img_2_size - 1, y, z, i,
				   out_xinc, out_yinc, out_zinc, out_iinc,
				   pixeltype);
	      xoptr += (first_img_2_size - xorig) * out_xinc;
	      in1psyimg->copyblock(xoptr, 0, y, z, i,
				   xend - first_img_2_size, y, z, i,
				   out_xinc, out_yinc, out_zinc, out_iinc,
				   pixeltype);
	    }
	  }
	  else {
	    in2psyimg->copyblock(xoptr, xorig-first_img_2_size, y, z, i,
				 xend-first_img_2_size, y, z, i,
				 out_xinc, out_yinc, out_zinc, out_iinc,
				 pixeltype);
	  }
	} // end for(y
      } // end for(z
    } // end for(i
    break;
  case 1:
    ioptr = outbuff;
    for(i=iorig; i<=iend; i++, ioptr += out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr += out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr += out_yinc) {
	  if(y<first_img_2_size) {
	    in1psyimg->copyblock(yoptr, xorig, y, z, i,
				 xend, y, z, i,
				 out_xinc, out_yinc, out_zinc, out_iinc,
				 pixeltype);
	  }
	  else {
	    in2psyimg->copyblock(yoptr, xorig, y-first_img_2_size, z, i,
				 xend, y-first_img_2_size, z, i,
				 out_xinc, out_yinc, out_zinc, out_iinc,
				 pixeltype);
	  } // end else
	} // end for(y
      } // end for(z
    } // end for(i
    break;
  case 2:
    ioptr = outbuff;
    for(i=iorig; i<=iend; i++, ioptr += out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr += out_zinc) {
	if(z<first_img_2_size) {
	  in1psyimg->copyblock(zoptr, xorig, yorig, z, i,
			       xend, yend, z, i,
			       out_xinc, out_yinc, out_zinc, out_iinc,
			       pixeltype);
	}
	else {
	  in2psyimg->copyblock(zoptr, xorig, yorig, z-first_img_2_size, i,
			       xend, yend, z-first_img_2_size, i,
			       out_xinc, out_yinc, out_zinc, out_iinc,
			       pixeltype);
	} // end else
      } // end for(z
    } // end for(i
    break;
  case 3:
    ioptr = outbuff;
    for(i=iorig; i<=iend; i++, ioptr += out_iinc) {
      if(i<first_img_2_size) {
	in1psyimg->copyblock(ioptr, xorig, yorig, zorig, i,
			     xend, yend, zend, i,
			     out_xinc, out_yinc, out_zinc, out_iinc,
			     pixeltype);
      }
      else {
	in2psyimg->copyblock(ioptr, xorig, yorig, zorig, i-first_img_2_size,
			     xend, yend, zend, i-first_img_2_size,
			     out_xinc, out_yinc, out_zinc, out_iinc,
			     pixeltype);
      } // end else
    } // end for(i
    break;
  }
}

void concatimgs::getpixel(char *pixel, psytype pixeltype,
			 int x, int y, int z, int i)
{
  switch(catdim) {
  case 0:
    if(x < first_img_2_size) in1psyimg->getpixel(pixel, pixeltype, x, y, z, i);
    else in2psyimg->getpixel(pixel, pixeltype, x-first_img_2_size, y, z, i);
    break;
  case 1:
    if(y < first_img_2_size) in1psyimg->getpixel(pixel, pixeltype, x, y, z, i);
    else in2psyimg->getpixel(pixel, pixeltype, x, y-first_img_2_size, z, i);
    break;
  case 2:
    if(z < first_img_2_size) in1psyimg->getpixel(pixel, pixeltype, x, y, z, i);
    else in2psyimg->getpixel(pixel, pixeltype, x, y, z-first_img_2_size, i);
    break;
  case 3:
    if(i < first_img_2_size) in1psyimg->getpixel(pixel, pixeltype, x, y, z, i);
    else in2psyimg->getpixel(pixel, pixeltype, x, y, z, i-first_img_2_size);
    break;
  }
}

void concatimgs::getnextpixel(char *pixel)
{
  output_tree(&cerr);
  cerr<<":concatimgs::getnextpixel - sorry, not implimented\n";
  exit(1);
}

void concatimgs::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  output_tree(&cerr);
  cerr<<":concatimgs::initgetpixel - sorry, not implimented\n";
  exit(1);
}

psyhistogram::psyhistogram(psyimg *psyimgptr,
			   double min, double max, int numberbins)
: psyfullbuff()
{
  initpsyhistogram(psyimgptr, min, max, numberbins);
}

void psyhistogram::initpsyhistogram(psyimg *psyimgptr,
				    double min, double max, int numberbins)
{
  if(numberbins <= 0) numberbins = 256;
  histmin = min;
  histmax = max;
  double xres = (histmax - histmin) / (numberbins - 1);
  initpsyimglnk(psyimgptr, numberbins, 1, 1, 1, psyint,
		0, 0, 0, 0, 0, xres);
}

void psyhistogram::chknfillbuff() {
  if(buffimage.getbuff() == NULL) {
// initialize buffer to full histogram size
    buffimage.initbuff(size.x, size.y, size.z, size.i, psyint);
// initialize histogram
    int *histptr = (int *) buffimage.getbuff();
    int numhistwords = size.x * size.y * size.z * size.i;
    for(int i=0; i<numhistwords; i++) histptr[i] = 0;
    lessercnt = 0; greatercnt = 0;
// initialize input pixel fetching
    psydims insize = inputpsyimg->getsize();
    int numwords=insize.x * insize.y * insize.z * insize.i;
    psydims inorig = inputpsyimg->getorig();
    psytype intype = inputpsyimg->gettype();
    inputpsyimg->initgetpixel(intype, inorig.x, inorig.y, inorig.z, inorig.i);
// loop through all input pixels
    for(int i=0; i<numwords; i++) {
      char_or_largest_pixel pixel;
      inputpsyimg->getnextpixel(pixel.c);
      double dpixel;
      type2double(pixel.c, intype, (char *)&dpixel);
      int bin = (int) ((dpixel - histmin) / res.x);
      if(bin < 0) lessercnt++;
      else if(bin >= size.x) greatercnt++;
      else histptr[bin]++;
    }
    inputpsyimg->freegetpixel();
  }
}
