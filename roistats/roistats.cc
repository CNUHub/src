#include "psyhdr.h"
#include <stdio.h>

/* Sort array a[] in place with values in dims[] also swapped to maintain
 same relative order */
// 5/2/2018 renamed to not confuse with shellsort in psytools.cc
void shellsort_w_pydims(unsigned int n, double a[], psydims dims[]) {
  // n = (input) size of arrays a[] and dims[]
  // a[] = (input/output) array to sort in place
  // dims[] = (input/output) array to order in place with the same
  //                      re-ordering as a[]
  unsigned int inc = 1;
  do {
    inc *= 3;
    inc ++;
  } while (inc <= n);
  do {
    inc /= 3;
    for( unsigned int i=inc; i<n; i++) {
      double v = a[i];
      psydims w = dims[i];
      unsigned int j = i;
      while (a[j - inc] > v) {
	a[j] = a[j-inc];
	dims[j] = dims[j-inc];
	j -= inc;
	if ((j+1) <= inc) break;
      }
      a[j] = v;
      dims[j] = w;
    }
  } while (inc > 1);
}


class topNtemplate : public psyimg {
  protected:
    int topN;
    psydims *topLocations;
    psydims getpixelloc;
    psytype getpixeltype;
    int getnextpixel_lock;
  public:
    topNtemplate(psyimg *psyimgptr, int n, psyimg *templateptr=NULL);
    void inittopNtemplate(psyimg *psyimgptr, int n, psyimg *templateptr=NULL);
    void output_tree(ostream *out) {psyimg::output_tree(out); *out<<"::topNtemplate";};
    void getpixel(char *pixel, psytype pixeltype,
			int x, int y, int z, int i);
    void initgetpixel(psytype pixeltype, int x, int y, int z, int i);
    void incgetpixel();
    void freegetpixel();
    void getnextpixel(char *pixel);
};

topNtemplate::topNtemplate(psyimg *psyimgptr, int n, psyimg *templateptr) : psyimg() {
  inittopNtemplate(psyimgptr, n, templateptr);
}

void topNtemplate::inittopNtemplate(psyimg *psyimgptr, int n, psyimg *templateptr) {
  int xdim, ydim, zdim, idim;
  psydims orig;
  double xres, yres, zres, ires;
// set psyimg values to same as input image
  if(psyimgptr != NULL) {
    psyimgptr->getsize(&xdim, &ydim, &zdim, &idim);
    orig=psyimgptr->getorig();
    psyimgptr->getres(&xres, &yres, &zres, &ires);
  }
  else {
    xdim=ydim=zdim=idim=0;
    orig.x=orig.y=orig.z=orig.i=0;
    xres=yres=zres=ires=1;
  }
  initpsyimg(xdim, ydim, zdim, idim,
	     psyuchar,
	     orig.x, orig.y, orig.z, orig.i, 0,
	     xres, yres, zres, ires,
	     psyimgptr->getwordres());
  int numwords = size.x * size.y * size.z * size.i;
  topN=n;
  if(topN < 1 || topN > numwords) {
	topN = 0; return;
  }
  topLocations = new psydims[topN];
  // fill topLocations with location of topN max values
  double topValues[topN];
  psytype imagetype=psyimgptr->gettype();
  psytype templatetype = psynotype;

  double max, min;
  int ncount = 0;
  psydims location;
  // loop through input image
  psyimgptr->initgetpixel(psydouble, orig.x, orig.y, orig.z, orig.i);
  if(templateptr != NULL) { 
    templatetype = templateptr->gettype();
    templateptr->initgetpixel(psyint, orig.x, orig.y, orig.z, orig.i);
  }

  double dpixel;
  int tpixel=1;
  for(location.z=orig.z; location.z<=end.z; location.z++) {
    for(location.y=orig.y; location.y<=end.y; location.y++) {
      for(location.x=orig.x; location.x<=end.x; location.x++) {
        psyimgptr->getnextpixel((char *)&dpixel); // always get next pixel even if off template
        if(templateptr != NULL) templateptr->getnextpixel((char*)&tpixel);
        if(! tpixel) ; // ignore pixels off template
        else if(ncount < topN) {
          if(ncount == 0) {
	    min=dpixel; max=dpixel;
          }
	  else if(dpixel < min) min=dpixel;
	  else if(dpixel > max) max=dpixel;

	  topValues[ncount] = dpixel;
	  topLocations[ncount]=location;
	  ncount++;
	  if(ncount == topN) shellsort_w_pydims(topN, topValues, topLocations);
        }
	else if(dpixel > topValues[0]) {
	    // insert pixel into array dropping smallest
	    topValues[0] = dpixel;
	    topLocations[0]=location;
            // shift it up to sorted location
	    for(int nn = 1; nn < topN; nn++) {
	      if(dpixel > topValues[nn]) {
		topValues[nn-1] = topValues[nn];
		topLocations[nn-1]=topLocations[nn];
	        topValues[nn]=dpixel;
	        topLocations[nn]=location;
	      }
	      else break;
	    } // end for(int nn...)
	 } // end else if(dpix...)
      } // end for(location.x=...)
    }// end for(location.y=...)
  } // end for(location.z=...)
  psyimgptr->freegetpixel();
  if(templateptr != NULL) templateptr->freegetpixel();
  // if less then topN voxels found reduce topN to reduce getpixel checks and not have to initialize remaining locations
  if(ncount < topN) topN=ncount;

} // end initpsyimg

void topNtemplate::getpixel(char *pixel, psytype pixeltype,
			int x, int y, int z, int i){
  int ontemplate=0;
  for(int j=0; j<topN; j++) {
    if(topLocations[j].x == x && topLocations[j].y == y && topLocations[j].z == z && topLocations[j].i == i) {
      ontemplate=1; break;
    }
  }
  pixeltypechg((char *) &ontemplate, psyint, pixel, pixeltype);
}

void topNtemplate::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  if(!inside(x, y, z, i)) {
    output_tree(&cerr);
    cerr << "::initgetpixel - request("<<x<<','<<y<<','<<z<<','<<i;
    cerr <<") outside of image\n";
    exit(1);
  }
  if(getnextpixel_lock) {
    output_tree(&cerr);
    cerr << "::initgetpixel - programming error - get pixel locked\n";
    cerr << "add a buffer or check for missing call to freegetpixel\n";
    exit(1);
  }
  getpixeltype=pixeltype;
  getpixelloc.x=x; getpixelloc.y=y; getpixelloc.z=z; getpixelloc.i=i;
  getnextpixel_lock=1;
}

void topNtemplate::getnextpixel(char *pixel)
{
// no checking assumes caller knows extents of buffer and called initgetpixel
  getpixel(pixel,getpixeltype,getpixelloc.x,getpixelloc.y,getpixelloc.z,getpixelloc.i);
  incgetpixel();
}

void topNtemplate::incgetpixel()
{
  getpixelloc.x++;
  if(getpixelloc.x > end.x) {
    getpixelloc.x=orig.x;
    getpixelloc.y++;
    if(getpixelloc.y > end.y) {
      getpixelloc.y=orig.y;
      getpixelloc.z++;
      if(getpixelloc.z > end.z) {
	getpixelloc.z=orig.z;
	getpixelloc.i++;
	if(getpixelloc.i > end.i)getpixelloc.i=orig.i;
     }  //end if(getpixelloc.z...)
    } //end if(getpixelloc.y...)
  } //end if(getpixelloc.x...)
}

void topNtemplate::freegetpixel() {
  getnextpixel_lock=0;
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean, sum, sqrsum;
  int pixelsused;
  psytype outtype=psynotype;
  psyfileclass outfileclass=psynoclass;
  int *outfileclassinfo = NULL;
  char *template_file=NULL;

  int or_templates=0;

  xyzdouble center;
  double radius;
  xyzdouble length;
  psydims box_orig, box_end;
  int irangetemplateneeded = 0;
  int ibegin = -1;
  int iend = -1;
  int single_pixel=0;
  double threshold_percent=40;
  double threshold=0;

  psyimg *templateptr=NULL;
  psyimg *templateptrtmp=NULL;

  psyimg *template_img_buff_ptr=NULL;

  int quantify=0;

  int prtmin=-1, prtmax=-1, prtmean=-1;
  int prtminloc=0, prtmaxloc=0;
  int prtpixelsused=0, prtsum=0, prtsqrsum=0;
  int prtvariance=0, prtstddev=0;
  int prtcm = 0;
  int prtadev=0;
  int prttext=1;
  int prtscientific=0;  // jtlee added 02/23/2015
  int prtprecision=0; // jtlee added 02/23/2015
  int tindices[argc - 1];
  int nt=0;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" infile [-q] [-o templated_outfile]\n";
      cout <<"      [ -or ]\n";
      cout <<"      [ -b xorig,yorig,zorig,xend,yend,zend ]\n";
      cout <<"      [ -bc xcenter,ycenter,zcenter,xlength,ylength,zlength ]\n";
      cout <<"      [ -c xcenter,ycenter,zcenter,radius ]\n";
      cout <<"      [ -e xcenter,ycenter,zcenter,xlength,ylength,zlength ]\n";
      cout <<"      [ -irange ibegin,iend ]\n";
      cout <<"      [ -tf template_file ]\n";
      cout <<"      [ -ttfn threshold,template_file ]\n";
      cout <<"      [ -tmmtfn minthresh,maxthresh,template_file ]\n";
      cout <<"      [ -tp threshold_percent ] [ -tpn threshold_percent ]\n";
      cout <<"      [ -t threshold ] [ -tn threshold ]"<<'\n';
      cout <<"      [ -topn N] [-bottomn N]\n";
      cout <<"      [-min][-minloc][-max][-maxloc]\n";
      cout <<"      [-mean][-npixels][-sum][-sumsqr]\n";
      cout <<"      [-var][-stddev][-adev][-cm]\n";
      cout <<"      [-notext][-scientific][-precision digits]\n"; // jtlee added scientific/precision on 02/23/2015
      cout <<"      ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"      ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-q")==0)quantify=1;
    else if(strcmp(argv[i],"-or")==0)or_templates=1;
    else if(strcmp(argv[i],"-min")==0)prtmin=1;
    else if(strcmp(argv[i],"-max")==0)prtmax=1;
    else if(strcmp(argv[i],"-minloc")==0)prtminloc=1;
    else if(strcmp(argv[i],"-maxloc")==0)prtmaxloc=1;
    else if(strcmp(argv[i],"-mean")==0)prtmean=1;
    else if(strcmp(argv[i],"-npixels")==0)prtpixelsused=1;
    else if(strcmp(argv[i],"-sum")==0)prtsum=1;
    else if(strcmp(argv[i],"-sumsqr")==0)prtsqrsum=1;
    else if(strcmp(argv[i],"-var")==0)prtvariance=1;
    else if(strcmp(argv[i],"-stddev")==0)prtstddev=1;
    else if(strcmp(argv[i],"-adev")==0)prtadev=1;
    else if(strcmp(argv[i],"-cm")==0)prtcm=1;
    else if(strcmp(argv[i],"-scientific")==0)prtscientific=1;  // jtlee added 02/23/2015
    else if((strcmp(argv[i],"-precision")==0)&&((i+1)<argc)) {  // jtlee added 02/23/2015
      if(sscanf(argv[++i],"%ld",&prtprecision) != 1) {
	cerr << argv[0] << ": error parsing precision: -precision ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if(strcmp(argv[i],"-notext")==0)prttext=0;
    else if((strcmp(argv[i],"-b")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-bc")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-c")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-e")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tf")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-ttfn")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tmmtfn")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tp")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-t")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tpn")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tn")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-topn")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-bottomn")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-irange")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%d,%d", &ibegin, &iend) != 2) {
	cerr << argv[0] << ": error parsing i origin and end: ";
	cerr << argv[i-1] << " " << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-o")==0)&&((i+1)<argc)) outfile=argv[++i];
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }

// reset default print options if no print requested
  if(prtmin<=0 && prtmax<=0 && prtminloc<=0 && prtmaxloc<=0 &&
     prtmean<=0 && prtpixelsused<=0 && prtsum<=0 && prtsqrsum<=0 &&
     prtvariance<=0 && prtstddev<=0 && prtadev<=0) {
    prtmin=prtmax=prtminloc=prtmaxloc=prtmean=1;
    prtpixelsused=prtsum=prtsqrsum=1;
    prtvariance=prtstddev=prtadev=1;
    prtcm=1;
  }

// set the default number format
  if(prtscientific) cout<<std::scientific; // jtlee added 02/23/2015
  if(prtprecision) cout<<setprecision(prtprecision); // jtlee added 02/23/2015

// open input files
//  analyzefile inimag(infile, "r");
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outfileclass==psynoclass)outfileclass=infileclass;
  if(outtype==psynotype)outtype=intype;

// buffer the input
  psypgbuff inimagbuff(inimag, 1);
  psydims orig = inimagbuff.getorig();
// check i range
  psydims end = inimagbuff.getend();
  ibegin = clip(ibegin, orig.i, end.i);
  if(iend < 0) iend = end.i;
  else if(iend < orig.i) iend = orig.i;
  else if(iend > end.i) iend = end.i;
  if( (ibegin != orig.i) || (iend != end.i) ) irangetemplateneeded = 1;

// And/or templates together
  for(int t=0; t<nt; t++) {
    if((strcmp(argv[tindices[t]],"-b")==0) ||
       (strcmp(argv[tindices[t]],"-bc")==0)) {
      if(strcmp(argv[tindices[t]],"-b")==0) {
	if(sscanf(argv[tindices[t]+1],"%d,%d,%d,%d,%d,%d",
		  &box_orig.x, &box_orig.y, &box_orig.z,
		  &box_end.x, &box_end.y, &box_end.z) != 6) {
	  cerr << argv[0] << ": error parsing box origin and end: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
      }
      else { // -bc
	if(sscanf(argv[tindices[t]+1],"%lf,%lf,%lf,%lf,%lf,%lf",
		  &center.x, &center.y, &center.z,
		  &length.x, &length.y, &length.z) != 6) {
	  cerr << argv[0] << ": error parsing box center and lengths: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
// calculate box region
	xyzint bo=xyzdouble2int((center - (length * 0.5)) + 0.5);
	xyzint be= bo + xyzdouble2int(length);
	box_orig.x = bo.x; box_orig.y = bo.y; box_orig.z = bo.z; box_orig.i = 0;
	box_end.x = be.x; box_end.y = be.y; box_end.z = be.z; box_end.i = 0;
// the following started failing for unknown assigment problems casting xyziint as xyzint
// the assignment left box_orig and box_end completely uninitialized
//	(xyzint)box_orig = xyzdouble2int((center - (length * 0.5)) + 0.5);
//	(xyzint)box_end =  xyzdouble2int((center + (length * 0.5)) + 0.5);
      }
      box_orig.i = ibegin; box_end.i = iend;
      irangetemplateneeded = 0; // box takes care of i templateing
// keep stats box on image
      box_orig.x = clip(box_orig.x, orig.x, end.x);
      box_orig.y = clip(box_orig.y, orig.y, end.y);
      box_orig.z = clip(box_orig.z, orig.z, end.z);
      box_end.x = clip(box_end.x, orig.x, end.x);
      box_end.y = clip(box_end.y, orig.y, end.y);
      box_end.z = clip(box_end.z, orig.z, end.z);
// i clipping done already
//      box_orig.i = clip(box_orig.i, orig.i, end.i);
//      box_end.i = clip(box_orig.i, orig.i,end.i);
// create box template
      templateptrtmp=(psyimg *)
        new psyimgbox((psyimg *)&inimagbuff, box_orig, box_end);
      if((box_orig.x == box_end.x) && (box_orig.y == box_end.y) &&
         (box_orig.z == box_end.z) && (box_orig.i == box_end.i) &&
	 (nt == 1)) {
        single_pixel=1;
        center.x=(double)box_orig.x; center.y=(double)box_orig.y;
        center.z=(double)box_orig.z;
      }
    }
    else if(strcmp(argv[tindices[t]],"-c")==0) {
      if(sscanf(argv[tindices[t]+1],"%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z, &radius) != 4) {
	cerr << argv[0] << ": error parsing sphere center and radius: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
      templateptrtmp=(psyimg *)
	new psyimgsphere((psyimg *)&inimagbuff, radius,
			 center.x, center.y, center.z);
      ((psyimgsphere *)templateptrtmp)->setirange(ibegin, iend);
      irangetemplateneeded = 0; // sphere takes care of i templateing
      if((fabs(radius) <= 1e-20) && (ibegin == iend) && (nt == 1))single_pixel=1;
    }
    else if(strcmp(argv[tindices[t]],"-e")==0) {
      if(sscanf(argv[tindices[t]+1],"%lf,%lf,%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z,
		&length.x, &length.y, &length.z) != 6) {
	cerr << argv[0] << ": error parsing ellipsoid center and lengths: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
      templateptrtmp=(psyimg *)
	new psyimgellipsoid((psyimg *)&inimagbuff, center, length);
      ((psyimgellipsoid *)templateptrtmp)->setirange(ibegin, iend);
      irangetemplateneeded = 0; // ellipsoid takes care of i templateing
      if((fabs(length.x) <= 1e-20) && (fabs(length.y) <= 1e-20) &&
	 (fabs(length.z) <= 1e-20) && (ibegin == iend) &&
	 (nt == 1))single_pixel=1;
    }
    else if(strcmp(argv[tindices[t]],"-tf")==0) {
      template_file=argv[tindices[t]+1];
// read the template
      templateptrtmp = psynewinfile(template_file);
      templateptrtmp = (psyimg *)new psypgbuff(templateptrtmp,1);
    }
    else if(strcmp(argv[tindices[t]],"-ttfn")==0) {
      template_file = new char[strlen(argv[tindices[t]+1]) + 1];
      if(sscanf(argv[tindices[t]+1],"%lf,%s", &threshold, template_file)
	 != 2) {
	cerr << argv[0] << ": error parsing threshold and file name: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
// read the template file
      templateptrtmp = psynewinfile(template_file);
      templateptrtmp = (psyimg *)new psypgbuff(templateptrtmp,1);
// recreate the template based on threshold with no filling
      scaleimg *scalethresholdimg =
	new scaleimg(templateptrtmp, outtype);
      // template pixels >= threshold set to 1
      // and pixels < threshold set to 0
      // Note - scaleimg compares min thresh first so
      // all pixels < thresholded set to 0
      scalethresholdimg->set_min_thresh(threshold, 0);
      // reduce the threshold more so that 
      // all pixels not thresholded to 0 are set to 1
      scalethresholdimg->set_max_thresh(threshold-1, 1);
      templateptrtmp = (psyimg *) scalethresholdimg;
    }
    else if(strcmp(argv[tindices[t]],"-tmmtfn")==0) {
      double threshmax = 0;
      template_file = new char[strlen(argv[tindices[t]+1]) + 1];
      if(sscanf(argv[tindices[t]+1],"%lf,%lf,%s", &threshold, &threshmax, template_file)
	 != 3) {
	cerr << argv[0] << ": error parsing min threshold, max threshold and file name: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
// read the template file
      templateptrtmp = psynewinfile(template_file);
      templateptrtmp = (psyimg *)new psypgbuff(templateptrtmp,1);
// recreate the template based on thresholds with no filling
// translation = 1.0-threshold insures values greater then min but less then max will be positive and greater then 1
      scaleimg *scalethresholdimg =
	new scaleimg(templateptrtmp, outtype, 1.0, 1.0-threshold);
      // pixels < threshold set to 0
      scalethresholdimg->set_min_thresh(threshold, 0);
      // pixels greater then max thresh set to 0
      scalethresholdimg->set_max_thresh(threshmax, 0);
      templateptrtmp = (psyimg *) scalethresholdimg;
    }
    else if((strcmp(argv[tindices[t]],"-tp")==0) ||
	    (strcmp(argv[tindices[t]],"-t")==0)) {
      if(strcmp(argv[tindices[t]],"-t")==0) {
	if(sscanf(argv[tindices[t]+1],"%lf",&threshold) != 1) {
	  cerr << argv[0] << ": error parsing threshold: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
      }
      else { // -tp
	if(sscanf(argv[tindices[t]+1],"%lf",&threshold_percent) != 1) {
	  cerr << argv[0] << ": error parsing threshold percent: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
	inimagbuff.getstats(&min, &max, &mean);
	threshold=threshold_percent * .01 * max;
        if(prttext)
          cout<<"threshold="<<threshold<<" ="<<100*threshold/max<<"% of max\n";
      }
// buffer the input again to build the template
      template_img_buff_ptr = (psyimg *) new psypgbuff(inimag, 1);
// build the threshold template filling in x and y directions
      templateptrtmp = (psyimg *)
        new bldtemplate(template_img_buff_ptr, threshold, 1);
    }
    else if(strcmp(argv[tindices[t]],"-topn")==0) {
      int topn = 0;
      if(sscanf(argv[tindices[t]+1],"%d",&topn) != 1) {
	cerr << argv[0] << ": error parsing topn: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
      template_img_buff_ptr = (psyimg *) new psypgbuff(inimag, 1);

      templateptrtmp = (psyimg *) new topNtemplate(template_img_buff_ptr, topn, (or_templates? NULL:templateptr));
      if(! or_templates) {
        // already anded previous templates with topNtemplate
	templateptr=templateptrtmp;
	templateptrtmp=NULL;
      }
    }
    else if(strcmp(argv[tindices[t]],"-bottomn")==0) {
      int bottomn = 0;
      if(sscanf(argv[tindices[t]+1],"%d",&bottomn) != 1) {
	cerr << argv[0] << ": error parsing bottomn: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
      template_img_buff_ptr = (psyimg *) new psypgbuff(inimag, 1);
      psyimg *invertedinput = new scaleimg(template_img_buff_ptr, psydouble, -1.0, 0.0);

      templateptrtmp = (psyimg *) new topNtemplate(invertedinput, bottomn, (or_templates? NULL:templateptr));
      if(! or_templates) {
        // already anded previous templates with topNtemplate
	templateptr=templateptrtmp;
	templateptrtmp=NULL;
      }
    }
    else if((strcmp(argv[tindices[t]],"-tpn")==0) ||
	    (strcmp(argv[tindices[t]],"-tn")==0)) {
      if(strcmp(argv[tindices[t]],"-tn")==0) {
	if(sscanf(argv[tindices[t]+1],"%lf",&threshold) != 1) {
	  cerr << argv[0] << ": error parsing threshold: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
      }
      else { // -tpn
	if(sscanf(argv[tindices[t]+1],"%lf",&threshold_percent) != 1) {
	  cerr << argv[0] << ": error parsing threshold percent: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
	inimagbuff.getstats(&min, &max, &mean);
	threshold=threshold_percent * .01 * max;
        if(prttext)
          cout<<"threshold="<<threshold<<" ="<<100*threshold/max<<"% of max\n";
      }
// buffer the input again to build the template
      template_img_buff_ptr = (psyimg *) new psypgbuff(inimag, 1);
// build the template based on threshold with no filling
      scaleimg *scalethresholdimg =
	new scaleimg(template_img_buff_ptr, outtype);
      // template pixels >= threshold set to 1
      // and pixels < threshold set to 0
      // Note - scaleimg compares min thresh first so
      // all pixels < thresholded set to 0
      scalethresholdimg->set_min_thresh(threshold, 0);
      // reduce the threshold more so that 
      // all pixels not thresholded to 0 are set to 1
      scalethresholdimg->set_max_thresh(threshold-1, 1);
      templateptrtmp = (psyimg *) scalethresholdimg;
    }

    if(templateptr == NULL) templateptr = templateptrtmp;
    else if(templateptrtmp != NULL) {
      if(or_templates) templateptr = (psyimg *) new addimgs(templateptr, templateptrtmp);
      else templateptr = (psyimg *) new templateimg(templateptr, templateptrtmp, 1);
    }
    templateptrtmp=NULL;
  }

  if(irangetemplateneeded) {
// keep stats box on image
    box_orig.x = orig.x; box_orig.y = orig.y; box_orig.z = orig.z;
    box_orig.i = clip(ibegin, orig.i, end.i);
    box_end.x = end.x; box_end.y = end.y; box_end.z = end.z;
    box_end.i = clip(iend, orig.i, end.i);
// create box template
    templateptrtmp=(psyimg *)
      new psyimgbox((psyimg *)&inimagbuff, box_orig, box_end);
    if(templateptr == NULL) templateptr = templateptrtmp;
    else templateptr = (psyimg *) new templateimg(templateptr, templateptrtmp, 1);
  }
  else if(templateptr == NULL) {
// defaults to stats on whole image
      templateptr=(psyimg *) new psyimgconstant((psyimg *)&inimagbuff, 1.0);
  }

// template the image
  templateimg templated((psyimg *)&inimagbuff, templateptr, 1);
// print stats
  double adev, var;
  psydims min_location, max_location;
  xyzidouble center_of_mass;
  if(single_pixel) {
// allow quicker retrieval of single pixel values
    int x=irint(center.x); int y=irint(center.y);
    int z=irint(center.z);
    if((fabs(center.x - (double)x) > 1e-20) ||
       (fabs(center.y - (double)y) > 1e-20) ||
       (fabs(center.z - (double)z) > 1e-20)) {
// to be consistent with templated, set pixelsused to zero if not exactly
// on pixel
      pixelsused=0;
    }
    else {
      char_or_largest_pixel inpixel;
      templated.getpixel(inpixel.c, templated.gettype(),
			 x, y, z, ibegin);
      pixeltypechg(inpixel.c, templated.gettype(), (char *)&min, psydouble);
      pixelsused=1;
      min_location.x=x; min_location.y=y; min_location.z=z; min_location.i=ibegin;
      max_location=min_location;
      max=mean=sum=min;
      sqrsum=min*min;
      var=adev=0;
      center_of_mass = xyziint2double(min_location);
    }
  }
  else {
    templated.gettemplatedstats(&min, &max, &mean, &pixelsused, &sum, &sqrsum,
				&var, &adev, &min_location, &max_location,
				&center_of_mass);
  }
  if(pixelsused == 0) {
// just insuring consistent results
    min_location.x=-1;
    min_location.y=min_location.z=min_location.i=0;
    max_location=min_location;
    max=mean=sum=min=sqrsum=var=adev=0;
    center_of_mass = xyziint2double(min_location);
  }
  if(quantify) {
    double wordres = templated.getwordres();
    min *= wordres; max *= wordres; mean *= wordres; sum *= wordres;
    adev *= wordres;
    sqrsum *= (wordres * wordres); var *= (wordres * wordres);
  }

  if(prttext)cout<<"stats over template region only:\n";
  if(prtmin==1){ if(prttext)cout<<"min="; cout<<min<<'\n'; }
  if(prtminloc==1){
    if(prttext)cout<<"min location=";
    cout<<'('<<min_location.x<<','<<min_location.y<<',';
    cout<<min_location.z<<','<<min_location.i<<")\n";
  }
  if(prtmax==1){ if(prttext)cout<<"max="; cout<<max<<'\n'; }
  if(prtmaxloc==1){
    if(prttext)cout<<"max location=";
    cout<<'('<<max_location.x<<','<<max_location.y<<',';
    cout<<max_location.z<<','<<max_location.i<<")\n";
  }
  if(prtmean==1){ if(prttext)cout<<"mean="; cout<<mean<<'\n'; }
  if(prtpixelsused==1){if(prttext)cout<<"pixelsused="; cout<<pixelsused<<'\n';}
  if(prtsum){ if(prttext)cout<<"sum="; cout<<sum<<'\n'; }
  if(prtsqrsum){ if(prttext)cout<<"square sum="; cout<<sqrsum<<'\n'; }
  if(prtvariance){ if(prttext)cout<<"variance="; cout<<var<<'\n'; }
  if(prtstddev){if(prttext)cout<<"standard deviation=";cout<<sqrt(var)<<'\n';}
  if(prtadev){ if(prttext)cout<<"mean absolute deviation="; cout<<adev<<'\n'; }
  if(prtcm) {
    if(prttext)cout<<"center of mass=";
    cout<<'('<<center_of_mass.x<<','<<center_of_mass.y<<',';
    cout<<center_of_mass.z<<','<<center_of_mass.i<<")\n";
  }

  if(outfile) {
// build new description
    string desc="roistats-templated output: ";
    desc += infile;
    templated.setdescription(desc);
// set date and time to current date and time
    templated.setdate();
    templated.settime();
// output result to analyze or ecat file
    psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg*)&templated,
				       outfileclass, outtype, outfileclassinfo);
// log
    logfile log(outfile, argc, argv);
    log.loginfilelog(infile);
    if(template_file != NULL)log.loginfilelog(template_file);
// print out templated images stats
    outpsyimgptr->getstats(&min, &max, &mean);
    if(prttext)
      cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
    delete outpsyimgptr;
  }

// clean up
  delete inimag;
  return 0;
/*
  if(templateptr)delete templateptr;
  if(templateimgptr)delete templateimgptr;
  if(template_img_buff_ptr)delete template_img_buff_ptr;
*/
}
