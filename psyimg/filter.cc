#include "psyhdr.h"

pixelavg::pixelavg()
{
  boxsize.x=boxsize.y=boxsize.z=0;
  startbox=endbox=boxsize;
  psypagebuffer=NULL;
  zplane=NULL;
  zptr=NULL;
  factor=1.0;
}

pixelavg::pixelavg(psyimg *psyimgptr, psytype pixeltype,
		   int incubesize, int sizey, int sizez,
		   double factor)
     : psyimglnkpxl(psyimgptr, pixeltype)
{
  if(incubesize < 1) {
    output_tree(&cerr);
    cerr<<"pixelavg::pixelavg - invalid cube size="<<incubesize<<'\n';
    exit(1);
  }
  if(sizey < 1)sizey=incubesize;
  if(sizez < 1)sizez=incubesize;
  boxsize.x=incubesize;
  boxsize.y=sizey;
  boxsize.z=sizez;

  if(boxsize.x%2)startbox.x = -(boxsize.x-1)/2;//odd size symetrical
  else startbox.x = -boxsize.x/2 + 1;  // even size shifted one

  if(boxsize.y%2)startbox.y = -(boxsize.y-1)/2;//odd size symetrical
  else startbox.y = -boxsize.y/2 + 1;  // even size shifted

  if(boxsize.z%2)startbox.z = -(boxsize.z-1)/2;//odd size symetrical
  else startbox.z = -boxsize.z/2 + 1;  // even size shifted

  endbox=startbox + boxsize + (-1);

  pixelavg::factor=factor;
  psypagebuffer=new psypgbuff(psyimgptr, boxsize.z);
  zplane = new int [boxsize.z];
  zptr = new char *[boxsize.z];
}

pixelavg::~pixelavg()
{
  if(psypagebuffer != NULL)delete psypagebuffer;
  if(zplane != NULL)delete[] zplane;
  if(zptr != NULL)delete[] zptr;
}

void pixelavg::copyblock(char *outbuff, int xorig, int yorig, int zorig,
		       int iorig, int xend, int yend, int zend, int iend,
		       int out_xinc, int out_yinc,
		       int out_zinc, int out_iinc,
		       psytype pixeltype)
{
  int x, y, z, i;
  int xf, yf;
  int j, jc;
  char *xoptr, *yoptr, *zoptr, *ioptr;
  int inneroffsetx, inneroffsety;
  int origoffset;
  double dpixel;
  int xstart, xstop, ystart, ystop;
  int xstep, ystep;
  double sum;
  int count;

  if(!inside(xorig, yorig, zorig, iorig) || 
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr << ":pixelavg::copyblock - request outside of buffer\n";
    exit(1);
  }
// get x step used on inner loop
//  xstep=psypagebuffer->inc.x;
//  ystep=psypagebuffer->inc.y;
  psypagebuffer->getinc(&xstep, &ystep, NULL, NULL);
// get origin offset into each plane
  origoffset=psypagebuffer->offset(orig.x, orig.y, orig.z, orig.i);
  ioptr=outbuff;
  for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
// initialize first set of zplane buffers
    for(j=0, jc=startbox.z; j<boxsize.z; j++, jc++) {
      zplane[j] = clip(zorig+jc, orig.z, end.z);//may add planes twice
      zptr[j] = psypagebuffer->getzptrlocked(zplane[j], i);
    }
    for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
      for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	ystart=clip(y+startbox.y,orig.y,end.y);
	ystop=clip(y+endbox.y,orig.y,end.y);
	for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
	  xstart=clip(x+startbox.x,orig.x,end.x);
	  xstop=clip(x+endbox.x,orig.x,end.x);
// start inner loop accumulation
	  sum=0; count=0;
          inneroffsety=psypagebuffer->offset(xstart,ystart,orig.z,orig.i)
	    - origoffset;
	  for(yf=ystart; yf<=ystop; yf++) {
	    inneroffsetx=inneroffsety;
	    for(xf=xstart; xf<=xstop; xf++) {
// accumulate over planes
              for(j=0; j<boxsize.z; j++) {
		type2double(zptr[j]+inneroffsetx, psypagebuffer->gettype(),
			    (char *)&dpixel);
		sum+=dpixel;
		count++;
	      }
	      inneroffsetx+=xstep;
	    }
	    inneroffsety+=ystep;
	  }
	  sum /= count;
	  sum *= factor;
	  pixeltypechg((char *)&sum, psydouble, xoptr, pixeltype);
	}//end for(x=
      }//end for(y=
// shift to next set of zplanes
      psypagebuffer->unlockzptr(zplane[0], i);
      for(j=1; j<boxsize.z; j++)zplane[j-1]=zplane[j];
      j=boxsize.z-1;
      zplane[j]=clip(zplane[j]+1, orig.z, end.z);//plane may add more then once
      zptr[j] = psypagebuffer->getzptrlocked(zplane[j], i)+origoffset;
    }//end for(z=
// unlock z page buffers
    for(j=0; j<boxsize.z; j++) {
      psypagebuffer->unlockzptr(zplane[j], i);
    }
  }//end for(i=
}

void pixelavg::getnextpixel(char *pixel)
{
  xyzint start=(xyzint)getpixelloc + startbox;
  start.x=clip(start.x,orig.x,end.x);
  start.y=clip(start.y,orig.y,end.y);

  xyzint stop=(xyzint)getpixelloc + endbox;
  stop.x=clip(stop.x,orig.x,end.x);
  stop.y=clip(stop.y,orig.y,end.y);

  int i=getpixelloc.i;
  psytype psypagebuffertype=psypagebuffer->gettype();

  double sum=0;
  int count=0;

  for(int z=start.z; z<=stop.z; z++) {
    int lz=clip(z, orig.z, end.z); //plane may add more then once
    for(int y=start.y; y<=stop.y; y++) {
      for(int x=start.x; x<=stop.x; x++) {
	char_or_largest_pixel inpixel;
	psypagebuffer->getpixel(inpixel.c, psypagebuffertype, x, y, lz, i);

	double dpixel;
	type2double(inpixel.c, psypagebuffertype, (char *)&dpixel);

	sum += dpixel;
	count++;
      }
    }
  }

  sum /= count;
  sum *= factor;
  pixeltypechg((char *)&sum, psydouble, pixel, getpixeltype);
  incgetpixel();
}

gradient::gradient()
{
  gradtype=no_gradient;
  gradsize.x=gradsize.y=gradsize.z=0;
  prevoffset=nextoffset=gradsize;
  factor=0;
  psypagebuffer=NULL;
}

gradient::gradient(psyimg *psyimgptr, gradienttype gradtype,
		   psytype pixeltype, double factor)
: psyimglnkpxl(psyimgptr, pixeltype)
{
  gradient::gradtype=gradtype;
  gradient::factor=factor;

  gradsize.x=gradsize.y=gradsize.z=1;
  switch (gradtype) {
  case x_gradient:
  case mag_of_x_gradient:
    gradsize.x=3;
    break;
  case y_gradient:
  case mag_of_y_gradient:
    gradsize.y=3;
    break;
  case z_gradient:
  case mag_of_z_gradient:
    gradsize.z=3;
    break;
    output_tree(&cerr);
  case mag_of_xy_gradient:
  case max_mag_of_xy_gradient:
    gradsize.x=gradsize.y=3;
    break;
  case mag_of_xyz_gradient:
  case max_mag_of_xyz_gradient:
    gradsize.x=gradsize.y=gradsize.z=3;
    break;
  default:
    gradsize.x=gradsize.y=gradsize.z=1;
    break;
  }
  if(gradsize.x%2)prevoffset.x = -(gradsize.x-1)/2;//odd size symetrical
  else prevoffset.x = -gradsize.x/2 + 1;  // even size shifted one
  if(gradsize.y%2)prevoffset.y = -(gradsize.y-1)/2;//odd size symetrical
  else prevoffset.y = -gradsize.y/2 + 1;  // even size shifted
  if(gradsize.z%2)prevoffset.z = -(gradsize.z-1)/2;//odd size symetrical
  else prevoffset.z = -gradsize.z/2 + 1;  // even size shifted
  nextoffset=prevoffset + gradsize + (-1);

  psypagebuffer=new psypgbuff(psyimgptr, gradsize.z);
}

gradient::~gradient()
{
  if(psypagebuffer != NULL)delete psypagebuffer;
}

void gradient::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			 int iorig, int xend, int yend, int zend, int iend,
			 int out_xinc, int out_yinc,
			 int out_zinc, int out_iinc,
			 psytype pixeltype)
{
  if(!inside(xorig, yorig, zorig, iorig) || 
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr << "::copyblock - request outside of buffer\n";
    exit(1);
  }

// get origin offset into each plane
  int origoffset=psypagebuffer->offset(orig.x, orig.y, orig.z, orig.i);

// get pagebuffer type
  psytype psypagebuffertype=psypagebuffer->gettype();

  char *ioptr=outbuff;
  for(int i=iorig; i<=iend; i++, ioptr+=out_iinc) {

// initialize first set of zplane buffers
    char *zoptr=ioptr;
    for(int z=zorig; z<=zend; z++, zoptr+=out_zinc) {
      int z_prev=clip(zorig+prevoffset.z, orig.z, end.z);
      char *prev_zptr = psypagebuffer->getzptrlocked(z_prev, i);
      char *zptr = psypagebuffer->getzptrlocked(z, i);
      int z_next=clip(zorig+nextoffset.z, orig.z, end.z);
      char *next_zptr = psypagebuffer->getzptrlocked(z_next, i);

      char *yoptr=zoptr;
      for(int y=yorig; y<=yend; y++, yoptr+=out_yinc) {
	int y_prev=clip(y+prevoffset.y,orig.y,end.y);
	int y_next=clip(y+nextoffset.y,orig.y,end.y);

	char *xoptr=yoptr;
	for(int x=xorig; x<=xend; x++, xoptr+=out_xinc) {
// start inner calcs
	  int x_prev=clip(x+prevoffset.x,orig.x,end.x);
	  int x_next=clip(x+nextoffset.x,orig.x,end.x);

	  double dprev, dnext;
	  int inneroffset;
// x diff
	  double diffx;
	  if(gradsize.x > 1){
	    inneroffset=psypagebuffer->offset(x_prev,y,orig.z,orig.i)
	      - origoffset;
	    type2double(zptr+inneroffset, psypagebuffertype,
			(char *)&dprev);
	    inneroffset=psypagebuffer->offset(x_next,y,orig.z,orig.i)
	      - origoffset;
	    type2double(zptr+inneroffset, psypagebuffertype,
			(char *)&dnext);
	    diffx=dprev-dnext;
	  }
	  else diffx=0;
// y diff
	  double diffy;
	  if(gradsize.y > 1){
	    inneroffset=psypagebuffer->offset(x,y_prev,orig.z,orig.i)
	      - origoffset;
	    type2double(zptr+inneroffset, psypagebuffertype,
			(char *)&dprev);
	    inneroffset=psypagebuffer->offset(x,y_next,orig.z,orig.i)
	      - origoffset;
	    type2double(zptr+inneroffset, psypagebuffertype,
			(char *)&dnext);
	    diffy=dprev-dnext;
	  }
	  else diffy=0;
// z diff
	  double diffz;
	  if(gradsize.z > 1){
	    inneroffset=psypagebuffer->offset(x,y,orig.z,orig.i)
	      - origoffset;
	    type2double(prev_zptr+inneroffset, psypagebuffertype,
			(char *)&dprev);
	    type2double(next_zptr+inneroffset, psypagebuffertype,
			(char *)&dnext);
	    diffz=dprev-dnext;
	  }
	  else diffz=0;
// calculate appropriate values
	  double dpixel;
	  switch (gradtype) {
	  case x_gradient:
	    dpixel=diffx;
	    break;
	  case y_gradient:
	    dpixel=diffy;
	    break;
	  case z_gradient:
	    dpixel=diffz;
	    break;
	  case mag_of_x_gradient:
	    dpixel=fabs(diffx);
	    break;
	  case mag_of_y_gradient:
	    dpixel=fabs(diffy);
	    break;
	  case mag_of_z_gradient:
	    dpixel=fabs(diffz);
	    break;
	  case mag_of_xy_gradient:
	    dpixel=sqrt((diffx*diffx)+(diffy*diffy));
	    break;
	  case mag_of_xyz_gradient:
	    dpixel=sqrt((diffx*diffx)+(diffy*diffy)+(diffz*diffz));
	    break;
	  case max_mag_of_xy_gradient:
	    dpixel=max(fabs(diffx),fabs(diffy));
	    break;
	  case max_mag_of_xyz_gradient:
	    dpixel=max(fabs(diffx),fabs(diffy));
	    dpixel=max(dpixel,fabs(diffz));
	    break;
	  case no_gradient:
	  default:
	    dpixel=0;
	    break;
	  }
// finish inner calcs setting pixel value
	  dpixel *= factor;
	  pixeltypechg((char *)&dpixel, psydouble, xoptr, pixeltype);
	}//end for(x=
      }//end for(y=
// unlock z page buffers
      psypagebuffer->unlockzptr(z_prev, i);
      psypagebuffer->unlockzptr(z, i);
      psypagebuffer->unlockzptr(z_next, i);
    }//end for(z=
  }//end for(i=
}

void gradient::getnextpixel(char *pixel)
{
  output_tree(&cerr);
  cerr << "::getnextpixel - sorry, not implemented\n";
  exit(1);
}
