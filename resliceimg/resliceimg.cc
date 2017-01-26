#include "psyhdr.h"
#include <iomanip>
#define SMALLEST_RES 1e-20
#define SMALLEST_SCALE 1e-16
const double DEG_TO_RAD = (M_PI/180.0);
const double RAD_TO_DEG = (180.0/M_PI);

void CHKCPTR(char *cptr, char *list, char *endlist)
{
  if(cptr < endlist)return;
  cerr<<"error parsing transform string=\""<<list<<"\"\n";
  exit(1);
}

double GETNUMBER(char **cptr, char *list, char *endlist)
{
  double result=atof(*cptr);
  for(; *cptr<endlist; (*cptr)++)if(**cptr=='X')break;
  CHKCPTR(*cptr, list, endlist);
  return(result);
}

matrix4X4 parse_transform(char *list, xyzdouble res, int inverse=0)
{

// parse the transform string as a list of single degree options
// to be applied in order from right to left the same order
// a matrix is built
//
// the options are built with the following characters:
//   X is a separator and must occur and the beginning and end of the string
//   D designates rotation in degrees
//   R designates rotation in radians
//   T designates translation in meters
//   P designates translation in pixels
//   S designates a scale factor
//   x, y, and z designate the perspective axis
//
// Example:
//    XRx.3839724XDz-15XPy-1.4XTz0.003X
// is a translation of 0.003 meters along the z axis
// followed by a translation of -1.4 pixels along the y axis
// followed by a rotation of -15 degrees around the z-axis
// followed by a rotation of .3839724 radians around the x axis

  matrix4X4 Result(Identity);
  if(list == NULL)return(Result);
  int sign=1;
  if(inverse)sign=-1;

  char mtitle[128];

  matrix4X4 Local(Identity);
  matrix4X4 tmp;
  char *cptr=list;
  char *endlist=cptr + strlen(list);
  while(cptr < endlist){
    double angle_factor=1.0;
    int pixel_units=0;
    int cnt=strcspn(cptr, "X");
    cnt=(cnt < 128)? cnt : 127;
    strncpy(mtitle, cptr, cnt);
    mtitle[cnt]='\0';
    double angle;
    char axis;
    double translation;
    double scale;
    switch(*cptr) {
    case 'X':
      break;
    case 'D':
      angle_factor=DEG_TO_RAD;
    case 'R':
      cptr++; CHKCPTR(cptr, list, endlist);
      axis= *cptr;
      cptr++; CHKCPTR(cptr, list, endlist);
      angle=sign*GETNUMBER(&cptr, list, endlist) * angle_factor;
      Local.reset(Identity);
      switch(axis) {
      case 'x':
	Local.m.m22= cos(angle); Local.m.m23= sin(angle);
	Local.m.m32= -Local.m.m23;   Local.m.m33= Local.m.m22;
	break;
      case 'y':
	Local.m.m11= cos(angle); Local.m.m13= sin(angle);
	Local.m.m31= -Local.m.m13;   Local.m.m33= Local.m.m11;
	break;
      case 'z':
	Local.m.m11= cos(angle); Local.m.m12= -sin(angle);
	Local.m.m21= -Local.m.m12;   Local.m.m22= Local.m.m11;
	break;
      default:
	CHKCPTR(endlist, list, endlist);
	break;
      }
      dispmatrix4X4(mtitle, &Local);
      if(inverse) tmp = Local * Result; // tmp used in case changes occur inplace on Result
      else tmp = Result * Local;
      Result.reset(&tmp);
      break;
    case 'P':
      pixel_units=1;
    case 'T':
      cptr++; CHKCPTR(cptr, list, endlist);
      axis= *cptr;
      cptr++; CHKCPTR(cptr, list, endlist);
      translation=sign*GETNUMBER(&cptr, list, endlist);
      Local.reset(Identity);
      switch(axis) {
      case 'x':
	if(pixel_units)translation *= res.x;
	Local.m.m14= translation;
	break;
      case 'y':
	if(pixel_units)translation *= res.y;
	Local.m.m24= translation;
	break;
      case 'z':
	if(pixel_units)translation *= res.z;
	Local.m.m34= translation;
	break;
      default:
	CHKCPTR(endlist, list, endlist);
	break;
      }
      dispmatrix4X4(mtitle, &Local);
      if(inverse) tmp = Local * Result ;
      else tmp = Result * Local;
      Result.reset(&tmp);
      break;
    case 'S':
      cptr++; CHKCPTR(cptr, list, endlist);
      axis= *cptr;
      cptr++; CHKCPTR(cptr, list, endlist);
      scale=GETNUMBER(&cptr, list, endlist);
      if(inverse) scale = 1.0/scale;
      Local.reset(Identity);
      switch(axis) {
      case 'x':
	Local.m.m11= scale;
	break;
      case 'y':
	Local.m.m22= scale;
	break;
      case 'z':
	Local.m.m33= scale;
	break;
      default:
	CHKCPTR(endlist, list, endlist);
	break;
      }
      dispmatrix4X4(mtitle, &Local);
      if(inverse) tmp = Local * Result;
      else tmp = Result * Local;
      Result.reset(&tmp);
      break;
    default:
      CHKCPTR(endlist, list, endlist);
      break;
    }
    cptr++;
  }
  return(Result);
}

class psyresample : public  psyimglnkpxl {
 protected:
  xyzidouble xinc_rel2in;
  xyzidouble yinc_rel2in;
  xyzidouble zinc_rel2in;
  xyzidouble iinc_rel2in;
  xyzidouble orig_rel2in;
  xyzidouble loc_rel2in;
  xyzidouble yloc_rel2in;
  xyzidouble zloc_rel2in;
  xyzidouble iloc_rel2in;
  psydims inorig;
  psydims inend;

 public:
  psyresample(psyimg *psyimgptr, char transformstring[], int invert,
	      xyzdouble scale, xyzdouble origin_of_rotates,
	      int calculateresize=1, psydims *outsize=NULL);
//  psyresample(psyimg *psyimgptr, xyzidouble factors);
  void initpsyresample(psyimg *psyimgptr, matrix4X4 InvTransform,
		       xyzdouble scale, xyzdouble origin_of_transform,
		       matrix4X4 transform, psydims *outsize=NULL,
		       int calculateresize=0);
  void getnextpixel(char *pixel);
  void initgetpixel(psytype pixeltype, int x, int y, int z, int i);
  void incgetpixel();
  void output_tree(ostream *out) {psyimglnkpxl::output_tree(out);*out<<"::psyresample";};
};

psyresample::psyresample(psyimg *psyimgptr, char transformstring[],
			 int invert, xyzdouble scale,
			 xyzdouble origin_of_transform, int calculateresize,
			 psydims *outsize)
{
// use input resolution to convert all dimension to the same meter units
  psyres resolution=psyimgptr->getres();
  if(fabs(resolution.x) < SMALLEST_RES)resolution.x=1.0;
  if(fabs(resolution.y) < SMALLEST_RES)resolution.y=1.0;
  if(fabs(resolution.z) < SMALLEST_RES)resolution.z=1.0;
  matrix4X4 InvTransform=parse_transform(transformstring, resolution, !invert);
  matrix4X4 transform=parse_transform(transformstring, resolution, invert);
  if(outsize != NULL) {
    initpsyresample(psyimgptr, InvTransform, scale, origin_of_transform,
		    transform, outsize, 0);
  }
  else {
    initpsyresample(psyimgptr, InvTransform, scale, origin_of_transform,
		    transform, NULL, calculateresize);
  }
}

void psyresample::initpsyresample(psyimg *psyimgptr, matrix4X4 InvTransform,
				  xyzdouble scale,
				  xyzdouble origin_of_transform,
				  matrix4X4 transform,
				  psydims *outsize, int calculateresize)
{
  psydims dim=psyimgptr->getsize();
  if(outsize != NULL) dim= *outsize;
  inorig=psyimgptr->getorig();
  inend=psyimgptr->getend();
// build transform matrix and its inverse
// which means working with inverses
// shift origin to pixel location around which rotation will occur
  matrix4X4 Orig_shift(Identity), InvOrig_shift(Identity);
  Orig_shift.m.m14=origin_of_transform.x;
  Orig_shift.m.m24=origin_of_transform.y;
  Orig_shift.m.m34=origin_of_transform.z;
  InvOrig_shift.m.m14 = -Orig_shift.m.m14;
  InvOrig_shift.m.m24 = -Orig_shift.m.m24;
  InvOrig_shift.m.m34 = -Orig_shift.m.m34;
// use input resolution to convert all dimension to the same meter units
  psyres resolution=psyimgptr->getres();
  if(fabs(resolution.x) < SMALLEST_RES)resolution.x=1.0;
  if(fabs(resolution.y) < SMALLEST_RES)resolution.y=1.0;
  if(fabs(resolution.z) < SMALLEST_RES)resolution.z=1.0;
  matrix4X4 InRes(Identity), InvInRes(Identity);
  InRes.m.m11=resolution.x; InRes.m.m22=resolution.y;
  InRes.m.m33=resolution.z;
  InvInRes.m.m11=1.0/resolution.x; InvInRes.m.m22=1.0/resolution.y;
  InvInRes.m.m33=1.0/resolution.z;
// incoporate scaling
  if(fabs(scale.x) < SMALLEST_SCALE)scale.x=1;
  if(fabs(scale.y) < SMALLEST_SCALE)scale.y=1;
  if(fabs(scale.z) < SMALLEST_SCALE)scale.z=1;
  matrix4X4 InvScale(Identity);
  InvScale.m.m11=1.0/scale.x; InvScale.m.m22=1.0/scale.y;
  InvScale.m.m33=1.0/scale.z;
// combine
  matrix4X4 InvCombined=
    Orig_shift*InvInRes*InvTransform*InRes*InvScale*InvOrig_shift;
  dispmatrix4X4("InvCombined", &InvCombined);
  psydims outorig;
  outorig.x = 0; outorig.y = 0; outorig.z = 0; outorig.i = 0;
// calculate forward combined transform
  matrix4X4 Scale(Identity);
  Scale.m.m11=scale.x; Scale.m.m22=scale.y;
  Scale.m.m33=scale.z;
  matrix4X4 Combined=
    Orig_shift*Scale*InvInRes*transform*InRes*InvOrig_shift;
  dispmatrix4X4("Combined", &Combined);
  matrix4X4 tmp4X4 = InvCombined * Combined;
  dispmatrix4X4("InvCombined * Combined", &tmp4X4);
// calculate output required dimensions if given forward transform
  if(calculateresize) {
    xyzidouble doutorig = Combined*xyziint2double(inorig);
    xyzidouble doutend = Combined*xyziint2double(inend);
    // swap the lesser numbers to the origin
    double dtmp=0;
    if(doutorig.x > doutend.x) {
      dtmp=doutorig.x; doutorig.x=doutend.x; doutend.x=dtmp;
    }
    if(doutorig.y > doutend.y) {
      dtmp=doutorig.y; doutorig.y=doutend.y; doutend.y=dtmp;
    }
    if(doutorig.z > doutend.z) {
      dtmp=doutorig.z; doutorig.z=doutend.z; doutend.z=dtmp;
    }
    if(doutorig.i > doutend.i) {
      dtmp=doutorig.i; doutorig.i=doutend.i; doutend.i=dtmp;
    }
    psydims ioutorig = lesserxyzidouble2int(doutorig);
    psydims ioutend = greaterxyzidouble2int(doutend);
//    psydims ioutorig = roundxyzidouble2int(doutorig);
//    psydims ioutend = roundxyzidouble2int(doutend);
    dim = (ioutend - ioutorig) + 1;
cout<<"output dim=("<<dim.x<<','<<dim.y<<','<<dim.z<<','<<dim.i<<")\n";
// create shift matrix to transform output origin of 0,0,0 to this origin
    matrix4X4 InvOutOrigShift(Identity);
    InvOutOrigShift.m.m14 = ioutorig.x; InvOutOrigShift.m.m24 = ioutorig.y;
    InvOutOrigShift.m.m34 = ioutorig.z;
//    InvOutOrigShift.m.m14 = doutorig.x; InvOutOrigShift.m.m24 = doutorig.y;
//    InvOutOrigShift.m.m34 = doutorig.z;
    tmp4X4 = InvCombined * InvOutOrigShift;
    InvCombined.reset(&tmp4X4);
  }
  if(dim.x<1)dim.x=1; if(dim.y<1)dim.y=1; if(dim.z<1)dim.z=1;
  if(dim.i<1)dim.i=1;
// calculate origin relative to input
  orig_rel2in= InvCombined*xyziint2double(outorig);
// use unit vectors to calculate increments
  xyzidouble unitv;
  unitv.x=unitv.y=unitv.z=unitv.i=0; unitv.x+=1;
  xinc_rel2in = (InvCombined * unitv) - orig_rel2in;
  unitv.x=unitv.y=unitv.z=unitv.i=0; unitv.y+=1;
  yinc_rel2in = (InvCombined * unitv) - orig_rel2in;
  unitv.x=unitv.y=unitv.z=unitv.i=0; unitv.z+=1;
  zinc_rel2in = (InvCombined * unitv) - orig_rel2in;
  iinc_rel2in.x=iinc_rel2in.y=iinc_rel2in.z=0; iinc_rel2in.i=1;
cout<<"orig_rel2in=("<<orig_rel2in.x<<','<<orig_rel2in.y<<','<<orig_rel2in.z
  <<','<<orig_rel2in.i<<")\n";
cout<<"xinc_rel2in=("<<xinc_rel2in.x<<','<<xinc_rel2in.y<<','<<xinc_rel2in.z
  <<','<<xinc_rel2in.i<<")\n";
cout<<"yinc_rel2in=("<<yinc_rel2in.x<<','<<yinc_rel2in.y<<','<<yinc_rel2in.z
  <<','<<yinc_rel2in.i<<")\n";
cout<<"zinc_rel2in=("<<zinc_rel2in.x<<','<<zinc_rel2in.y<<','<<zinc_rel2in.z
  <<','<<zinc_rel2in.i<<")\n";
cout<<"iinc_rel2in=("<<iinc_rel2in.x<<','<<iinc_rel2in.y<<','<<iinc_rel2in.z
  <<','<<iinc_rel2in.i<<")\n";
  resolution=InvScale*resolution;
  initpsyimglnkpxl(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
		   outorig.x, outorig.y, outorig.z, outorig.i, 0,
		   resolution.x, resolution.y, resolution.z, resolution.i,
		   psyimgptr->getwordres());
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    setspatialtransform(new threeDtransform(tdt->getMatrix() * InvCombined), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    setspatialtransform2(new threeDtransform(tdt->getMatrix() * InvCombined), getspatialtransformcode2());
  }
// set locations relative to input for getpixel
  initgetpixel(type, 0, 0, 0, 0);
  freegetpixel();
}
/*
* psyresample::psyresample(psyimg *psyimgptr, xyzidouble factors)
* {
*   psydims dim=psyimgptr->getsize();
*   inorig=psyimgptr->getorig();
*   inend=psyimgptr->getend();
*   psyres resolution=psyimgptr->getres();
* // avoid divide by zero and allow defaults to 1
*   if(fabs(factors.x) < SMALLEST_SCALE)factors.x=1;
*   if(fabs(factors.y) < SMALLEST_SCALE)factors.y=1;
*   if(fabs(factors.z) < SMALLEST_SCALE)factors.z=1;
*   if(fabs(factors.i) < SMALLEST_SCALE)factors.i=1;
* // set increments relative to input image
* // first zero them out
*   xinc_rel2in.x=xinc_rel2in.y=xinc_rel2in.z=xinc_rel2in.i=0;
*   yinc_rel2in=zinc_rel2in=iinc_rel2in=xinc_rel2in;
* // then set used values
*   xinc_rel2in.x=factors.x;
*   yinc_rel2in.y=factors.y;
*   zinc_rel2in.z=factors.z;
*   iinc_rel2in.i=factors.i;
* // set dims and resolution
*   xyzidouble invfactors;
*   invfactors.x = fabs(1.0/factors.x); invfactors.y = fabs(1.0/factors.y);
*   invfactors.z = fabs(1.0/factors.z); invfactors.i = fabs(1.0/factors.i);
*   dim = xyzidouble2int(xyziint2double(dim) * invfactors + .5);
*   if(dim.x<1)dim.x=1; if(dim.y<1)dim.y=1; if(dim.z<1)dim.z=1;
*   if(dim.i<1)dim.i=1;
*   resolution = resolution * factors;
*   initpsyimglnkpxl(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
* 		   inorig.x, inorig.y, inorig.z, inorig.i, 0,
* 		   resolution.x, resolution.y, resolution.z, resolution.i,
* 		   psyimgptr->getwordres());
* // set locations relative to input for getpixel
*   orig_rel2in=xyziint2double(inorig);
*   initgetpixel(type, 0, 0, 0, 0);
*   freegetpixel();
* }
*/
void psyresample::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  psyimglnkpxl::initgetpixel(pixeltype, x, y, z, i);
  iloc_rel2in= orig_rel2in + (iinc_rel2in * (double)i);
  zloc_rel2in= iloc_rel2in + (zinc_rel2in * (double)z);
  yloc_rel2in= zloc_rel2in + (yinc_rel2in * (double)y);
  loc_rel2in=  yloc_rel2in + (xinc_rel2in * (double)x);
}

void psyresample::incgetpixel()
{
  loc_rel2in = loc_rel2in + xinc_rel2in;
//further changes to rel2in values done by initgetpixel
  psyimglnkpxl::incgetpixel();
}

void psyresample::getnextpixel(char *pixel)
{
// retrieve proper input pixel
//  xyziint intloc_rel2in = xyzidouble2int(loc_rel2in + .5);
  xyziint intloc_rel2in = roundxyzidouble2int(loc_rel2in);
  if(inputpsyimg->inside(intloc_rel2in)) {
    inputpsyimg->getpixel(pixel, type, intloc_rel2in.x, intloc_rel2in.y,
			  intloc_rel2in.z, intloc_rel2in.i);
  }
  else {
    unsigned char c=0;
    pixeltypechg((char *)&c, psyuchar, pixel, type);
  }
  incgetpixel();
}

class interp_psyresample : public  psyresample {
 public:
  interp_psyresample(psyimg *psyimgptr, char transformstring[], int invert,
		     xyzdouble scale, xyzdouble origin_of_transform,
		     int calculateresize=1, psydims *outsize=NULL)
    : psyresample(psyimgptr, transformstring, invert, scale,
		  origin_of_transform, calculateresize, outsize) {};
//  interp_psyresample(psyimg *psyimgptr, xyzidouble factors)
//    : psyresample(psyimgptr, factors){};
  void getnextpixel(char *pixel);
  void output_tree(ostream *out) {psyresample::output_tree(out);*out<<"::interp_psyresample";};
};

void interp_psyresample::getnextpixel(char *pixel)
{
// interpolate 8 neighbors
  xyziint loc=xyzidouble2int(loc_rel2in);
  xyzdouble wght[2];
  wght[1].x=loc_rel2in.x-(double)loc.x; wght[0].x=1-wght[1].x;
  wght[1].y=loc_rel2in.y-(double)loc.y; wght[0].y=1-wght[1].y;
  wght[1].z=loc_rel2in.z-(double)loc.z; wght[0].z=1-wght[1].z;
  double totwght=0;
  double total=0;
  xyziint locj, lock;
  for(int i=0; i<2; i++, loc.z++) {
    if((inorig.z<=loc.z) && (loc.z<=inend.z) && (wght[i].z>1e-10)) {
      locj=loc;
      for(int j=0; j<2; j++, locj.y++) {
	if((inorig.y<=locj.y) && (locj.y<=inend.y) && (wght[j].y>1e-10)) {
	  lock=locj;
	  for(int k=0; k<2; k++, lock.x++) {
	    if((inorig.x<=lock.x) && (lock.x<=inend.x) && (wght[k].x>1e-10)) {
	      double current_wght=wght[k].x * wght[j].y * wght[i].z;
	      totwght += current_wght;
	      char_or_largest_pixel inpixel;
	      inputpsyimg->getpixel(inpixel.c, type,
				    lock.x, lock.y, lock.z, lock.i);
	      double dpixel;
	      pixeltypechg(inpixel.c, type, (char *)&dpixel, psydouble);
	      total += current_wght * dpixel;
	    }
	  }
	}
      }
    }
  }
  if(totwght > 1e-10)total /= totwght;
  else total=0;
  pixeltypechg((char *)&total, psydouble, pixel, type);
  incgetpixel();
}
/*
* class psyresamplez : public psypgbuff {
*   xyzdouble factors;
*   psydims in_end;
*   psypgbuff *dualplanebuffer;
*  protected:
*   void fillpage(char *buff, int z, int i);
*  public:
*   psyresamplez(psyimg *psyimgptr, xyzdouble factors);
*   ~psyresamplez();
*   void output_tree(ostream *out) {psypgbuff::output_tree(out);*out<<"::psyresamplez";};
* };
* 
* psyresamplez::psyresamplez(psyimg *psyimgptr, xyzdouble factors)
* {
*   dualplanebuffer=NULL;
*   psydims dim=psyimgptr->getsize();
*   psydims origin=psyimgptr->getorig();
*   psyres resolution=psyimgptr->getres();
*   in_end=psyimgptr->getend();
* // avoid divide by zero and allow defaults to 1
*   if(fabs(factors.z) < SMALLEST_SCALE)factors.z=1;
*   psyresamplez::factors=factors;
* // calculate inverse factors
*   xyzdouble invfactors;
*   invfactors.z = fabs(1.0/factors.z);
*   dim.z = (int)((double)dim.z * invfactors.z + .5);
*   if(dim.z<1)dim.z=1;
*   resolution.z = resolution.z * factors.z;
*   int maxnumpages=1;
*   initpgbuff(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
* 	     origin.x, origin.y, origin.z, origin.i, 0,
* 	     resolution.x, resolution.y, resolution.z, resolution.i,
* 	     psyimgptr->getwordres(), maxnumpages);
* }
* 
* psyresamplez::~psyresamplez()
* {
*   if(dualplanebuffer != NULL)delete dualplanebuffer;
* }
* 
* void psyresamplez::fillpage(char *buff, int z, int i)
* {
*   double trueplane=(double)orig.z + factors.z * (double)(z-orig.z);
*   int lowerplane = (int)trueplane;
*   double lower_plane_dist=trueplane - (double)lowerplane;
*   int upperplane=lowerplane+1;
*   if(upperplane > in_end.z)upperplane=lowerplane;
*   else if( fabs(lower_plane_dist)
* 	  < SMALLEST_SCALE )upperplane=lowerplane; // close to same plane
*   if(upperplane == lowerplane) {
* // duplicate input plane
*     psypgbuff::fillpage(buff, upperplane, i);
*   }
*   else
*   {
* // perform linear interpolation between planes
*     if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
*     double factor2 = lower_plane_dist;
*     double factor1 = 1.0 - factor2;
*     char *yoptr=buff;
*     char *yiptr1=dualplanebuffer->getzptrlocked(lowerplane,i);
*     char *yiptr2=dualplanebuffer->getzptrlocked(upperplane,i);
*     for(int y=orig.y; y<=end.y;
* 	y++, yoptr+=inc.y, yiptr1+=inc.y, yiptr2+=inc.y) {
*       char *xoptr=yoptr;
*       char *xiptr1=yiptr1;
*       char *xiptr2=yiptr2;
*       for(int x=orig.x; x<=end.x; x++, 
* 	  xoptr+=inc.x, xiptr1+=inc.x, xiptr2+=inc.x) {
* 	double in1, in2, out;
* 	type2double(xiptr1, type, (char *)&in1);
* 	type2double(xiptr2, type, (char *)&in2);
* 	out= factor1 * in1 + factor2 * in2;
* 	pixeltypechg((char *)&out, psydouble, xoptr, type);
*       } // end for x
*     } // end for y
*     dualplanebuffer->unlockzptr(lowerplane,i);
*     dualplanebuffer->unlockzptr(upperplane,i);
*   }
* }
*/
/*
* class psyresamplexyz : public psypgbuff {
*   xyzdouble factors;
*   psydims in_end;
*   psypgbuff *dualplanebuffer;
*   int *xin_offsets;
*   int *yin_offsets;
*   int *zin_locations;
*   double *x_weights;
*   double *y_weights;
*   double *z_weights;
*  protected:
*   void fillpage(char *buff, int z, int i);
*  public:
*   psyresamplexyz(psyimg *psyimgptr, xyzdouble factors);
*   ~psyresamplexyz();
*   void output_tree(ostream *out) {psypgbuff::output_tree(out);*out<<"::psyresamplexyz";};
* };
* 
* psyresamplexyz::psyresamplexyz(psyimg *psyimgptr, xyzdouble factors)
* {
*   dualplanebuffer=NULL;
*   x_weights=y_weights=z_weights=NULL;
*   xin_offsets=yin_offsets=zin_locations=NULL;
*   psydims dim=psyimgptr->getsize();
*   psydims origin=psyimgptr->getorig();
*   psyres resolution=psyimgptr->getres();
*   in_end=psyimgptr->getend();
* // avoid divide by zero and allow defaults to 1
*   if(fabs(factors.x) < SMALLEST_SCALE)factors.x=1;
*   if(fabs(factors.y) < SMALLEST_SCALE)factors.y=1;
*   if(fabs(factors.z) < SMALLEST_SCALE)factors.z=1;
*   psyresamplexyz::factors=factors;
* // calculate inverse factors
*   xyzidouble invfactors;
*   invfactors.x = fabs(1.0/factors.x);
*   invfactors.y = fabs(1.0/factors.y);
*   invfactors.z = fabs(1.0/factors.z);
*   invfactors.i = 1;
*   dim = xyzidouble2int(xyziint2double(dim) * invfactors + .5);
*   if(dim.x<1)dim.x=1;
*   if(dim.y<1)dim.y=1;
*   if(dim.z<1)dim.z=1;
*   resolution.x *= factors.x; resolution.y *= factors.y;
*   resolution.z *= factors.z;
*   int maxnumpages=1;
*   initpgbuff(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
* 	     origin.x, origin.y, origin.z, origin.i, 0,
* 	     resolution.x, resolution.y, resolution.z, resolution.i,
* 	     psyimgptr->getwordres(), maxnumpages);
* // calculate x weights
*   if(fabs(factors.x-1.0) > SMALLEST_SCALE) {
*     if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
*     x_weights = new double[dim.x];
*     xin_offsets = new int[dim.x];
*     int x;
*     double true_pixel;
*     for(x=0, true_pixel=orig.x; x<=dim.x; x++, true_pixel+=factors.x) {
*       int x_lower_pixel=(int)true_pixel;
*       xin_offsets[x]=
* 	dualplanebuffer->offset(x_lower_pixel, orig.y, orig.z, orig.i)
* 	  - dualplanebuffer->offset(orig.x, orig.y, orig.z, orig.i);
*       x_weights[x]=(double)(x_lower_pixel+1)-true_pixel;
*       if(fabs(x_weights[x]-1) < SMALLEST_SCALE)x_weights[x]=1;
*     }
*   }
* // calculate y weights
*   if(fabs(factors.y-1.0) > SMALLEST_SCALE) {
*     if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
*     y_weights = new double[dim.y];
*     yin_offsets = new int[dim.y];
*     int y;
*     double true_pixel;
*     for(y=0, true_pixel=orig.y; y<=dim.y; y++, true_pixel+=factors.y) {
*       int y_lower_pixel=(int)true_pixel;
*       yin_offsets[y]=
* 	dualplanebuffer->offset(orig.x, y_lower_pixel, orig.z, orig.i)
* 	  - dualplanebuffer->offset(orig.x, orig.y, orig.z, orig.i);
*       y_weights[y]=(double)(y_lower_pixel+1)-true_pixel;
*       if(fabs(y_weights[y]-1) < SMALLEST_SCALE)y_weights[y]=1;
*     }
*   }
* // calculate z weights
*   z_weights = new double[dim.z];
*   zin_locations = new int[dim.z];
*   if(fabs(factors.z-1.0) > SMALLEST_SCALE) {
*     if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
*     int z;
*     double true_pixel;
*     for(z=0, true_pixel=orig.z; z<=dim.z; z++, true_pixel+=factors.z) {
*       int z_lower_pixel=(int)true_pixel;
*       zin_locations[z]=z_lower_pixel;
*       z_weights[z]=(double)(z_lower_pixel+1)-true_pixel;
*       if(fabs(z_weights[z]-1) < SMALLEST_SCALE)z_weights[z]=1;
*     }
*   }
*   else {
*     for(int z=0, true_pixel=orig.z; z<=dim.z; z++, true_pixel+=1) {
*       z_weights[z]=1;
*       zin_locations[z]=true_pixel;
*     }
*   }
* }
* 
* psyresamplexyz::~psyresamplexyz()
* {
*   if(dualplanebuffer != NULL)delete dualplanebuffer;
*   if(xin_offsets != NULL)delete[] xin_offsets;
*   if(yin_offsets != NULL)delete[] yin_offsets;
*   if(zin_locations != NULL)delete[] zin_locations;
*   if(x_weights != NULL)delete[] x_weights;
*   if(y_weights != NULL)delete[] y_weights;
*   if(z_weights != NULL)delete[] z_weights;
* }
* 
* #define COMBINE3FLAGS(flag1,flag2,flag3)\
*   (((flag1)<<16) | ((flag2)<<8) | ((flag3)))
* 
* void psyresamplexyz::fillpage(char *buff, int z, int i)
* {
*   switch(COMBINE3FLAGS(xin_offsets!=NULL, yin_offsets!=NULL,
* 		       z_weights[z] != 1)) {
*   case COMBINE3FLAGS(0,0,0):
* // no interpolation
*     psypgbuff::fillpage(buff, z, i);
*     break;
*   case COMBINE3FLAGS(0,0,1):
* // interpolate z only
*     {
*       if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
*       double factor1 = z_weights[z];
*       double factor2 = 1.0 - factor1;;
*       int lowerplane = zin_locations[z];
*       int upperplane = lowerplane + 1;
*       char *yoptr=buff;
*       char *yiptr1=dualplanebuffer->getzptrlocked(lowerplane,i);
*       char *yiptr2=dualplanebuffer->getzptrlocked(upperplane,i);
*       for(int y=orig.y; y<=end.y;
* 	  y++, yoptr+=inc.y, yiptr1+=inc.y, yiptr2+=inc.y) {
* 	char *xoptr=yoptr;
* 	char *xiptr1=yiptr1;
* 	char *xiptr2=yiptr2;
* 	for(int x=orig.x; x<=end.x; x++, 
* 	    xoptr+=inc.x, xiptr1+=inc.x, xiptr2+=inc.x) {
* 	  double in1, in2, out;
* 	  type2double(xiptr1, type, (char *)&in1);
* 	  type2double(xiptr2, type, (char *)&in2);
* 	  out= factor1 * in1 + factor2 * in2;
* 	  pixeltypechg((char *)&out, psydouble, xoptr, type);
* 	} // end for x
*       } // end for y
*       dualplanebuffer->unlockzptr(lowerplane,i);
*       dualplanebuffer->unlockzptr(upperplane,i);
*     }
*     break;
*   case COMBINE3FLAGS(1,1,0):
*   case COMBINE3FLAGS(1,0,0):
*   case COMBINE3FLAGS(0,1,0):
* // interpolate x and/or y but not z
*     {
*     }
*     break;
*   case COMBINE3FLAGS(1,0,1):
*   case COMBINE3FLAGS(0,1,1):
*     break;
*   case COMBINE3FLAGS(1,1,1):
* // interpolate x and/or y and z
*     {
*       if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
*       double factor_z1 = z_weights[z];
*       double factor_z2 = 1.0 - factor_z1;;
*       int lowerplane = zin_locations[z];
*       int upperplane = lowerplane + 1;
*       char *yoptr=buff;
*       char *yiptr_z1=dualplanebuffer->getzptrlocked(lowerplane,i);
*       char *yiptr_z2=dualplanebuffer->getzptrlocked(upperplane,i);
*       for(int y=0; y<size.y; y++, yoptr+=inc.y) {
* 	double factor_y1 = y_weights[y];
* 	double factor_y2 = 1-factor_y1;
* 	char *xoptr=yoptr;
* 	char *xiptr_z1_y1=yiptr_z1 + yin_offsets[y];
* 	char *xiptr_z1_y2=xiptr_z1_y1 + inc.y;
* 	char *xiptr_z2_y1=yiptr_z2 + yin_offsets[y];
* 	char *xiptr_z2_y2=xiptr_z2_y1 + inc.y;
* 	for(int x=0; x<size.x; x++, 
* 	    xoptr+=inc.x) {
* 	  double factor_x1 = x_weights[x];
* 	  double factor_x2 = 1-factor_x1;
* 	  double in_z1_y1_x1, in_z1_y1_x2, in_z1_y2_x1, in_z1_y2_x2;
* 	  double in_z2_y1_x1, in_z2_y1_x2, in_z2_y2_x1, in_z2_y2_x2;
* 	  double out;
* 	  char *ptr=xiptr_z1_y1+xin_offsets[x];
* 	  type2double(ptr, type, (char *)&in_z1_y1_x1);
* 	  type2double(ptr+inc.x, type, (char *)&in_z1_y1_x2);
* 	  ptr=xiptr_z1_y2 + xin_offsets[x];
* 	  type2double(ptr, type, (char *)&in_z1_y2_x1);
* 	  type2double(ptr+inc.x, type, (char *)&in_z1_y2_x2);
* 	  ptr=xiptr_z2_y1+xin_offsets[x];
* 	  type2double(ptr, type, (char *)&in_z2_y1_x1);
* 	  type2double(ptr+inc.x, type, (char *)&in_z2_y1_x2);
* 	  ptr=xiptr_z2_y2+xin_offsets[x];
* 	  type2double(ptr, type, (char *)&in_z2_y2_x1);
* 	  type2double(ptr+inc.x, type, (char *)&in_z2_y2_x2);
* 	  out= factor_z1*(factor_y1*
* 			  (factor_x1*in_z1_y1_x1+factor_x2*in_z1_y1_x2) +
* 			  factor_y2*
* 			  (factor_x1*in_z1_y2_x1+factor_x2*in_z1_y2_x2)
* 			  ) + 
* 	       factor_z2*(factor_y1*
* 			  (factor_x1*in_z2_y1_x1+factor_x2*in_z2_y1_x2) +
* 			  factor_y2*
* 			  (factor_x1*in_z2_y2_x1+factor_x2*in_z2_y2_x2)
* 			  );
* 	  pixeltypechg((char *)&out, psydouble, xoptr, type);
* 	} // end for x
*       } // end for y
*       dualplanebuffer->unlockzptr(lowerplane,i);
*       dualplanebuffer->unlockzptr(upperplane,i);
*     }
*     break;
*   }
* }
*/

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  psyimg *psyimgptr;
  int npages=2;
  char *transformstring=NULL;
  psyres outres;
  outres.x=outres.y=outres.z=outres.i=0;
  xyzdouble factors;
  factors.x=factors.y=factors.z=0;
  psytype outtype=psynotype; //defaults to input type
  psyfileclass outfileclass=psynoclass; //defaults to in file format
  int *outfileclassinfo = NULL;
  int interpolate=0;
  int invert_flag=0;
  int resize_flag=0;
  xyzidouble transform_origin;
  transform_origin.x=transform_origin.y=transform_origin.z=transform_origin.i=0;
  int transform_origin_set = 0;
  xyziint *outsize=NULL;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-interpolate]"<<'\n';
      cout <<"       [-t transform_string(ie. XDz15XPx3X) [-invert]]\n";
      cout <<"       [-xf x_factor | -xres x_outres]\n";
      cout <<"       [-yf y_factor | -yres y_outres]\n";
      cout <<"       [-zf z_factor | -zres z_outres]\n";
      cout <<"       [-transform_origin x,y,z]\n";
      cout <<"       [-np number_of_planes_to_buffer]\n";
      cout <<"       [-resize]\n";
      cout <<"       [-outsize xdim,ydim,zdim]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i], "-interpolate")==0)interpolate=1;
    else if(strcmp(argv[i], "-invert")==0)invert_flag=1;
    else if(strcmp(argv[i], "-resize")==0)resize_flag=1;
    else if((strcmp(argv[i], "-t")==0) && ((i+1)<argc))
      transformstring=argv[++i];
    else if((strcmp(argv[i], "-np")==0) && ((i+1)<argc))
      npages=atoi(argv[++i]);
    else if((strcmp(argv[i], "-xf")==0) && ((i+1)<argc))
      factors.x=atof(argv[++i]);
    else if((strcmp(argv[i], "-yf")==0) && ((i+1)<argc))
      factors.y=atof(argv[++i]);
    else if((strcmp(argv[i], "-zf")==0) && ((i+1)<argc))
      factors.z=atof(argv[++i]);
    else if((strcmp(argv[i], "-xres")==0) && ((i+1)<argc))
      outres.x=atof(argv[++i]);
    else if((strcmp(argv[i], "-yres")==0) && ((i+1)<argc))
      outres.y=atof(argv[++i]);
    else if((strcmp(argv[i], "-zres")==0) && ((i+1)<argc))
      outres.z=atof(argv[++i]);
    else if((strcmp(argv[i],"-outsize")==0) && ((i+1)<argc)) {
      outsize = new xyziint(); outsize->i = 1;
      if(sscanf(argv[++i],"%d,%d,%d", &outsize->x,
		&outsize->y, &outsize->z) != 3) {
	cerr << argv[0] << ": error parsing output size: ";
	cerr << argv[i-1] << " " << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-transform_origin")==0) && ((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf,%lf,%lf", &transform_origin.x,
		&transform_origin.y, &transform_origin.z) != 3) {
	cerr << argv[0] << ": error parsing in_origin: ";
	cerr << argv[i-1] << " " << argv[i] << '\n';
	exit(1);
      }
      else transform_origin_set = 1;
    }
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
  
// open input files
  psytype intype;
  psyfileclass infileclass;
  psyimg *inputimgptr = psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outtype == psynotype)outtype=intype;
  if(outfileclass == psynoclass)outfileclass=infileclass;
  psyimgptr = inputimgptr;

// buffer
  psypgbuff buffered(psyimgptr, npages);
  psyimgptr = (psyimg *)&buffered;

  psyres resolution=psyimgptr->getres();
  psydims dims=psyimgptr->getsize();
  psydims orig=psyimgptr->getorig();
  psydims end=psyimgptr->getend();

//set correct factors
  if(fabs(outres.x) > SMALLEST_RES) {
    if(fabs(resolution.x) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file x resolution too small for factor calculate\n";
      exit(1);
    }
    factors.x = resolution.x/outres.x;
  }

  if(fabs(outres.y) > SMALLEST_RES) {
    if(fabs(resolution.y) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file y resolution too small for factor calculate\n";
      exit(1);
    }
    factors.y = resolution.y/outres.y;
  }

  if(fabs(outres.z) > SMALLEST_RES) {
    if(fabs(resolution.z) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file z resolution too small for factor calculate\n";
      exit(1);
    }
    factors.z = resolution.z/outres.z;
  }

  cout<<"scale factors=("<<factors.x<<','<<factors.y<<','<<factors.z<<")\n";

// set default origin to center of image
  if(! transform_origin_set) transform_origin =
    xyziint2double(orig) + (xyziint2double(end - orig) * 0.5);

/*
  cout<<"rotation=("<<RAD_TO_DEG*rotation.x<<','
    <<RAD_TO_DEG*rotation.y<<','<<RAD_TO_DEG*rotation.z<<")\n";
  char transstring[256];
  sprintf(transstring, "XRx%lfXRy%lfXRz%lfXPx%lfXPy%lfXPz%lfzX",
	  rotation.x, rotation.y, rotation.z,
	  translation.x, translation.y, translation.z);
  matrix4X4 InvCombined=parse_transform(transstring, resolution, 1);
*/
  if(transformstring != NULL)cout<<"transformstring="<<transformstring<<'\n';
  psyimg *resampledimg;
  if(interpolate) {
    resampledimg = new interp_psyresample(psyimgptr, transformstring,
					  invert_flag, factors,
					  (xyzdouble) transform_origin,
					  resize_flag, outsize);
  }
  else {
    resampledimg = new psyresample(psyimgptr, transformstring,
				   invert_flag, factors,
				   (xyzdouble) transform_origin,
				   resize_flag, outsize);
  }
  psyimgptr = (psyimg *)resampledimg;

// build new description
  char *desc = new char[strlen(infile) + strlen(" resampled image: ") + 1];
  strcpy(desc, " resampled image: ");
  strcat(desc, infile);
  psyimgptr->setdescription(desc);
  delete[] desc;

// set date and time to current date and time
  psyimgptr->setdate();
  psyimgptr->settime();

// output result to an image file
  psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(resampledimg != NULL)delete resampledimg;
  if(inputimgptr != NULL)delete inputimgptr;
}
