#include "psyhdr.h"

const char* psytypenames[] = {"psynotype", "psychar", "psyuchar", "psyshort",
			      "psyushort", "psyint", "psyuint", "psyfloat",
			      "psydouble", "psyshortsw", "psycomplex",
			      "psydate", "psystring", "psydicomdataelement",
                              "psyrgb", "psyargb"};

const char* psyfileclassnames[] = {
  "psynoclass", "rawfileclass", "analyzefileclass",
  "ecatfileclass", "ecat7fileclass", "dicomfileclass",
  "sdtfileclass", "niftifileclass"};

const char* imagespacenames[] = {"unknown_space", "scanner_anatomical_space",
				 "aligned_anatomical_space", "talairach_space",
				 "mni_152_space"};

psyimg::psyimg()
{
  initpsyimg(0, 0, 0, 0, psyuchar);
}

psyimg::psyimg(int xdim, int ydim, int zdim, int idim,
	       psytype pixeltype,
	       int xorig, int yorig, int zorig, int iorig,
	       int skippixels,
	       double xres, double yres, double zres,
	       double ires, double wres)
{
  initpsyimg(xdim,ydim,zdim,idim,pixeltype,
	     xorig,yorig,zorig,iorig,skippixels,
	     xres, yres, zres, ires, wres);
}

psyimg::~psyimg()
{
// clean up allocated strings
}

void psyimg::setorig(int xorig, int yorig, int zorig, int iorig)
{
  orig.x=xorig, orig.y=yorig, orig.z=zorig; orig.i=iorig;
}

void psyimg::setend(int xend, int yend, int zend, int iend)
{
  end.x=xend; end.y=yend; end.z=zend; end.i=iend;
}

void psyimg::setres(double xres, double yres, double zres, double ires)
{
  res.x=xres; res.y=yres; res.z=zres; res.i=ires;
}

void psyimg::setinc(int xinc, int yinc, int zinc, int iinc)
{
  inc.x=xinc; inc.y=yinc; inc.z=zinc; inc.i=iinc;
}

void psyimg::setstats(double min, double max, double mean, double sum,
		      double sqrsum)
{
  minimum=min; maximum=max; average=mean; summation=sum;
  sum_of_squares=sqrsum;
  statstatus=StatsInitialized;
}

void psyimg::setpatientid(string pid)
{
  patientid=pid;
}

void psyimg::setdate()
{
  date = currentdate();
}

void psyimg::setdate(string dt)
{
  date = dt;
}

void psyimg::settime()
{
  time=currenttime();
}

void psyimg::settime(string tm)
{
  time=tm;
}

void psyimg::setdescription(string desc)
{
  description = desc;
}

string psyimg::getpatientid()
{
  return(patientid);
}

string psyimg::getdate()
{
  return(date);
}

void psyimg::getdate(int *month, int *day, int *year)
{
  int localmonth=0; int localday=0; int localyear=0;
  string date=getdate();
  if(date.length() > 0) {
    sscanf(date.c_str(), "%d/%d/%d", &localmonth, &localday, &localyear);
  }
  if(month != NULL)*month=localmonth;
  if(day != NULL)*day=localday;
  if(year != NULL)*year=localyear;
  return;
}

string psyimg::gettime()
{
  return time;
}

void psyimg::gettime(int *hour, int *minute, int *second)
{
  int localhour=0; int localminute=0; int localsecond=0;
  string time=gettime();
  if(time.length()) {
    sscanf(time.c_str(), "%d:%d:%d", &localhour, &localminute, &localsecond);
  }
  if(hour != NULL)*hour=localhour;
  if(minute != NULL)*minute=localminute;
  if(second != NULL)*second=localsecond;
  return;
}

string psyimg::getdescription()
{
  return(description);
}

void psyimg::setspatialtransform(threeDtransform *transform, imagespacecode code) {
  spatialtransformcode = code;
  spatialtransform = transform;
}

threeDtransform *psyimg::getspatialtransform() {
  return spatialtransform;
}

imagespacecode psyimg::getspatialtransformcode() {
  return spatialtransformcode;
}

void psyimg::setspatialtransform2(threeDtransform *transform, imagespacecode code) {
  spatialtransformcode2 = code;
  spatialtransform2 = transform;
}

threeDtransform *psyimg::getspatialtransform2() {
  return spatialtransform2;
}

imagespacecode psyimg::getspatialtransformcode2() {
  return spatialtransformcode2;
}

void psyimg::initpsyimg(int xdim, int ydim, int zdim, int idim,
			psytype pixeltype,
			int xorig, int yorig, int zorig,
			int iorig, int skippixels,
			double xres, double yres, double zres,
			double ires, double wres)
{
  type=pixeltype;
  setorig(xorig, yorig, zorig, iorig);
  setsizeendinc(xdim, ydim, zdim, idim);
  skip=skippixels;
  statstatus=StatsUninitialized;
  setres(xres, yres, zres, ires);
  setwordres(wres);
  setorient(psynoorient);
  setpatientid("");
  setdate();
  settime();
  setdescription("");
  spatialtransform=NULL; spatialtransformcode=unknown_space;
  spatialtransform2=NULL; spatialtransformcode2=unknown_space;
}

void psyimg::setsizeendinc(int xdim, int ydim, int zdim, int idim)
{
  int xinc, yinc, zinc, iinc;
  size.x=xdim; size.y=ydim, size.z=zdim; size.i=idim;
  setend(orig.x+size.x-1, orig.y+size.y-1, orig.z+size.z-1, orig.i+size.i-1);
  xinc=gettypebytes(type); yinc=xinc*size.x;
  zinc=yinc*size.y; iinc=zinc*size.z;
  setinc(xinc, yinc, zinc, iinc);
  length=inc.i*size.i;
}

int psyimg::samedim(int xdim, int ydim, int zdim, int idim)
{
  return((xdim==size.x)&&(ydim==size.y)&&(zdim==size.z)&&(idim==size.i));
}

int psyimg::samedim(psyimg *psyimgptr)
{
  int xdim, ydim, zdim, idim;
  if(psyimgptr == NULL)return(0);
  psyimgptr->getsize(&xdim, &ydim, &zdim, &idim);
  return((xdim==size.x)&&(ydim==size.y)&&(zdim==size.z)&&(idim==size.i));
}

void psyimg::initgetpixel(psytype pixeltype, int x, int y, int z, int i) {
  output_tree(&cerr);
  cerr<<":psyimg::initgetpixel - not implement\n";
  cerr<<"  this message inherited from psyimg class\n";
  exit(1);
}

void psyimg::freegetpixel() {
  output_tree(&cerr);
  cerr<<":psyimg::freegetpixel - not implement\n";
  cerr<<"  this message inherited from psyimg class\n";
  exit(1);
}

void psyimg::getnextpixel(char *pixel) {
  output_tree(&cerr);
  cerr<<":psyimg::getnextpixel(char*) - not implement\n";
  cerr<<"  this message inherited from psyimg class\n";
  exit(1);
}

void psyimg::copyblock(char *outbuff, int xorig, int yorig, int zorig,
		       int iorig, int xend, int yend, int zend, int iend,
		       int out_xinc, int out_yinc,
		       int out_zinc, int out_iinc,
		       psytype pixeltype)
{
  //  char_or_largest_pixel inpixel;
  int x, y, z, i;
  char *xoptr, *yoptr, *zoptr, *ioptr;
// select most efficient loop to use
  ioptr=outbuff;
  if(xorig != orig.x || xend != end.x) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  initgetpixel(pixeltype, xorig, y, z, i);
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            getnextpixel(xoptr);
	  }//end for(x=
	  freegetpixel();
        }//end for(y=
      }//end for(z=
    }//end for(i=
  } else if(yorig != orig.y || yend != end.y) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	initgetpixel(pixeltype, xorig, yorig, z, i);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            getnextpixel(xoptr);
	  }//end for(x=
        }//end for(y=
	freegetpixel();
      }//end for(z=
    }//end for(i=
  } else if(zorig != orig.z || zend != end.z) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      initgetpixel(pixeltype, xorig, yorig, zorig, i);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            getnextpixel(xoptr);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
      freegetpixel();
    }//end for(i=
  } else {
    initgetpixel(pixeltype, xorig, yorig, zorig, iorig);
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routine
            getnextpixel(xoptr);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
    }//end for(i=
    freegetpixel();
  }
}

void psyimg::getsize(int *x, int *y, int *z, int *i)
{
  if(x!=NULL)*x=size.x;
  if(y!=NULL)*y=size.y;
  if(z!=NULL)*z=size.z;
  if(i!=NULL)*i=size.i;
}

void psyimg::getinc(int *x, int *y, int *z, int *i)
{
  if(x!=NULL)*x=inc.x;
  if(y!=NULL)*y=inc.y;
  if(z!=NULL)*z=inc.z;
  if(i!=NULL)*i=inc.i;
}

void psyimg::getorig(int *x, int *y, int *z, int *i)
{
  if(x!=NULL)*x=orig.x;
  if(y!=NULL)*y=orig.y;
  if(z!=NULL)*z=orig.z;
  if(i!=NULL)*i=orig.i;
}

void psyimg::getend(int *x, int *y, int *z, int *i)
{
  if(x!=NULL)*x=end.x;
  if(y!=NULL)*y=end.y;
  if(z!=NULL)*z=end.z;
  if(i!=NULL)*i=end.i;
}

void psyimg::getres(double *x, double *y, double *z, double *i)
{
  if(x!=NULL)*x=res.x;
  if(y!=NULL)*y=res.y;
  if(z!=NULL)*z=res.z;
  if(i!=NULL)*i=res.i;
}

int psyimg::inside(int x, int y, int z, int i)
{
  return( (orig.x<=x) && (x<=end.x) && (orig.y<=y) && (y<=end.y) &&
	  (orig.z<=z) && (z<=end.z) && (orig.i<=i) && (i<=end.i));
}

int psyimg::inside(psydims location)
{
  return((orig.x<=location.x) && (location.x<=end.x) &&
	 (orig.y<=location.y) && (location.y<=end.y) &&
	 (orig.z<=location.z) && (location.z<=end.z) &&
	 (orig.i<=location.i) && (location.i<=end.i));
}

long psyimg::offset(int x, int y, int z, int i)
{
  return( (x-orig.x)*inc.x + (y-orig.y)*inc.y + (z-orig.z)*inc.z +
	 (i-orig.i)*inc.i + skip);
}

void psyimg::showpsyimg()
{
  cout << "size=(" << size.x << "," << size.y << "," << size.z << ","
    << size.i << ")\n";
  cout << "inc=(" << inc.x << "," << inc.y << "," << inc.z << ","
    << inc.i << ")\n";
  cout << "orig=(" << orig.x << "," << orig.y << "," << orig.z << ","
    << orig.i << ")\n";
  cout << "end=(" << end.x << "," << end.y << "," << end.z << ","
    << end.i << ")\n";
  cout << "res=(" << res.x << "," << res.y << "," << res.z << ","
    << res.i << ")\n";
  cout <<"wordres="<<wordres<<'\n';
  cout << "skip=" << skip << "\n";
  cout << "length=" << length << "\n";
  cout << "type=" << type << "\n";
  cout << "orientation=" << orientation << "\n";
  cout << "bytesperpixel=" << getbytesperpixel() << "\n";
  cout << "statstatus=" << statstatus << "\n";
  if(statstatus == StatsInitialized) {
    cout<<"minimum="<<minimum<<'\n';
    cout<<"maximum="<<maximum<<'\n';
    cout<<"average="<<average<<'\n';
    cout<<"summation="<<summation<<'\n';
    cout<<"sum_of_squares="<<sum_of_squares<<'\n';
  }
  else cout<<"no stats\n";
  string pid=getpatientid();
  if(pid.length() > 0) cout <<"patientid="<<pid<<'\n';
  else cout<<"no patientid\n";
  string localdate=getdate();
  if(localdate.length() > 0) cout <<"date="<<localdate<<'\n';
  else cout<<"no date\n";
  string localtime=gettime();
  if(localtime.length() > 0) cout <<"time="<<localtime<<'\n';
  else cout<<"no time\n";
  string desc = getdescription();
  if(desc.length() > 0) cout <<"description="<<desc<<'\n';
  else cout<<"no description\n";
  if(getspatialtransform() != NULL) {
    cout <<"spatialtransform=\n";
    getspatialtransform()->write(&cout);
  }
  else cout<<"no spatialtransform\n";
  cout <<"spatialtransformcode="<<getspatialtransformcode()<<" ("<<imagespacenames[getspatialtransformcode()]<<")\n";
  if(getspatialtransform2() != NULL) {
    cout <<"spatialtransform2=\n";
    getspatialtransform2()->write(&cout);
  }
  else cout<<"no spatialtransform2\n";
  cout <<"spatialtransformcode2="<<getspatialtransformcode2()<<" ("<<imagespacenames[getspatialtransformcode2()]<<")\n";
}

void psyimg::getstats(double *min, double *max, double *mean, double *sum,
		      double *sqrsum)
{
  double localmin, localmax, localmean, localsum, localsqrsum;
  double dpixel;
  char_or_largest_pixel pixel;
  int numwords;

  if(getstatstatus() != StatsInitialized) {
    switch(type) {
    default:
      // type2double not available -- no stats
      setstats(0, 0, 0, 0, 0);
      break;
    case psychar:
    case psyuchar:
    case psyshort:
    case psyushort:
    case psyint:
    case psyuint:
    case psyfloat:
    case psycomplex:
    case psydouble:
    case psystring:   
      numwords=size.x * size.y * size.z * size.i;

// retrieve and initialize values using first pixel
      initgetpixel(type, orig.x, orig.y, orig.z, orig.i);
      getnextpixel(pixel.c);
      type2double(pixel.c, type, (char *)&dpixel);
      localmax = localmin = localsum = dpixel;
      localsqrsum = dpixel*dpixel;

// loop through remaining pixels
      for(int i=1; i<numwords; i++) {
	getnextpixel(pixel.c);
	type2double(pixel.c, type, (char *)&dpixel);
	localsum += dpixel;
	localsqrsum += dpixel*dpixel;
	if(dpixel > localmax)localmax=dpixel;
	else if(dpixel < localmin)localmin=dpixel;
      }
      localmean=localsum/numwords;
      
      setstats(localmin, localmax, localmean, localsum, localsqrsum);
      freegetpixel();
    }
  }
  if(min != NULL)*min=minimum;
  if(max != NULL)*max=maximum;
  if(mean != NULL)*mean=average;
  if(sum != NULL)*sum=summation;
  if(sqrsum != NULL)*sqrsum=sum_of_squares;
}

void psyimg::showstats()
{
  double min, max, mean, sum, sqrsum;
  getstats(&min, &max, &mean, &sum, &sqrsum);
  cout<<"min="<<min<<" max="<<max<<" mean="<<mean;
  cout<<" sum="<<sum<<" sqrsum="<<sqrsum<<'\n';
}

void psyimg::getthreshstats(double *min, double *max, double *mean,
			    int *pixelsused, double thresh,
			    double *sum, double *sqrsum)
{
  double localsum, localsqrsum, localmin, localmax, localmean;
  localsum=localsqrsum=localmin=localmax=0;
  int localpixelsused;
  double dpixel;
  char_or_largest_pixel pixel;
  int numwords;

  numwords=size.x * size.y * size.z * size.i;
// initialize getpixel to first pixel
  initgetpixel(type, orig.x, orig.y, orig.z, orig.i);
// loop through remaining pixels
  localpixelsused=0;
  for(int i=0; i<numwords; i++) {
    getnextpixel(pixel.c);
    type2double(pixel.c, type, (char *)&dpixel);
    if(dpixel >= thresh) {
      if(localpixelsused == 0) {
	localmin = localmax = localsum = dpixel;
	localsqrsum = dpixel*dpixel;
      }
      else {
	if(dpixel > localmax)localmax=dpixel;
	else if(dpixel < localmin)localmin=dpixel;
	localsum += dpixel;
	localsqrsum += dpixel*dpixel;
      }
      localpixelsused++;
    }
  }
  freegetpixel();

  if(localpixelsused == 0){
// avoid possible divide by zero and set unset values to zero
    localmean=0;
    localmin=localmax=localsum=localsqrsum=0;
  }
  else {
    localmean=localsum/(localpixelsused);
  }
// return values
  if(min != NULL)*min = localmin;
  if(max != NULL)*max = localmax;
  if(mean != NULL)*mean = localmean;
  if(pixelsused != NULL)*pixelsused = localpixelsused;
  if(sum != NULL)*sum = localsum;
  if(sqrsum != NULL)*sqrsum = localsqrsum;
}

psyimglnk::psyimglnk() : psyimg()
{
  inputpsyimg=NULL;
}

psyimglnk::psyimglnk(psyimg *psyimgptr, psytype pixeltype) : psyimg()
{
  initpsyimglnk(psyimgptr, pixeltype);
}

psyimglnk::psyimglnk(psyimg *psyimgptr,
		     int xdim, int ydim, int zdim, int idim,
		     psytype pixeltype,
		     int xorig, int yorig, int zorig,
		     int iorig, int skippixels,
		     double xres, double yres, double zres,
		     double ires, double wres) : psyimg()
{
  initpsyimglnk(psyimgptr, xdim, ydim, zdim, idim, pixeltype,
		xorig, yorig, zorig, iorig, skippixels,
		xres, yres, zres, ires, wres);
}

void psyimglnk::initpsyimglnk(psyimg *psyimgptr, psytype pixeltype)
{
  int xdim, ydim, zdim, idim;
  int xorig, yorig, zorig, iorig;
  double xres, yres, zres, ires;
// set psyimg values to same as input image
  if(psyimgptr != NULL) {
    psyimgptr->getsize(&xdim, &ydim, &zdim, &idim);
    psyimgptr->getorig(&xorig, &yorig, &zorig, &iorig);
    psyimgptr->getres(&xres, &yres, &zres, &ires);
  }
  else {
    xdim=ydim=zdim=idim=0;
    xorig=yorig=zorig=iorig=0;
    xres=yres=zres=ires=1;
  }
  initpsyimglnk(psyimgptr, xdim, ydim, zdim, idim,
		pixeltype,
		xorig, yorig, zorig, iorig, 0,
		xres, yres, zres, ires,
		psyimgptr->getwordres());
}

void psyimglnk::initpsyimglnk(psyimg *psyimgptr,
			      int xdim, int ydim, int zdim, int idim,
			      psytype pixeltype,
			      int xorig, int yorig, int zorig,
			      int iorig, int skippixels,
			      double xres, double yres, double zres,
			      double ires, double wres)
{
  inputpsyimg=psyimgptr;
// set psyimg type to input image type
  if(pixeltype == psynotype) {
    if(inputpsyimg != NULL)pixeltype=inputpsyimg->gettype();
    else pixeltype=psyuchar;
  }
  initpsyimg(xdim,ydim,zdim,idim,pixeltype,xorig,yorig,zorig,iorig,
	     skippixels,xres,yres,zres,ires,wres);
  if(inputpsyimg != NULL) {
    setpatientid(inputpsyimg->getpatientid());
    setdate(inputpsyimg->getdate());
    settime(inputpsyimg->gettime());
    setdescription(inputpsyimg->getdescription());
    setorient(inputpsyimg->getorient());
    setspatialtransform(inputpsyimg->getspatialtransform(),inputpsyimg->getspatialtransformcode());
    setspatialtransform2(inputpsyimg->getspatialtransform2(),inputpsyimg->getspatialtransformcode2());
  }
}

threeDtransform *psyimglnk::getspatialtransform() {
  if(spatialtransform == NULL && inputpsyimg != NULL) return inputpsyimg->getspatialtransform();
  return spatialtransform;
}

imagespacecode psyimglnk::getspatialtransformcode() {
  if(spatialtransformcode == unknown_space && spatialtransform == NULL && inputpsyimg != NULL)
    return inputpsyimg->getspatialtransformcode();
  else return spatialtransformcode;
}

threeDtransform *psyimglnk::getspatialtransform2() {
  if(spatialtransform2 == NULL && inputpsyimg != NULL) return inputpsyimg->getspatialtransform2();
  return spatialtransform2;
}

imagespacecode psyimglnk::getspatialtransformcode2() {
  if(spatialtransformcode2 == unknown_space && spatialtransform2 == NULL && inputpsyimg != NULL)
    return inputpsyimg->getspatialtransformcode2();
  else return spatialtransformcode2;
}

psybuff::psybuff() : psyimg()
{
  buff=(char *)NULL;
  currentbuffptr=buff;
  currentouttype=psynotype;
  getnextpixel_lock=0;
}

psybuff::psybuff(int x, int y, int z, int i, psytype pixeltype,
		 int xorig, int yorig, int zorig, int iorig,
		 double xres, double yres, double zres,
		 double ires, double wres)
{
  initbuff(x,y,z,i,pixeltype,xorig,yorig,zorig,iorig,xres,yres,zres,ires,wres);
}

void psybuff::initbuff(int x, int y, int z, int i,
		       psytype pixeltype,
		       int xorig, int yorig, int zorig, int iorig,
		       double xres, double yres, double zres,
		       double ires, double wres)
{
  initpsyimg(x,y,z,i,pixeltype,xorig,yorig,zorig,iorig,0,xres,yres,zres,ires,wres);
  buff=new char[length];
  currentbuffptr=(char *)NULL;
  getnextpixel_lock=0;
}

psybuff::~psybuff()
{
  delete[] buff;
}

void psybuff::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  if(!inside(x, y, z, i)) {
    output_tree(&cerr);
    cerr << ":psybuff::initgetpixel - request("<<x<<','<<y<<','<<z<<','<<i;
    cerr <<") outside of buffer\n";
    exit(1);
  }
  if(getnextpixel_lock) {
    output_tree(&cerr);
    cerr << ":psybuff::initgetpixel - programming error - get pixel locked\n";
    cerr << "add a buffer or check for missing call to freegetpixel\n";
    exit(1);
  }
  currentbuffptr=buff + offset(x,y,z,i);
  currentouttype=pixeltype;
  getnextpixel_lock=1;
}

void psybuff::getpixel(char *pixel, psytype pixeltype,
		       int x, int y, int z, int i)
{
  initgetpixel(pixeltype, x, y, z, i);
  getnextpixel(pixel);
  freegetpixel();
}

void psybuff::getnextpixel(char *pixel)
{
// no checking assumes caller knows extents of buffer and called initgetpixel
  if(type == currentouttype) {
    for(int j=0; j<inc.x; j++) *pixel++ = *currentbuffptr++;
  }
  else {
    pixeltypechg(currentbuffptr, type, pixel, currentouttype);
    currentbuffptr += inc.x;
  }
}

void psybuff::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			 int iorig, int xend, int yend, int zend, int iend,
			 int xinc, int yinc, int zinc, int iinc,
			 psytype pixeltype)
{
  char *xptr, *yptr, *zptr, *iptr;
  char *xoptr, *yoptr, *zoptr, *ioptr;
  int b, x, y, z, i;
// check bounds
  if(!inside(xorig, yorig, zorig, iorig) || 
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr << ":psybuff::copyblock - request outside of buffer\n";
    cerr<<"orig=("<<xorig<<","<<yorig<<","<<zorig<<","<<iorig<<")\n";
    cerr<<"end=("<<xend<<","<<yend<<","<<zend<<","<<iend<<")\n";
    exit(1);
  }
// transfer data
  iptr=buff + offset(xorig, yorig, zorig, iorig);
  ioptr=outbuff;
  if(type == pixeltype) {
    for(i=iorig; i<=iend; i++, iptr+=inc.i, ioptr+=iinc) {
      for(z=zorig, zptr=iptr, zoptr=ioptr; z<=zend;
	  z++, zptr+=inc.z, zoptr+=zinc) {
	for(y=yorig, yptr=zptr, yoptr=zoptr; y<=yend;
	    y++, yptr+=inc.y, yoptr+=yinc) {
	  for(x=xorig, xptr=yptr, xoptr=yoptr; x<=xend;
	      x++, xptr+=inc.x, xoptr+=xinc) {
	    for(b=0; b<inc.x; b++)xoptr[b]=xptr[b];
	  } // end for(x=0
	}  // end for(y=0
      } // end for(z=0
    } // end for(i=0
  }
  else {
    for(i=iorig; i<=iend; i++, iptr+=inc.i, ioptr+=iinc) {
      for(z=zorig, zptr=iptr, zoptr=ioptr; z<=zend;
	  z++, zptr+=inc.z, zoptr+=zinc) {
	for(y=yorig, yptr=zptr, yoptr=zoptr; y<=yend;
	    y++, yptr+=inc.y, yoptr+=yinc) {
	  for(x=xorig, xptr=yptr, xoptr=yoptr; x<=xend;
	      x++, xptr+=inc.x, xoptr+=xinc) {
	    pixeltypechg(xptr, type, xoptr, pixeltype);
	  } // end for(x=0
	}  // end for(y=0
      } // end for(z=0
    } // end for(i=0
  }
}

void psybuff::showbuff()
{
  cout << "buff=" << (int *)buff << '\n';
  cout << "currentbuffptr=" << (int *)currentbuffptr << '\n';
  showpsyimg();
}

psyfullbuff::psyfullbuff() : psyimglnk()
{
}

psyfullbuff::psyfullbuff(psyimg *psyimgptr, psytype pixeltype)
: psyimglnk()
{
  initpsyfullbuff(psyimgptr, pixeltype);
}

void psyfullbuff::initpsyfullbuff(psyimg *psyimgptr,
				  psytype pixeltype)
{
  initpsyimglnk(psyimgptr, pixeltype);
}

void psyfullbuff::chknfillbuff() {
  if(buffimage.getbuff() == NULL) {
// initialize buffer to full image size
    buffimage.initbuff(size.x, size.y, size.z, size.i, type);
// transfer image to buffer
    inputpsyimg->copyblock(buffimage.getbuff(),
			   orig.x, orig.y, orig.z, orig.i,
			   end.x, end.y, end.z, end.i,
			   inc.x, inc.y, inc.z, inc.i,
			   type);
  }
}

void psyfullbuff::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			    int iorig, int xend, int yend, int zend, int iend,
			    int xinc, int yinc, int zinc, int iinc,
			    psytype pixeltype)
{
  if(!inside(xorig, yorig, zorig, iorig) || 
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr << ":psyfullbuff::copyblock - request outside of buffer\n";
    cerr<<"orig=("<<xorig<<","<<yorig<<","<<zorig<<","<<iorig<<")\n";
    cerr<<"end=("<<xend<<","<<yend<<","<<zend<<","<<iend<<")\n";

    exit(1);
  }

// make sure buffer has been filled
  chknfillbuff();

// transfer from buffer to output buffer
  buffimage.copyblock(outbuff, xorig, yorig, zorig, iorig,
		      xend, yend, zend, iend, xinc, yinc, zinc, iinc,
		      pixeltype);
}

void psyfullbuff::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
// make sure buffer has been filled
  chknfillbuff();
  buffimage.initgetpixel(pixeltype,x,y,z,i);
}

void psyfullbuff::getpixel(char *pixel, psytype pixeltype,
			   int x, int y, int z, int i)
{
// make sure buffer has been filled
  chknfillbuff();
  buffimage.getpixel(pixel,pixeltype,x,y,z,i);
}

void psyfullbuff::getnextpixel(char *pixel)
{
// no checking - hopefully user called initgetpixel first
  buffimage.getnextpixel(pixel);
}

pgbuff::pgbuff(int size)
{
  buff=NULL;
  next=prev=NULL;
  arrayloc=NULL;
  lockcount=0;
  if(size > 0)buff=new char [size];
  else buff=NULL;
}

pgbuff::~pgbuff()
{
  delete[] buff;
}

psypgbuff::psypgbuff() : psyimglnk()
{
  linklist=NULL;
  pgbuffarray=NULL;
  numpages=0;
  maxpages=0;
  currentz=0;
  currenti=0;
  currentbuffptr=NULL;
  endcurrentpg=currentbuffptr-1;
  currentouttype=psynotype;
  getnextpixel_lock=0;
}

psypgbuff::psypgbuff(psyimg *psyimgptr, int maxnumpages) : psyimglnk()
{
  linklist=NULL;
  pgbuffarray=NULL;
  numpages=0;
  maxpages=0;
  currentz=0;
  currenti=0;
  currentbuffptr=NULL;
  endcurrentpg=currentbuffptr-1;
  currentouttype=psynotype;
  initpgbuff(psyimgptr, maxnumpages);
}

psypgbuff::~psypgbuff()
{
  pgbuff *pgbuffptr;
// checked for locked pages
  if(pgbuffarray != NULL) {
    int npgs = size.z * size.i;
    for(int j=0; j<npgs; j++) {
      if(pgbuffarray[j] != NULL) {
	if(pgbuffarray[j]->lockcount != 0) {
	  output_tree(&cerr);
	  cerr<<":psypgbuff::~psypgbuff - warning page still locked not deleted\n";
	}
      }
    }
    delete[] pgbuffarray;
  }
// delete page buffers in link list
  while(linklist != NULL) {
    pgbuffptr=linklist->next;
    delete linklist;
    linklist=pgbuffptr;
  }
}

void psypgbuff::initpgbuff(psyimg *psyimgptr, int maxnumpages,
			   psytype pixeltype)
{
  initpsyimglnk(psyimgptr, pixeltype);
  initpgbuff(maxnumpages);
}

void psypgbuff::initpgbuff(psyimg *psyimgptr,
			   int xdim, int ydim, int zdim, int idim,
			   psytype pixeltype,
			   int xorig, int yorig, int zorig,
			   int iorig, int skippixels,
			   double xres, double yres, double zres,
			   double ires, double wres,
			   int maxnumpages)
{
  initpsyimglnk(psyimgptr, xdim, ydim, zdim, idim, pixeltype,
		xorig, yorig, zorig, iorig, skippixels,
		xres, yres, zres, ires, wres);
  initpgbuff(maxnumpages);
}

void psypgbuff::initpgbuff(int maxnumpages)
{
  int npgs;
// assumes psyimgptr and sizes already set
// set remaining values
  linklist=NULL;
  numpages=0;
  maxpages=maxnumpages;
  npgs=size.z*size.i;
  if(npgs <= 0) {
    cout<<"psypgbuff::initpgbuff - invalid psyimg sizes\n";
    exit(1);
  }
  pgbuffarray=new pgbuff *[npgs];
  for(int j=0; j<npgs; j++)pgbuffarray[j]=NULL;
  currentz=0;
  currenti=0;
  currentbuffptr=NULL;
  endcurrentpg=currentbuffptr-1;
  currentouttype=psynotype;
  getnextpixel_lock=0;
}

void psypgbuff::reset(psyimg *psyimgptr)
{
  int oldincz, newincz;
  int npgs;
// stats no longer valid
  unsetstats();
  if(pgbuffarray == NULL) {
// never set in the first place - initialize and return
    if(psyimgptr != NULL)initpgbuff(psyimgptr);
    return;
  }
  if(psyimgptr != NULL) {
// check dimensions
    if(!samedim(psyimgptr)) {
      output_tree(&cerr);
      cerr<<":psypgbuff::reset - error new image dimensions differ from old\n";
      exit(1);
    }
// save old page size
    getinc(NULL, NULL, &oldincz, NULL);
// reset pointer
    initpsyimglnk(psyimgptr, gettype());
    getinc(NULL, NULL, &newincz, NULL);
    if(oldincz != newincz) {
      output_tree(&cerr);
      cerr<<":psypgbuff::reset - error new image page size differs from old\n";
      exit(1);
    }
  }
// remove indexed page pointers while checking for locked pages
// leaves allocated buffers in link list
  npgs = size.z * size.i;
  for(int j=0; j<npgs; j++) {
    if(pgbuffarray[j] != NULL) {
      if(pgbuffarray[j]->lockcount != 0) {
	output_tree(&cerr);
	cerr<<":psypgbuff::reset - error pages still locked\n";
	exit(1);
      }
      pgbuffarray[j]=NULL; // page not filled
    }
  }
}

char *psypgbuff::getzptrlocked(int z, int i)
{
// get buffer filled for this page
  char *ptr=getzptr(z, i);
// get pointer to this page buffer
  pgbuff *pgbuffptr=pgbuffarray[(i-orig.i)*size.z + z-orig.z];
// locking the zptr depends on getting it out of the linked list
// used by getzptr
// remove from link list so getzptr wont use it
  if(pgbuffptr->prev != NULL)pgbuffptr->prev->next=pgbuffptr->next;
  else if(linklist == pgbuffptr)linklist=pgbuffptr->next;
  if(pgbuffptr->next != NULL)pgbuffptr->next->prev=pgbuffptr->prev;
  pgbuffptr->prev=NULL;
  pgbuffptr->next=NULL;
// keep track of number of seperate calls locking it
  pgbuffptr->lockcount++;
  return(ptr);
}

void psypgbuff::unlockzptr(int z, int i)
{
  pgbuff *pgbuffptr=pgbuffarray[(i-orig.i)*size.z + z-orig.z];
  if(pgbuffptr == NULL) {
    output_tree(&cerr);
    cerr<<":psypgbuff:psypgbuff::unlockzptr called with invalid page\n";
    exit(1);
  }
  if(pgbuffptr->lockcount < 1) {
    output_tree(&cerr);
    cerr<<":psypgbuff::unlockzptr called with non-locked page\n";
    exit(1);
  }
// decrement lock count
  pgbuffptr->lockcount--;
// if no other locks
  if(pgbuffptr->lockcount == 0) {
// insert at the top of the link list
    pgbuffptr->next=linklist;
    if(linklist != NULL)linklist->prev=pgbuffptr;
    pgbuffptr->prev=NULL;
    linklist=pgbuffptr;
  }
  else if(pgbuffptr->lockcount < 0) {
    output_tree(&cerr);
    cerr<<":psypgbuff::unlockzptr called for unlocked page\n";
    exit(1);
  }    
}

char *psypgbuff::getzptr(int z, int i)
{
  int indice;
  pgbuff *pgbuffptr;

  indice=(i-orig.i)*size.z + z-orig.z;
  pgbuffptr=pgbuffarray[indice];
  if(pgbuffptr == NULL) {
    if(numpages < maxpages) {
      numpages++;
// get memory for this page buffer
      pgbuffptr=new pgbuff(inc.z);
// put this new page at head of link list
      pgbuffptr->prev=NULL;
      pgbuffptr->next=linklist;
      if(linklist != NULL)linklist->prev=pgbuffptr;
      linklist=pgbuffptr;
// insert it into the array
      pgbuffptr->arrayloc= &pgbuffarray[indice];
      pgbuffarray[indice]=pgbuffptr;
    }
    else {
// check if any pages available
      if(linklist == NULL) {
	output_tree(&cerr);
	cerr<<":psypgbuff::getzptr - out of page buffers\n";
	exit(1);
      }
// find page unused for longest time
      pgbuffptr=linklist;
      while(pgbuffptr->next != NULL) {
	pgbuffptr=pgbuffptr->next;
      }
// set the old location in the array to null
      *(pgbuffptr->arrayloc)=NULL;
// reset it as this page
      pgbuffptr->arrayloc= &pgbuffarray[indice];
      pgbuffarray[indice]=pgbuffptr;
    }
// get the data into this page buffer
    fillpage(pgbuffptr->buff, z, i);
  }

// if pgbuffptr not at top move it to the top of the link list
// note also if pgbuffptr is not in link list(locked) prev will be NULL
  if(pgbuffptr->prev != NULL) {
// first remove it from the link list
    pgbuffptr->prev->next=pgbuffptr->next;
    if(pgbuffptr->next != NULL)pgbuffptr->next->prev=pgbuffptr->prev;
// then insert it at the top
     pgbuffptr->prev=NULL;
     pgbuffptr->next=linklist;
     if(linklist != NULL)linklist->prev=pgbuffptr;
     linklist=pgbuffptr;
  }
// return the data buffer
  return(pgbuffptr->buff);
}

void psypgbuff::fillpage(char *buff, int z, int i)
{
  inputpsyimg->copyblock(buff, orig.x, orig.y, z, i,
			 end.x, end.y, z, i, inc.x, inc.y, inc.z, inc.i,
			 type);
  return;
}

void psypgbuff::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			  int iorig, int xend, int yend, int zend, int iend,
			  int xinc, int yinc, int zinc, int iinc,
			  psytype pixeltype)
{
  int i, z, y, x, b;
  char *zptr, *yptr, *xptr, *ioptr, *zoptr, *yoptr, *xoptr;
  int origoffset;

  if(!inside(xorig, yorig, zorig, iorig) ||
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr<<":psypgbuff::copyblock - block falls outside of image\n";
    exit(1);
  }
// get offset needed for x and y orig into plane
  origoffset=offset(xorig, yorig, orig.z, orig.i) -
    offset(orig.x, orig.y, orig.z, orig.i);
  ioptr=outbuff;
  if(type == pixeltype) {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	zptr=getzptr(z, i)+origoffset;
	for(y=yorig, yptr=zptr, yoptr=zoptr; y<=yend;
	    y++, yptr+=inc.y, yoptr+=yinc) {
	  for(x=xorig, xptr=yptr, xoptr=yoptr; x<=xend;
	      x++, xptr+=inc.x, xoptr+=xinc) {
	    for(b=0; b<inc.x; b++)xoptr[b]=xptr[b];
	  } // end for(x=0
	}  // end for(y=0
      } // end for(z=
    } // end for(i=
  }
  else {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	zptr=getzptr(z, i)+origoffset;
	for(y=yorig, yptr=zptr, yoptr=zoptr; y<=yend;
	    y++, yptr+=inc.y, yoptr+=yinc) {
	  for(x=xorig, xptr=yptr, xoptr=yoptr; x<=xend;
	      x++, xptr+=inc.x, xoptr+=xinc) {
	    pixeltypechg(xptr, type, xoptr, pixeltype);
	  } // end for(x=0
	}  // end for(y=0
      } // end for(z=
    } // end for(i=
  }
}

void psypgbuff::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  if(!inside(x, y, z, i)) {
    output_tree(&cerr);
    cerr << ":psypgbuff::initgetpixel - request(";
    cerr <<x<<','<<y<<','<<z<<','<<i;
    cerr <<") outside of buffer\n";
    exit(1);
  }
  if(getnextpixel_lock) {
    output_tree(&cerr);
    cerr << ":psypgbuff::initgetpixel - programming error - get pixel locked\n";
    cerr << "add a buffer or check for missing call to freegetpixel\n";
    exit(1);
  }
  currentbuffptr=getzptrlocked(z, i);
// set the current pg buffer values used by getnextpixel
  endcurrentpg=currentbuffptr + inc.z;
  currentbuffptr += (x-orig.x)*inc.x + (y-orig.y)*inc.y;
  currentouttype=pixeltype;
  currentz=z;
  currenti=i;
  getnextpixel_lock=1;
}

void psypgbuff::freegetpixel() {
  unlockzptr(currentz, currenti);
  getnextpixel_lock=0;
}

void psypgbuff::getpixel(char *pixel, psytype pixeltype,
			 int x, int y, int z, int i)
{
  initgetpixel(pixeltype, x, y, z, i);
  if(type == currentouttype) {
    for(int j=0; j<inc.x; j++) *pixel++ = *currentbuffptr++;
  }
  else {
    pixeltypechg(currentbuffptr, type, pixel, currentouttype);
    currentbuffptr += inc.x;
  }
  freegetpixel();
}

void psypgbuff::getnextpixel(char *pixel)
{
  if(currentbuffptr >= endcurrentpg) {
    int oldz=currentz;
    int oldi=currenti;
    currentz++;
    if(currentz > end.z) {
      currentz = orig.z;
      currenti++;
      if(currenti > end.i) {
	output_tree(&cerr);
	cerr<<":psypgbuff::getnextpixel - request beyond end of image\n";
	exit(1);
      }
    }
    unlockzptr(oldz, oldi);
    currentbuffptr=getzptrlocked(currentz, currenti);
    endcurrentpg=currentbuffptr + inc.z;
  }
  if(type == currentouttype) {
    for(int j=0; j<inc.x; j++) *pixel++ = *currentbuffptr++;
  }
  else {
    pixeltypechg(currentbuffptr, type, pixel, currentouttype);
    currentbuffptr += inc.x;
  }
}

reverserows::reverserows(psyimg *psyimgptr, int maxnumpages)
  : psypgbuff(psyimgptr, maxnumpages) {
  // fix spatial transforms
  psydims dim=psyimgptr->getsize();
  matrix4X4 Factors(Identity); Factors.m.m22 = -1.0l;
  xyzidouble newvoxorig;
  newvoxorig.x=newvoxorig.z=newvoxorig.i=0; newvoxorig.y = (dim.y-1);
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix * Factors;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform(new threeDtransform(newmatrix), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix * Factors;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform2(new threeDtransform(newmatrix), getspatialtransformcode2());
  }
}

void reverserows::fillpage(char *buff, int z, int i)
{
// set buffer to last y row
  buff += offset(orig.x, end.y, z, i) - offset(orig.x, orig.y, z, i);
// use negative increment to fill y rows backwards
  inputpsyimg->copyblock(buff, orig.x, orig.y, z, i,
			 end.x, end.y, z, i, inc.x, -inc.y, inc.z, inc.i,
			 type);
  return;
}

psyimglnkpxl::psyimglnkpxl() : psyimglnk()
{ 
  getpixeltype=psynotype;
  getpixelloc.x=getpixelloc.y=getpixelloc.z=getpixelloc.i=0;
  getnextpixel_lock=0;
}

psyimglnkpxl::psyimglnkpxl(psyimg *psyimgptr, psytype pixeltype)
{
  initpsyimglnkpxl(psyimgptr, pixeltype);
}

void psyimglnkpxl::initpsyimglnkpxl(psyimg *psyimgptr,
				    psytype pixeltype)
{
  initpsyimglnk(psyimgptr, pixeltype);
  getnextpixel_lock=0;
}

void psyimglnkpxl::initpsyimglnkpxl(psyimg *psyimgptr,
				    int xdim, int ydim, int zdim, int idim,
				    psytype pixeltype,
				    int xorig, int yorig, int zorig,
				    int iorig, int skippixels,
				    double xres, double yres,
				    double zres,
				    double ires, double wres)
{
  initpsyimglnk(psyimgptr, xdim, ydim, zdim, idim, pixeltype,
		xorig, yorig, zorig, iorig, skippixels,
		xres, yres, zres, ires, wres);
  getnextpixel_lock=0;
}

void psyimglnkpxl::getpixel(char *pixel, psytype pixeltype,
			int x, int y, int z, int i)
{
  initgetpixel(pixeltype, x, y, z, i);
  getnextpixel(pixel);
  freegetpixel();
}

void psyimglnkpxl::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  if(!inside(x, y, z, i)) {
    output_tree(&cerr);
    cerr << ":psyimglnkpxl::initgetpixel - request(";
    cerr <<x<<','<<y<<','<<z<<','<<i;
    cerr <<") outside of buffer\n";
    exit(1);
  }
  if(getnextpixel_lock) {
    output_tree(&cerr);
    cerr << ":psyimglnkpxl::initgetpixel - programming error - get pixel locked\n";
    cerr << "add a buffer or check for missing call to freegetpixel\n";
    exit(1);
  }
  getpixeltype=pixeltype;
  getpixelloc.x=x; getpixelloc.y=y; getpixelloc.z=z; getpixelloc.i=i;
  getnextpixel_lock=1;
}

void psyimglnkpxl::incgetpixel()
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
     }  //end if(getpixelloc.z
    } //end if(getpixelloc.y
    freegetpixel();
    initgetpixel(getpixeltype, getpixelloc.x, getpixelloc.y,
		 getpixelloc.z, getpixelloc.i);
  } //end if(getpixelloc.x
}
