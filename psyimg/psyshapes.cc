#include "psyhdr.h"

psyimgblk::psyimgblk(psyimg *psyimgptr,
		     int xorig, int yorig, int zorig, int iorig,
		     int xend, int yend, int zend, int iend,
		     int reset_origin,
		     psytype pixeltype)
{
  init(psyimgptr, xorig, yorig, zorig, iorig,
       xend, yend, zend, iend, reset_origin, pixeltype);
}

psyimgblk::psyimgblk(psyimg *psyimgptr,
		     psydims orig, psydims end,
		     int reset_origin,
		     psytype pixeltype)
{
  init(psyimgptr, orig.x, orig.y, orig.z, orig.i,
       end.x, end.y, end.z, end.i, reset_origin, pixeltype);
}

void psyimgblk::init(psyimg *psyimgptr,
		     int xorig, int yorig, int zorig, int iorig,
		     int xend, int yend, int zend, int iend,
		     int reset_origin,
		     psytype pixeltype)
{
  int xdim, ydim, zdim, idim;
  double xres, yres, zres, ires;

  if(!psyimgptr->inside(xorig,yorig,zorig,iorig) ||
     !psyimgptr->inside(xend,yend,zend,iend)) {
    cerr<<"psyimgblk::initpsyimgblk requested region outside of image\n";
    cerr<<"orig=("<<xorig<<","<<yorig<<","<<zorig<<","<<iorig<<")\n";
    cerr<<"end=("<<xend<<","<<yend<<","<<zend<<","<<iend<<")\n";
    exit(1);
  }
  xdim=xend-xorig+1; ydim=yend-yorig+1;
  zdim=zend-zorig+1; idim=iend-iorig+1;
  if(xdim<1 || ydim<1 || zdim<1 || idim<1) {
    cerr<<"psyimgblk::initpsyimgblk - error in requested region\n";
    cerr<<"orig=("<<xorig<<','<<yorig<<','<<zorig<<','<<iorig<<")\n";
    cerr<<"end=("<<xend<<','<<yend<<','<<zend<<','<<iend<<")\n";
    exit(1);
  }
  if(reset_origin) {
    orig_fix.x=xorig; orig_fix.y=yorig;
    orig_fix.z=zorig; orig_fix.i=iorig;
    xorig=yorig=zorig=iorig=0;
  }
  else orig_fix.x=orig_fix.y=orig_fix.z=orig_fix.i=0;

  psyimgptr->getres(&xres, &yres, &zres, &ires);
  initpsyimglnkpxl(psyimgptr, xdim, ydim, zdim, idim, pixeltype,
		   xorig, yorig, zorig, iorig, 0,
		   xres, yres, zres, ires, psyimgptr->getwordres());

  // fix spatial transforms
  xyzidouble newvoxorig;
  newvoxorig.x = xorig+orig_fix.x; newvoxorig.y = yorig+orig_fix.y;
  newvoxorig.z = zorig+orig_fix.z; newvoxorig.i = 1.0l;
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform(new threeDtransform(newmatrix), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform2(new threeDtransform(newmatrix), getspatialtransformcode2());
  }
}

void psyimgblk::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			  int iorig, int xend, int yend, int zend, int iend,
			  int out_xinc, int out_yinc, int out_zinc, int out_iinc,
			  psytype pixeltype)
{
  inputpsyimg->copyblock(outbuff, xorig+orig_fix.x, yorig+orig_fix.y,
			 zorig+orig_fix.z, iorig+orig_fix.i,
			 xend+orig_fix.x, yend+orig_fix.y,
			 zend+orig_fix.z, iend+orig_fix.i,
			 out_xinc, out_yinc, out_zinc, out_iinc,
			 pixeltype);
}

void psyimgblk::getnextpixel(char *pixel)
{
  inputpsyimg->getpixel(pixel, getpixeltype,
			getpixelloc.x+orig_fix.x, getpixelloc.y+orig_fix.y,
			getpixelloc.z+orig_fix.z, getpixelloc.i+orig_fix.i);
  incgetpixel();
}

psyimgconstant::psyimgconstant(psyimg *psyimgptr, double value,
			       psytype pixeltype)
{
  init(psyimgptr, value, pixeltype);
}

psyimgconstant::psyimgconstant(double value, int xdim, int ydim,
			       int zdim, int idim,
			       psytype pixeltype,
			       int xorig, int yorig,
			       int zorig, int iorig,
			       double xres, double yres,
			       double zres, double ires,
			       double wres)
{
  initpsyimglnkpxl(NULL, xdim, ydim, zdim, idim, pixeltype,
		   xorig, yorig, zorig, iorig, 0,
		   xres, yres, zres, ires, wres);
  psyimgconstant::value = value;
}

void psyimgconstant::init(psyimg *psyimgptr, double value,
			  psytype pixeltype)
{
  initpsyimglnkpxl(psyimgptr, pixeltype);
  psyimgconstant::value = value;
}

void psyimgconstant::getnextpixel(char *pixel)
{
  pixeltypechg((char *)&value, psydouble, pixel, getpixeltype);
}

psyimgshape::psyimgshape()
{
  foreground_value=1.0;
  background_value=0.0;
  in_image_as_background=0;
}

psyimgshape::psyimgshape(psyimg *psyimgptr, psytype pixeltype)
{
  init(psyimgptr, pixeltype);
}

void psyimgshape::init(psyimg *psyimgptr, psytype pixeltype)
{
  initpsyimglnkpxl(psyimgptr, pixeltype);
  foreground_value = 1.0;
  background_value = 0.0;
  in_image_as_background=0;
}

double psyimgshape::getbackground_value() {
  double value;
  if(in_image_as_background) {
    char_or_largest_pixel inpixel;
    psytype intype=inputpsyimg->gettype();
    inputpsyimg->getpixel(inpixel.c, intype,
			  getpixelloc.x, getpixelloc.y,
			  getpixelloc.z, getpixelloc.i);
    type2double(inpixel.c, intype, (char *)&value);
  }
  else value=background_value;
  return(value);
}

void psyimgshape::getnextpixel(char *pixel)
{
  double value;
  if(inshape(getpixelloc))value=getforeground_value();
  else value=getbackground_value();
  pixeltypechg((char *)&value, psydouble, pixel, getpixeltype);
  incgetpixel();
}

psyimgbox::psyimgbox()
{
  box_orig=getorig();
  box_end=getend();
}

psyimgbox::psyimgbox(psyimg *psyimgptr, psydims box_orig, psydims box_end,
		     psytype pixeltype)
{
  init(psyimgptr, box_orig, box_end, pixeltype);
}

void psyimgbox::init(psyimg *psyimgptr, psydims box_orig, psydims box_end,
		     psytype pixeltype)
{
  psyimgshape::init(psyimgptr, pixeltype);
  psyimgbox::box_orig = box_orig;
  psyimgbox::box_end = box_end;
}

int psyimgbox::inshape(psydims location) {
  if((box_orig.x<=location.x) && (location.x<=box_end.x) &&
     (box_orig.y<=location.y) && (location.y<=box_end.y) &&
     (box_orig.z<=location.z) && (location.z<=box_end.z) &&
     (box_orig.i<=location.i) && (location.i<=box_end.i))
    return(1);
  else return(0);
}

psyimgsphere::psyimgsphere(psyimg *psyimgptr, double radius,
			   double xcenter, double ycenter, double zcenter,
			   psytype pixeltype)
{
  init(psyimgptr, radius, xcenter, ycenter, zcenter, pixeltype);
}

void psyimgsphere::init(psyimg *psyimgptr, double radius,
			double xcenter, double ycenter, double zcenter,
			psytype pixeltype)
{
  psyimgshape::init(psyimgptr, pixeltype);
  psyimgsphere::radius_squared=radius*radius;
  centroid.x=xcenter;
  centroid.y=ycenter;
  centroid.z=zcenter;
// set i range to full image
  getorig(NULL, NULL, NULL, &ibegin);
  getend(NULL, NULL, NULL, &iend);
}

void psyimgsphere::setirange(int ibegin, int iend) {
  psyimgsphere::ibegin = ibegin;
  psyimgsphere::iend = iend;
}

int psyimgsphere::inshape(psydims location)
{
  if((location.i < ibegin) || (location.i > iend)) return(0);
  xyzdouble dist = xyzint2double((xyzint)location) - centroid;
  if((dist.x*dist.x + dist.y*dist.y + dist.z*dist.z) <= radius_squared)
    return(1);
  else return(0);
}

psyimgcylinder::psyimgcylinder(psyimg *psyimgptr, double radius,
			   double icenter, double jcenter, int axis,
			   psytype pixeltype)
{
  init(psyimgptr, radius, icenter, jcenter, axis, pixeltype);
}

void psyimgcylinder::init(psyimg *psyimgptr, double radius,
			double icenter, double jcenter, int axis,
			psytype pixeltype)
{
  psyimgshape::init(psyimgptr, pixeltype);
  psyimgcylinder::radius_squared=radius*radius;
  psyimgcylinder::icenter=icenter;
  psyimgcylinder::jcenter=jcenter;
  psyimgcylinder::axis=axis;
// set i range to full image
  getorig(NULL, NULL, NULL, &ibegin);
  getend(NULL, NULL, NULL, &iend);
}

void psyimgcylinder::setirange(int ibegin, int iend) {
  psyimgcylinder::ibegin = ibegin;
  psyimgcylinder::iend = iend;
}

int psyimgcylinder::inshape(psydims location)
{
  if((location.i < ibegin) || (location.i > iend)) return(0);
  double iloc;
  double jloc;
  if(axis == 0) { iloc = location.y; jloc = location.z; }
  else if(axis == 1) { iloc = location.z; jloc = location.x; }
  else { iloc = location.x; jloc = location.y; }
  double distj = iloc - icenter;
  double disti = jloc - jcenter;
  if( ((distj * distj) + (disti * disti)) <= radius_squared ) return(1);
  else return(0);
}

psyimgellipsoid::psyimgellipsoid(psyimg *psyimgptr, xyzdouble centroid,
				 xyzdouble axis_lengths,
				 psytype pixeltype)
{
  init(psyimgptr, centroid, axis_lengths, pixeltype);
}

void psyimgellipsoid::init(psyimg *psyimgptr,  xyzdouble centroid,
			   xyzdouble axis_lengths,
			   psytype pixeltype)
{
  psyimgshape::init(psyimgptr, pixeltype);
  psyimgellipsoid::centroid=centroid;
// convert to half lengths
  axis_lengths = axis_lengths * 0.5;

  squared_factors = axis_lengths * axis_lengths;
// avoid divide by zero
  if(squared_factors.x <= 1e-6)squared_factors.x=1e6;
  else squared_factors.x = 1.0/squared_factors.x;
  if(squared_factors.y <= 1e-6)squared_factors.y=1e6;
  else squared_factors.y = 1.0/squared_factors.y;
  if(squared_factors.z <= 1e-6)squared_factors.z=1e6;
  else squared_factors.z = 1.0/squared_factors.z;
// set i range to full image
  getorig(NULL, NULL, NULL, &ibegin);
  getend(NULL, NULL, NULL, &iend);
}

void psyimgellipsoid::setirange(int ibegin, int iend) {
  psyimgellipsoid::ibegin = ibegin;
  psyimgellipsoid::iend = iend;
}

int psyimgellipsoid::inshape(psydims location)
{
  if((location.i < ibegin) || (location.i > iend)) return(0);
  xyzdouble dist = xyzint2double((xyzint)location) - centroid;
  if((squared_factors.x*dist.x*dist.x +
      squared_factors.y*dist.y*dist.y + 
      squared_factors.z*dist.z*dist.z) <= 1.0)return(1);
  else return(0);
}
