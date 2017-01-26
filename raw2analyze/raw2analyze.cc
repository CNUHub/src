#include "psyhdr.h"
#include "psyanalyze.h"

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  psytype pixeltype=psyuchar;
  psytype outtype;
  double min, max, mean;
  psydims dims;
  dims.x=dims.y=256; dims.z=dims.i=1;
  int skip_pixels=0;
  int swap=0;
  int res_set=0;
  int incx; int incx_set=0;
  int incy; int incy_set=0;
  int incz; int incz_set=0;
  int inci; int inci_set=0;
  xyzidouble res;
  res.x=res.y=res.z=res.i=1;
  double word_res=1;
  psyorient orientation=psytransverse;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile outfile [-x xdim] [-y ydim] [-z zdim] [-i idim]"<<'\n';
      cout<<"       [-xr xres] [-yr yres] [-zr zres] [-ir ires]"<<'\n';
      cout<<"       [-xi xinc] [-yi yinc] [-zi zinc] [-ii iinc]"<<'\n';
      cout<<"       [-wr word_res] [-skip skip_pixels] [-swap]\n";
      cout<<"       [-uchar | -char | -short | -ushort | -int | -uint | -float | -double]\n";
      cout<<"       [-tran | -sag | -cor]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-x", argv[i])==0)&&((i+1)<argc))dims.x=atoi(argv[++i]);
    else if((strcmp("-y", argv[i])==0)&&((i+1)<argc))dims.y=atoi(argv[++i]);
    else if((strcmp("-z", argv[i])==0)&&((i+1)<argc))dims.z=atoi(argv[++i]);
    else if((strcmp("-i", argv[i])==0)&&((i+1)<argc))dims.i=atoi(argv[++i]);
    else if((strcmp("-xi", argv[i])==0)&&((i+1)<argc)) {
      incx=atoi(argv[++i]); incx_set=1;
    }
    else if((strcmp("-yi", argv[i])==0)&&((i+1)<argc)) {
      incy=atoi(argv[++i]); incy_set=1;
    }
    else if((strcmp("-zi", argv[i])==0)&&((i+1)<argc)) {
      incz=atoi(argv[++i]); incz_set=1;
    }
    else if((strcmp("-ii", argv[i])==0)&&((i+1)<argc)) {
      inci=atoi(argv[++i]); inci_set=1;
    }
    else if((strcmp("-xr", argv[i])==0)&&((i+1)<argc)){
      res.x=atof(argv[++i]);
      if(!res_set) { res.y=res.z=res.x; res_set=1; }
    }
    else if((strcmp("-yr", argv[i])==0)&&((i+1)<argc)){
      res.y=atof(argv[++i]);
      if(!res_set) { res.x=res.z=res.y; res_set=1; }
    }
    else if((strcmp("-zr", argv[i])==0)&&((i+1)<argc)){
      res.z=atof(argv[++i]);
      if(!res_set) { res.x=res.y=res.z; res_set=1; }
    }
    else if((strcmp("-ir", argv[i])==0)&&((i+1)<argc))res.i=atof(argv[++i]);
    else if((strcmp("-wr", argv[i])==0)&&((i+1)<argc))word_res=atof(argv[++i]);
    else if((strcmp("-skip", argv[i])==0)&&((i+1)<argc))
      skip_pixels=atoi(argv[++i]);
    else if(strcmp("-swap", argv[i])==0)swap=1;
    else if(strcmp("-uchar", argv[i])==0)pixeltype=psyuchar;
    else if(strcmp("-char", argv[i])==0)pixeltype=psychar;
    else if(strcmp("-short", argv[i])==0)pixeltype=psyshort;
    else if(strcmp("-ushort", argv[i])==0)pixeltype=psyushort;
    else if(strcmp("-int", argv[i])==0)pixeltype=psyint;
    else if(strcmp("-uint", argv[i])==0)pixeltype=psyuint;
    else if(strcmp("-float", argv[i])==0)pixeltype=psyfloat;
    else if(strcmp("-double", argv[i])==0)pixeltype=psydouble;
    else if(strcmp("-tran", argv[i])==0)orientation=psytransverse;
    else if(strcmp("-sag", argv[i])==0)orientation=psysagittal;
    else if(strcmp("-cor", argv[i])==0)orientation=psycoronal;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }
  }
  outtype=pixeltype;
  if(swap) {
    if(pixeltype == psyshort)pixeltype=psyshortsw;
    else {
      cerr<<"raw2analyze: sorry, can only swap short pixels\n";
      exit(1);
    }
  }
  if(outtype == psyushort) outtype=psyint;
  if(outtype == psyuint) outtype=psydouble;
// open input files
  rawfile inputimg(infile, "r", dims.x, dims.y, dims.z, dims.i, pixeltype,
		   0,0,0,0,skip_pixels,res.x,res.y,res.z,res.i,word_res);
  if(incx_set || incy_set || incz_set || inci_set) {
    if(! incx_set) inputimg.getinc(&incx, NULL, NULL, NULL);
    if(! incy_set) inputimg.getinc(NULL, &incy, NULL, NULL);
    if(! incz_set) inputimg.getinc(NULL, NULL, &incz, NULL);
    if(! inci_set) inputimg.getinc(NULL, NULL, NULL, &inci);
    inputimg.setinc(incx, incy, incz, inci);
  }
  inputimg.setorient(orientation);
  cout<<"input psyimg stuff:\n";
  inputimg.showpsyimg();
// output image
  outputanalyze outputimg(outfile, (psyimg *)&inputimg, outtype);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

  cout<<"\noutput image info:\n";
  outputimg.showpsyimg();
  cout<<"output stats:\n";
  outputimg.showstats();
  cout<<"output analyze headar:\n";
  outputimg.showhdr();
}
