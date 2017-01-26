#include "psyhdr.h"

typedef unsigned long dimension;
void fourn(float data[], dimension nn[], int ndim, int isign);



class psygauss : public psyimglnkpxl {
  xydouble mean;
  xydouble std_dev;
  double scale;
 public:
  psygauss(xydouble std_dev, double percent_peak,
	   psytype pixeltype=psyfloat);
  void output_tree(ostream *out) {psyimglnkpxl::output_tree(out);*out<<"::psygauss";};
  void getnextpixel(char *pixel);
  double gauss_value(double x, double y);
};

psygauss::psygauss(xydouble std_dev, double percent_peak,
		   psytype pixeltype)
{
  psygauss::std_dev=std_dev;
  psydims dim;
// determine needed size
  mean.x=mean.y=0;
  scale=1.0;
  int x=0;
  int y=0;
  double thresh=percent_peak*gauss_value((double)x,(double)y);
  x=1;
  double value=gauss_value((double)x,(double)y);
  while(value>thresh){x++; value=gauss_value((double)x,(double)y);}
  mean.x=x;
  dim.x=2*x+1;
  y=1;
  value=gauss_value((double)x,(double)y);
  while(value>thresh){y++; value=gauss_value((double)x,(double)y);}
  mean.y=y;
  dim.y=2*y+1;
  initpsyimglnkpxl(NULL, dim.x, dim.y, 1, 1, pixeltype);
  double sum;
  getstats(NULL, NULL, NULL, &sum);
  scale=1.0/sum;
cout<<"std_dev=("<<std_dev.x<<','<<std_dev.y<<")\n";
}

void psygauss::getnextpixel(char *pixel)
{
  double value=
    gauss_value((double) getpixelloc.x, (double) getpixelloc.y);
  pixeltypechg((char *)&value, psydouble, pixel, type);
  incgetpixel();
}

double psygauss::gauss_value(double x, double y)
{
  x=(x - mean.x)/std_dev.x;
  y=(y - mean.y)/std_dev.y;
  return(scale * exp(-0.5*(x*x + y*y)));
}


class multiplycompleximgs : public proc2img {
  void proc2pixels(char *in1, psytype in1type,
		   char *in2, psytype in2type,
		   char *out, psytype outtype);
 public:
  multiplycompleximgs(psyimg *psy1imgptr, psyimg *psy2imgptr);
  void output_tree(ostream *out) {proc2img::output_tree(out);*out<<"::multiplycompleximgs";};
};

multiplycompleximgs::multiplycompleximgs(psyimg *psy1imgptr,
					psyimg *psy2imgptr) :
  proc2img(psy1imgptr, psy2imgptr)
{
  if(psy1imgptr->gettype() != psycomplex ||
     psy2imgptr->gettype() != psycomplex) {
    output_tree(&cerr);
    cerr<<"multiplycompleximgs::multiplycompleximgs - error both inputs must be complex\n";
    exit(1);
  }
}

void multiplycompleximgs::proc2pixels(char *in1, psytype in1type,
				      char *in2, psytype in2type,
				      char *out, psytype outtype)
{
  float *i1= (float *)in1;
  float *i2= (float *)in2;
  float *o= (float *)out;
  o[0]=i1[0]*i2[0]-i1[1]*i2[1];
  o[1]=i1[0]*i2[1]+i1[1]*i2[0];
}

class fftimg : public psypgbuff {
  int isign;
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  fftimg(psyimg *psyimgptr, int isign=1,
	 int prepadx=0, int prepady=0, int postpadx=0, int postpady=0);
  ~fftimg();
  void output_tree(ostream *out) {psypgbuff::output_tree(out);*out<<"::fftimg";};
};

fftimg::fftimg(psyimg *psyimgptr, int isign,
	       int prepadx, int prepady, int postpadx, int postpady)
{
  fftimg::isign=isign;
  psydims dims=psyimgptr->getsize();
  psydims origin=psyimgptr->getorig();
  psyres res=psyimgptr->getres();
// zero pad
  dims.x += prepadx+postpadx;
  dims.y += prepady+postpady;
  origin.x -= prepadx;
  origin.y -= prepady;
// increase size to a power of 2
  int s=2;
  while(s<dims.x)s *= 2;
  dims.x=s;
  s=2;
  while(s<dims.y)s *= 2;
  dims.y=s;
  double wordres = psyimgptr->getwordres();
  // inverse fft scales the results by the fft size
  if(isign == -1) wordres /= dims.x * dims.y;
// set pgbuff values
  initpgbuff(psyimgptr, dims.x, dims.y, dims.z, dims.i, psycomplex,
	     origin.x, origin.y, origin.z, orig.i, 0,
	     res.x, res.y, res.z, res.i, wordres);
}

fftimg::~fftimg()
{
}

void fftimg::fillpage(char *buff, int z, int i)
{
// intitial buffer to zeros
  char *endptr=buff+inc.z;
  for(char *ptr=buff; ptr<endptr; ptr++)*ptr=0;
  psydims inorig=inputpsyimg->getorig();
  psydims inend=inputpsyimg->getend();
// copy the input data into the fft buffer
  inputpsyimg->copyblock(buff+offset(inorig.x, inorig.y, inorig.z, inorig.i),
			 inorig.x, inorig.y, z, i,
			 inend.x, inend.y, z, i,
			 inc.x, inc.y, inc.z, inc.i, type);
// call routine to do actual work
  dimension nn[2];
  nn[0]=size.x; nn[1]=size.y;
  fourn((float *)buff, nn, 2, isign);
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  psytype outtype=psynotype; //defaults to input type
  psyfileclass outfileclass=psynoclass; //defaults to in file format
  int *outfileclassinfo = NULL;
  int avgsize=0;
  int no_extra_pad=0;
  xydouble std_dev; // standard deviation of gaussian in pixels;
  std_dev.x=std_dev.y=0;
// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-avg size] [-gs stddev]"<<'\n';
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-avg", argv[i])==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&avgsize);
    else if((strcmp("-gs", argv[i])==0)&&((i+1)<argc))
      {sscanf(argv[++i],"%lf",&std_dev.x); std_dev.y=std_dev.x;}
    else if(strcmp("-no_extra_pad", argv[i])==0)no_extra_pad=1;
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
// buffer the input
  psypgbuff inimagbuff(inputimgptr, 1);
  psyimg *psyimgptr = (psyimg *) &inimagbuff;
// save origin and end locations
  psydims inorig=psyimgptr->getorig();
  psydims inend=psyimgptr->getend();
  psydims insize=psyimgptr->getsize();
// create filter
  psyimg *psyimgfltr=NULL;
  if(avgsize > 1) {
    psyimgfltr=(psyimg *)new psyimgconstant(1.0/(avgsize*avgsize),
					    avgsize, avgsize, 1, 1, psyfloat,
					    inorig.x, inorig.y,
					    inorig.z, inorig.i);
  }
  else if(std_dev.x > 0 || std_dev.y > 0) {
    psyimgfltr=(psyimg *)new psygauss(std_dev, 0.01, psyfloat);
//    scaleimg scaledtmp(psyimgfltr, psyshort);
//    scaledtmp.set_scale_factor_for_max_res();
//    psynewoutfile("gsimage", (psyimg *)&scaledtmp, outfileclass, outtype);
  }
  else {
    cerr<<argv[0]<<": no filter specified\n";
    exit(1);
  }
  psydims fltrsize=psyimgfltr->getsize();
// perform fft on image
  int postpadx=fltrsize.x; int postpady=fltrsize.y;
  if(no_extra_pad){ postpadx=0; postpady=0; }
  fftimg fftedimg(psyimgptr, 1, 0, 0, postpadx, postpady);
  psyimgptr=(psyimg *)&fftedimg;
  psydims fftsize=psyimgptr->getsize();
// check sizes
  cout<<"image size=("<<insize.x<<','<<insize.y<<','<<insize.z<<','
    <<insize.i<<")\n";
  cout<<"filter size=("<<fltrsize.x<<','<<fltrsize.y<<")\n";
  cout<<"fft size=("<<fftsize.x<<','<<fftsize.y<<','<<fftsize.z<<','
    <<fftsize.i<<")\n";
  if((insize.x+fltrsize.x) > fftsize.x || (insize.y+fltrsize.y) > fftsize.y) {
    cout<<
      "warning - insufficient padding - output may have wrapping effects\n";
  }
  if(fltrsize.x > fftsize.x || fltrsize.y > fftsize.y) {
    cerr << argv[0] << ": filter larger then fft\n";
    exit(1);
  }
// pad filter to size of fftimage before shifting
  psydims prepadfltr; prepadfltr.x=prepadfltr.y=prepadfltr.z=prepadfltr.i=0;
  psydims postpadfltr=fftsize-fltrsize; postpadfltr.z=postpadfltr.i=0;
  padimage fltrpadded(psyimgfltr, prepadfltr, postpadfltr);
  psyimgfltr=(psyimg *)&fltrpadded;
// shift filter centering it around zero - wont have to shift output
  psydims shift;
  shift.x= -fltrsize.x/2; shift.y= -fltrsize.y/2;
  shift.z=0; shift.i=0;
  psyshiftimg fltrshfted(psyimgfltr, shift);
  psyimgfltr=(psyimg *)&fltrshfted;
// perform fft on filter which has already been padded to prior fft size
  fftimg fftbox(psyimgfltr, 1);
  psyimgfltr=(psyimg *)&fftbox;
// duplicate z-planes to filter all planes
  psyimgdupplanes fullfftbox(psyimgfltr, fftsize.z);
  psyimgfltr=(psyimg *)&fullfftbox;
// multiply fft of image and filter
  multiplycompleximgs multiplied(psyimgptr, psyimgfltr);
  psyimgptr=(psyimg *)&multiplied;
// invert fft
  fftimg fftedimginv(psyimgptr, -1);
  psyimgptr=(psyimg *)&fftedimginv;
// reduce image to original box
  psyimgblk outregion(psyimgptr, inorig, inend);
  psyimgptr=(psyimg *)&outregion;
// convert from complex to output type - imaginary part ignored
// while correcting for inverse fft scaling
  scaleimg scaled(psyimgptr, outtype, 1.0/(fftsize.x*fftsize.y));
//scaled.setwordres(psyimgptr->getwordres());
  psyimgptr=(psyimg *) &scaled;
// build new description
  char *program_desc=" fft filtered image: ";
  char *desc = new char[strlen(infile) + strlen(program_desc) + 1];
  strcpy(desc, program_desc);
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
cout<<"output word res="<<outpsyimgptr->getwordres()<<"\n";
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// print output filtered images stats
  cout<<"output image stats:";
  outpsyimgptr->showstats();

  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(inputimgptr != NULL)delete inputimgptr;
}




#define PI M_PI
inline void SWAP(float *a, float *b) {float temp= *a; *a = *b; *b=temp;};

void fourn(float data[], dimension nn[], int ndim, int isign)
{
// this routine is taken derived from "Numerical Recipes in C" 2nd edition
//
// Changed nn to run from 0 to ndims-1
// calculate total number of complex values
// Change data so externally it runs from 0 to ntot-1
// here it will run from 1 to ntot
  data = data - 1;
  int idim=0;
  dimension ntot=1;
  dimension n, nrem, ip1, ip2, ip3, i2rev, i1, i2, i3, i3rev;
  for(; idim<ndim; idim++)ntot *= nn[idim];  // calcs total size

// main loop over dimensions
  dimension nprev=1;  // stores combined size of all previous looped dims
  dimension ifp1, ifp2, k1, k2, ibit;
  double theta, wtemp, wpr, wpi, wr, wi;
  float tempr, tempi;

  for(idim=ndim-1; idim>=0; idim--){  // start loop with slowest changer
    n=nn[idim];
    nrem=ntot/(n*nprev);  // total size of remaining faster dims
    ip1=nprev << 1;  // double previous slower dims for complex
    ip2=ip1*n;  // combined size of previous and current complex
    ip3=ip2*nrem; // total size complex
    i2rev=1;
// perform bit reversal swaping
    for(i2=1; i2<=ip2; i2+=ip1) {  // i2 counts by slower dims complex
      if(i2 < i2rev) {
	for(i1=i2; i1<=i2+ip1-2; i1+=2) {  // swap whole areas
	  for(i3=i1; i3<=ip3; i3+=ip2) {
	    i3rev=i2rev+i3-i2;
	    SWAP(data+i3, data+i3rev);
	    SWAP(data+i3+1, data+i3rev+1);
	  } //end for(... i3=i1; ...
	} //end for(... i1=i2; ...
      } //end if(i2 < i2rev)
// i2rev counts by slower dims complex in reverse bit order
// reverse counting done by setting upper bits to zero until it
// finds a bit not set then sets that bit
// check depends on counting values being odd and bits even
// in other words the lowest bit which can't be reset is always 1
// the actual lowest bit corresponds to real(1) or imaginary(0)
      ibit=ip2 >> 1;  // start by checking highest bit
      while(ibit >= ip1 && i2rev > ibit) { //not lowest bit & > current bit set
	i2rev -= ibit;  // unset current bit
	ibit >>= 1; // check next lower bit
      } //end while (ibit ...
      i2rev += ibit; // set the highest unset bit
    } //end for(... i2=1 ...
// Danielson-Lanczos algorithm
    ifp1=ip1;
    while(ifp1 < ip2) {
      ifp2=ifp1 << 1;
      theta=isign*6.28318530717959/(ifp2/ip1);
      wtemp=sin(0.5*theta);
      wpr= -2.0*wtemp*wtemp;
      wpi=sin(theta);
      wr=1.0;
      wi=0.0;
      for(i3=1; i3<=ifp1; i3+=ip1) {
	for(i1=i3; i1<=i3+ip1-2; i1+=2) {
	  for(i2=i1; i2<=ip3; i2+=ifp2) {
	    k1=i2;
	    k2=k1+ifp1;
	    tempr=(float)wr*data[k2]-(float)wi*data[k2+1];
	    tempi=(float)wr*data[k2+1]+(float)wi*data[k2];
	    data[k2]=data[k1]-tempr;
	    data[k2+1]=data[k1+1]-tempi;
	    data[k1] += tempr;
	    data[k1+1] += tempi;
	  } //end for(i2=i1 ...
	} //end for(i1=i3
// trig recurrence
	wr=(wtemp=wr)*wpr-wi*wpi+wr;
	wi=wi*wpr+wtemp*wpi+wi;
      } //end for(i3=1 ...
      ifp1=ifp2;
    } //end while(ifp1 < ip2)
    nprev *= n; // increment combined size of all previous looped dims
  } //end for(idim=ndim; ...
} //end fourn

