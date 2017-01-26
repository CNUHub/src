#include "psyhdr.h"
#define SMALLEST_RES 1e-20
#define SMALLEST_SCALE 1e-16

class psyresample : public  psyimglnkpxl {
  xyzidouble xinc_rel2in;
  xyzidouble yinc_rel2in;
  xyzidouble zinc_rel2in;
  xyzidouble iinc_rel2in;
  xyzidouble orig_rel2in;
  xyzidouble loc_rel2in;
  xyzidouble yloc_rel2in;
  xyzidouble zloc_rel2in;
  xyzidouble iloc_rel2in;

 public:
  psyresample(psyimg *psyimgptr, xyzidouble factors);
  void getnextpixel(char *pixel);
  void initgetpixel(psytype pixeltype, int x, int y, int z, int i);
  void incgetpixel();
  void output_tree(ostream *out) {psyimglnkpxl::output_tree(out);*out<<"::psyresample";};
};

psyresample::psyresample(psyimg *psyimgptr, xyzidouble factors)
{
  psydims dim=psyimgptr->getsize();
  psydims origin=psyimgptr->getorig();
  psyres resolution=psyimgptr->getres();
// avoid divide by zero and allow defaults to 1
  if(fabs(factors.x) < SMALLEST_SCALE)factors.x=1;
  if(fabs(factors.y) < SMALLEST_SCALE)factors.y=1;
  if(fabs(factors.z) < SMALLEST_SCALE)factors.z=1;
  if(fabs(factors.i) < SMALLEST_SCALE)factors.i=1;
// set increments relative to input image
// first zero them out
  xinc_rel2in.x=xinc_rel2in.y=xinc_rel2in.z=xinc_rel2in.i=0;
  yinc_rel2in=zinc_rel2in=iinc_rel2in=xinc_rel2in;
// then set used values
  xinc_rel2in.x=factors.x;
  yinc_rel2in.y=factors.y;
  zinc_rel2in.z=factors.z;
  iinc_rel2in.i=factors.i;
// set dims and resolution
  xyzidouble invfactors;
  invfactors.x = fabs(1.0/factors.x); invfactors.y = fabs(1.0/factors.y);
  invfactors.z = fabs(1.0/factors.z); invfactors.i = fabs(1.0/factors.i);
  dim = xyzidouble2int(xyziint2double(dim) * invfactors + .5);
  if(dim.x<1)dim.x=1; if(dim.y<1)dim.y=1; if(dim.z<1)dim.z=1;
  if(dim.i<1)dim.i=1;
  resolution = resolution * factors;
  initpsyimglnkpxl(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
		   origin.x, origin.y, origin.z, origin.i, 0,
		   resolution.x, resolution.y, resolution.z, resolution.i,
		   psyimgptr->getwordres());
// fix spatial transforms
  threeDtransform *tdt = getspatialtransform();
  matrix4X4 Factors(Identity);
  Factors.m.m11 = factors.x; Factors.m.m22 = factors.y; Factors.m.m33 = factors.z;
  if(tdt != NULL) {
    if(tdt->isQuaternSet()) {
      cnuquatern *q = tdt->getQuatern();
      q->xfactor = resolution.x; q->yfactor = resolution.y; q->zfactor = resolution.z;
      setspatialtransform(new threeDtransform(*q), getspatialtransformcode2());
      delete q;
    }
    else setspatialtransform(new threeDtransform(tdt->getMatrix() * Factors), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    if(tdt->isQuaternSet()) {
      cnuquatern *q = tdt->getQuatern();
      q->xfactor = resolution.x; q->yfactor = resolution.y; q->zfactor = resolution.z;
      setspatialtransform2(new threeDtransform(*q), getspatialtransformcode2());
      delete q;
    }
    else setspatialtransform2(new threeDtransform(tdt->getMatrix() * Factors), getspatialtransformcode2());
  }
// set locations relative to input for getpixel
  orig_rel2in=xyziint2double(origin);
  initgetpixel(type, 0, 0, 0, 0);
  freegetpixel();
}

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
  xyziint intloc_rel2in = xyzidouble2int(loc_rel2in + .5);
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

class psyresamplez : public psypgbuff {
  xyzdouble factors;
  psydims in_end;
  psypgbuff *dualplanebuffer;
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  psyresamplez(psyimg *psyimgptr, xyzdouble factors);
  ~psyresamplez();
  void output_tree(ostream *out) {psypgbuff::output_tree(out);*out<<"::psyresamplez";};
};

psyresamplez::psyresamplez(psyimg *psyimgptr, xyzdouble factors)
{
  dualplanebuffer=NULL;
  psydims dim=psyimgptr->getsize();
  psydims origin=psyimgptr->getorig();
  psyres resolution=psyimgptr->getres();
  in_end=psyimgptr->getend();
// avoid divide by zero and allow defaults to 1
  if(fabs(factors.z) < SMALLEST_SCALE)factors.z=1;
  psyresamplez::factors=factors;
// calculate inverse factors
  xyzdouble invfactors;
  invfactors.z = fabs(1.0/factors.z);
  dim.z = (int)((double)dim.z * invfactors.z + .5);
  if(dim.z<1)dim.z=1;
  resolution.z = resolution.z * factors.z;
  int maxnumpages=1;
  initpgbuff(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
	     origin.x, origin.y, origin.z, origin.i, 0,
	     resolution.x, resolution.y, resolution.z, resolution.i,
	     psyimgptr->getwordres(), maxnumpages);
}

psyresamplez::~psyresamplez()
{
  if(dualplanebuffer != NULL)delete dualplanebuffer;
}

void psyresamplez::fillpage(char *buff, int z, int i)
{
  double trueplane=(double)orig.z + factors.z * (double)(z-orig.z);
  int lowerplane = (int)trueplane;
  double lower_plane_dist=trueplane - (double)lowerplane;
  int upperplane=lowerplane+1;
  if(upperplane > in_end.z)upperplane=lowerplane;
  else if( fabs(lower_plane_dist)
	  < SMALLEST_SCALE )upperplane=lowerplane; // close to same plane
  if(upperplane == lowerplane) {
// duplicate input plane
    psypgbuff::fillpage(buff, upperplane, i);
  }
  else
  {
// perform linear interpolation between planes
    if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
    double factor2 = lower_plane_dist;
    double factor1 = 1.0 - factor2;
    char *yoptr=buff;
    char *yiptr1=dualplanebuffer->getzptrlocked(lowerplane,i);
    char *yiptr2=dualplanebuffer->getzptrlocked(upperplane,i);
    for(int y=orig.y; y<=end.y;
	y++, yoptr+=inc.y, yiptr1+=inc.y, yiptr2+=inc.y) {
      char *xoptr=yoptr;
      char *xiptr1=yiptr1;
      char *xiptr2=yiptr2;
      for(int x=orig.x; x<=end.x; x++, 
	  xoptr+=inc.x, xiptr1+=inc.x, xiptr2+=inc.x) {
	double in1, in2, out;
	type2double(xiptr1, type, (char *)&in1);
	type2double(xiptr2, type, (char *)&in2);
	out= factor1 * in1 + factor2 * in2;
	pixeltypechg((char *)&out, psydouble, xoptr, type);
      } // end for x
    } // end for y
    dualplanebuffer->unlockzptr(lowerplane,i);
    dualplanebuffer->unlockzptr(upperplane,i);
  }
}

class psyresamplexyz : public psypgbuff {
  xyzdouble factors;
  psydims in_end;
  psypgbuff *dualplanebuffer;
  int *xin_offsets;
  int *yin_offsets;
  int *zin_locations;
  double *x_weights;
  double *y_weights;
  double *z_weights;
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  psyresamplexyz(psyimg *psyimgptr, xyzdouble factors);
  ~psyresamplexyz();
  void output_tree(ostream *out) {psypgbuff::output_tree(out);*out<<"::psyresamplexyz";};
};

psyresamplexyz::psyresamplexyz(psyimg *psyimgptr, xyzdouble factors)
{
  dualplanebuffer=NULL;
  x_weights=y_weights=z_weights=NULL;
  xin_offsets=yin_offsets=zin_locations=NULL;
  psydims dim=psyimgptr->getsize();
  psydims origin=psyimgptr->getorig();
  psyres resolution=psyimgptr->getres();
  in_end=psyimgptr->getend();
// avoid divide by zero and allow defaults to 1
  if(fabs(factors.x) < SMALLEST_SCALE)factors.x=1;
  if(fabs(factors.y) < SMALLEST_SCALE)factors.y=1;
  if(fabs(factors.z) < SMALLEST_SCALE)factors.z=1;
  psyresamplexyz::factors=factors;
// calculate inverse factors
  xyzidouble invfactors;
  invfactors.x = fabs(1.0/factors.x);
  invfactors.y = fabs(1.0/factors.y);
  invfactors.z = fabs(1.0/factors.z);
  invfactors.i = 1;
  dim = xyzidouble2int(xyziint2double(dim) * invfactors + .5);
  if(dim.x<1)dim.x=1;
  if(dim.y<1)dim.y=1;
  if(dim.z<1)dim.z=1;
  resolution.x *= factors.x; resolution.y *= factors.y;
  resolution.z *= factors.z;
  int maxnumpages=1;
  initpgbuff(psyimgptr, dim.x, dim.y, dim.z, dim.i, psynotype,
	     origin.x, origin.y, origin.z, origin.i, 0,
	     resolution.x, resolution.y, resolution.z, resolution.i,
	     psyimgptr->getwordres(), maxnumpages);
// fix spatial transforms
  threeDtransform *tdt = getspatialtransform();
  matrix4X4 Factors(Identity);
  Factors.m.m11 = factors.x; Factors.m.m22 = factors.y; Factors.m.m33 = factors.z;
  if(tdt != NULL) {
    setspatialtransform(new threeDtransform(tdt->getMatrix() * Factors), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    setspatialtransform2(new threeDtransform(tdt->getMatrix() * Factors), getspatialtransformcode2());
  }
// calculate x weights
  if(fabs(factors.x-1.0) > SMALLEST_SCALE) {
    if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
    x_weights = new double[dim.x];
    xin_offsets = new int[dim.x];
    int x;
    double true_pixel;
    for(x=0, true_pixel=orig.x; x<=dim.x; x++, true_pixel+=factors.x) {
      int x_lower_pixel=(int)true_pixel;
      xin_offsets[x]=
	dualplanebuffer->offset(x_lower_pixel, orig.y, orig.z, orig.i)
	  - dualplanebuffer->offset(orig.x, orig.y, orig.z, orig.i);
      x_weights[x]=(double)(x_lower_pixel+1)-true_pixel;
      if(fabs(x_weights[x]-1) < SMALLEST_SCALE)x_weights[x]=1;
    }
  }
// calculate y weights
  if(fabs(factors.y-1.0) > SMALLEST_SCALE) {
    if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
    y_weights = new double[dim.y];
    yin_offsets = new int[dim.y];
    int y;
    double true_pixel;
    for(y=0, true_pixel=orig.y; y<=dim.y; y++, true_pixel+=factors.y) {
      int y_lower_pixel=(int)true_pixel;
      yin_offsets[y]=
	dualplanebuffer->offset(orig.x, y_lower_pixel, orig.z, orig.i)
	  - dualplanebuffer->offset(orig.x, orig.y, orig.z, orig.i);
      y_weights[y]=(double)(y_lower_pixel+1)-true_pixel;
      if(fabs(y_weights[y]-1) < SMALLEST_SCALE)y_weights[y]=1;
    }
  }
// calculate z weights
  z_weights = new double[dim.z];
  zin_locations = new int[dim.z];
  if(fabs(factors.z-1.0) > SMALLEST_SCALE) {
    if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
    int z;
    double true_pixel;
    for(z=0, true_pixel=orig.z; z<=dim.z; z++, true_pixel+=factors.z) {
      int z_lower_pixel=(int)true_pixel;
      zin_locations[z]=z_lower_pixel;
      z_weights[z]=(double)(z_lower_pixel+1)-true_pixel;
      if(fabs(z_weights[z]-1) < SMALLEST_SCALE)z_weights[z]=1;
    }
  }
  else {
    for(int z=0, true_pixel=orig.z; z<=dim.z; z++, true_pixel+=1) {
      z_weights[z]=1;
      zin_locations[z]=true_pixel;
    }
  }
}

psyresamplexyz::~psyresamplexyz()
{
  if(dualplanebuffer != NULL)delete dualplanebuffer;
  if(xin_offsets != NULL)delete[] xin_offsets;
  if(yin_offsets != NULL)delete[] yin_offsets;
  if(zin_locations != NULL)delete[] zin_locations;
  if(x_weights != NULL)delete[] x_weights;
  if(y_weights != NULL)delete[] y_weights;
  if(z_weights != NULL)delete[] z_weights;
}

#define COMBINE3FLAGS(flag1,flag2,flag3)\
  (((flag1)<<16) | ((flag2)<<8) | ((flag3)))

void psyresamplexyz::fillpage(char *buff, int z, int i)
{
  switch(COMBINE3FLAGS(xin_offsets!=NULL, yin_offsets!=NULL,
		       z_weights[z] != 1)) {
  case COMBINE3FLAGS(0,0,0):
// no interpolation
    psypgbuff::fillpage(buff, z, i);
    break;
  case COMBINE3FLAGS(0,0,1):
// interpolate z only
    {
      if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
      double factor1 = z_weights[z];
      double factor2 = 1.0 - factor1;;
      int lowerplane = zin_locations[z];
      int upperplane = lowerplane + 1;
      char *yoptr=buff;
      char *yiptr1=dualplanebuffer->getzptrlocked(lowerplane,i);
      char *yiptr2=dualplanebuffer->getzptrlocked(upperplane,i);
      for(int y=orig.y; y<=end.y;
	  y++, yoptr+=inc.y, yiptr1+=inc.y, yiptr2+=inc.y) {
	char *xoptr=yoptr;
	char *xiptr1=yiptr1;
	char *xiptr2=yiptr2;
	for(int x=orig.x; x<=end.x; x++, 
	    xoptr+=inc.x, xiptr1+=inc.x, xiptr2+=inc.x) {
	  double in1, in2, out;
	  type2double(xiptr1, type, (char *)&in1);
	  type2double(xiptr2, type, (char *)&in2);
	  out= factor1 * in1 + factor2 * in2;
	  pixeltypechg((char *)&out, psydouble, xoptr, type);
	} // end for x
      } // end for y
      dualplanebuffer->unlockzptr(lowerplane,i);
      dualplanebuffer->unlockzptr(upperplane,i);
    }
    break;
  case COMBINE3FLAGS(1,1,0):
  case COMBINE3FLAGS(1,0,0):
  case COMBINE3FLAGS(0,1,0):
// interpolate x and/or y but not z
    {
    }
    break;
  case COMBINE3FLAGS(1,0,1):
  case COMBINE3FLAGS(0,1,1):
    break;
  case COMBINE3FLAGS(1,1,1):
// interpolate x and/or y and z
    {
      if(dualplanebuffer == NULL)dualplanebuffer=new psypgbuff(inputpsyimg,2);
      double factor_z1 = z_weights[z];
      double factor_z2 = 1.0 - factor_z1;;
      int lowerplane = zin_locations[z];
      int upperplane = lowerplane + 1;
      char *yoptr=buff;
      char *yiptr_z1=dualplanebuffer->getzptrlocked(lowerplane,i);
      char *yiptr_z2=dualplanebuffer->getzptrlocked(upperplane,i);
      for(int y=0; y<size.y; y++, yoptr+=inc.y) {
	double factor_y1 = y_weights[y];
	double factor_y2 = 1-factor_y1;
	char *xoptr=yoptr;
	char *xiptr_z1_y1=yiptr_z1 + yin_offsets[y];
	char *xiptr_z1_y2=xiptr_z1_y1 + inc.y;
	char *xiptr_z2_y1=yiptr_z2 + yin_offsets[y];
	char *xiptr_z2_y2=xiptr_z2_y1 + inc.y;
	for(int x=0; x<size.x; x++, 
	    xoptr+=inc.x) {
	  double factor_x1 = x_weights[x];
	  double factor_x2 = 1-factor_x1;
	  double in_z1_y1_x1, in_z1_y1_x2, in_z1_y2_x1, in_z1_y2_x2;
	  double in_z2_y1_x1, in_z2_y1_x2, in_z2_y2_x1, in_z2_y2_x2;
	  double out;
	  char *ptr=xiptr_z1_y1+xin_offsets[x];
	  type2double(ptr, type, (char *)&in_z1_y1_x1);
	  type2double(ptr+inc.x, type, (char *)&in_z1_y1_x2);
	  ptr=xiptr_z1_y2 + xin_offsets[x];
	  type2double(ptr, type, (char *)&in_z1_y2_x1);
	  type2double(ptr+inc.x, type, (char *)&in_z1_y2_x2);
	  ptr=xiptr_z2_y1+xin_offsets[x];
	  type2double(ptr, type, (char *)&in_z2_y1_x1);
	  type2double(ptr+inc.x, type, (char *)&in_z2_y1_x2);
	  ptr=xiptr_z2_y2+xin_offsets[x];
	  type2double(ptr, type, (char *)&in_z2_y2_x1);
	  type2double(ptr+inc.x, type, (char *)&in_z2_y2_x2);
	  out= factor_z1*(factor_y1*
			  (factor_x1*in_z1_y1_x1+factor_x2*in_z1_y1_x2) +
			  factor_y2*
			  (factor_x1*in_z1_y2_x1+factor_x2*in_z1_y2_x2)
			  ) + 
	       factor_z2*(factor_y1*
			  (factor_x1*in_z2_y1_x1+factor_x2*in_z2_y1_x2) +
			  factor_y2*
			  (factor_x1*in_z2_y2_x1+factor_x2*in_z2_y2_x2)
			  );
	  pixeltypechg((char *)&out, psydouble, xoptr, type);
	} // end for x
      } // end for y
      dualplanebuffer->unlockzptr(lowerplane,i);
      dualplanebuffer->unlockzptr(upperplane,i);
    }
    break;
  }
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  psyimg *psyimgptr;
  psydims outdim;
  outdim.x=outdim.y=outdim.z=outdim.i=-1;
  psyres outres;
  outres.x=outres.y=outres.z=outres.i=0;
  xyzidouble factors;
  factors.x=factors.y=factors.z=factors.i=0;
  psytype outtype=psynotype; //defaults to input type
  psyfileclass outfileclass=psynoclass; //defaults to in file format
  int *outfileclassinfo = NULL;
  int interpolate=0;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-interpolate]"<<'\n';
      cout <<"       [-xf x_factor | -xd x_outdim | -xr x_outres]\n";
      cout <<"       [-yf y_factor | -yd y_outdim | -yr y_outres]\n";
      cout <<"       [-zf z_factor | -zd z_outdim | -zr z_outres]\n";
      cout <<"       [-if i_factor | -id i_outdim | -ir i_outres]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i], "-interpolate")==0)interpolate=1;
    else if((strcmp(argv[i], "-xd")==0) && ((i+1)<argc))
      outdim.x=atoi(argv[++i]);
    else if((strcmp(argv[i], "-yd")==0) && ((i+1)<argc))
      outdim.y=atoi(argv[++i]);
    else if((strcmp(argv[i], "-zd")==0) && ((i+1)<argc))
      outdim.z=atoi(argv[++i]);
    else if((strcmp(argv[i], "-id")==0) && ((i+1)<argc))
      outdim.i=atoi(argv[++i]);
    else if((strcmp(argv[i], "-xf")==0) && ((i+1)<argc))
      factors.x=atof(argv[++i]);
    else if((strcmp(argv[i], "-yf")==0) && ((i+1)<argc))
      factors.y=atof(argv[++i]);
    else if((strcmp(argv[i], "-zf")==0) && ((i+1)<argc))
      factors.z=atof(argv[++i]);
    else if((strcmp(argv[i], "-zi")==0) && ((i+1)<argc))
      factors.i=atof(argv[++i]);
    else if((strcmp(argv[i], "-xr")==0) && ((i+1)<argc))
      outres.x=atof(argv[++i]);
    else if((strcmp(argv[i], "-yr")==0) && ((i+1)<argc))
      outres.y=atof(argv[++i]);
    else if((strcmp(argv[i], "-zr")==0) && ((i+1)<argc))
      outres.z=atof(argv[++i]);
    else if((strcmp(argv[i], "-ir")==0) && ((i+1)<argc))
      outres.i=atof(argv[++i]);
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
  psypgbuff buffered(psyimgptr, 2);
  psyimgptr = (psyimg *)&buffered;

  psyres resolution=psyimgptr->getres();
  psydims dims=psyimgptr->getsize();

//set correct factors
  if(fabs(outres.x) > SMALLEST_RES) {
    if(fabs(resolution.x) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file x resolution too small for factor calculate\n";
      exit(1);
    }
    factors.x = outres.x/resolution.x;
  }
  else if(outdim.x > 0)
    factors.x = (double)dims.x / ((double)outdim.x - .5); //-.5 allows rounding
//    factors.x = ((double)outdim.x - .5)/(double)dims.x; //-.5 allows rounding

  if(fabs(outres.y) > SMALLEST_RES) {
    if(fabs(resolution.y) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file y resolution too small for factor calculate\n";
      exit(1);
    }
    factors.y = outres.y/resolution.y;
  }
  else if(outdim.y > 0)
    factors.y = (double)dims.y/((double)outdim.y - .5); //-.5 allows rounding
//   factors.y = ((double)outdim.y - .5)/(double)dims.y; //-.5 allows rounding

  if(fabs(outres.z) > SMALLEST_RES) {
    if(fabs(resolution.z) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file z resolution too small for factor calculate\n";
      exit(1);
    }
    factors.z = outres.z/resolution.z;
  }
  else if(outdim.z > 0)
    factors.z = (double)dims.z/((double)outdim.z - .5); //-.5 allows rounding
//    factors.z = ((double)outdim.z - .5)/(double)dims.z; //-.5 allows rounding

  if(fabs(outres.i) > SMALLEST_RES) {
    if(fabs(resolution.i) < SMALLEST_RES) {
      cerr << argv[0]
	<< ":input file i resolution too small for factor calculate\n";
      exit(1);
    }
    factors.i = outres.i/resolution.i;
  }
  else if(outdim.i > 0)
    factors.i = ((double)outdim.i - .5)/(double)dims.i; //-.5 allows rounding

  cout<<"scale factors=("<<factors.x<<','<<factors.y<<','<<factors.z
    <<','<<factors.i<<")\n";

  psyimg *resampledimg;
  if(interpolate) {
    if(factors.x != 0 || factors.y != 0 || factors.i != 0) {
      cerr << argv[0] << ": interpolation only implemented for z\n";
      exit(1);
    }
// resample z planes
    resampledimg = new psyresamplexyz(psyimgptr, (xyzdouble)factors);
  }
  else resampledimg = new psyresample(psyimgptr, factors);
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
