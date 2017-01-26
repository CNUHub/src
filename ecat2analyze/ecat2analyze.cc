#include "psyhdr.h"
#include "psyanalyze.h"
#include "psyecat.h"


class psyltc : public psypgbuff {
  double *offset;
  double *slope;
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  psyltc(psyimg *psyimgptr, char *ltc_fname);
  virtual ~psyltc();
};

psyltc::psyltc(psyimg *psyimgptr, char *ltc_fname)
     : psypgbuff(psyimgptr, 1)
{
  fstream ltcfd;
  psyopenfile(ltc_fname, "r", &ltcfd);
  int zdim;
  getsize(NULL, NULL, &zdim, NULL);
  offset = new double[zdim];
  slope = new double[zdim];
// skip header info - 3 lines
  char garbage[128], c;
  ltcfd.get(garbage, 127); if(ltcfd.peek()=='\n')ltcfd.get(c);
  cout << garbage << '\n';
  ltcfd.get(garbage, 127); if(ltcfd.peek()=='\n')ltcfd.get(c);
  cout << garbage << '\n';
  ltcfd.get(garbage, 127); if(ltcfd.peek()=='\n')ltcfd.get(c);
  cout << garbage << '\n';
// read constants
  int plane;
  for(int i=0; i<zdim; i++) {
    ltcfd >> plane >> offset[i] >> slope[i];
    if(ltcfd.bad() || ltcfd.fail()) {
      cerr<<"psyltc::psyltc - problem reading plane "<<i+1<<" of ltc file \"";
      cerr<<ltc_fname<<"\"\n";
      exit(1);
    }
    cout<<"plane="<<plane<<" offset="<<offset[i]<<" slope="<<slope[i]<<'\n';
  }
  ltcfd.close();
}

psyltc::~psyltc()
{
  if(offset != NULL)delete[] offset;
  if(slope != NULL)delete[] slope;
}

void psyltc::fillpage(char *buff, int z, int i)
{
  int x, y;
  char *xoptr, *yoptr;
  double dpixel;
  char_or_largest_pixel pixel;

  psytype intype=inputpsyimg->gettype();
  inputpsyimg->initgetpixel(intype, orig.x, orig.y, z, i);
  yoptr=buff;
  for(y=orig.y; y<=end.y; y++, yoptr+=inc.y) {
    for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
      inputpsyimg->getnextpixel(pixel.c);
      type2double(pixel.c, intype, (char *)&dpixel); 
      dpixel = dpixel * slope[z] + offset[z];
      pixeltypechg((char *)&dpixel, psydouble, xoptr, type);
    } // end for x
  } // end for y
  inputpsyimg->freegetpixel();
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL, *ltc_fname=NULL;
  psytype outtype=psynotype; // defaults to short with scaling
  int flip=1;
  int scale=1;
  int debug=0;
  psydims beg, end;
  beg.x = beg.y = beg.z = beg.i = -1;
  end.x = end.y = end.z = end.i = -1;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile outfile [-noflip]"<<'\n';
      cout<<"       [-short | -uchar | -int | -float | -double]\n";
      cout<<"       [-xb xbeg] [-xe xend] [-yb ybeg] [-ye yend]\n";
      cout<<"       [-zb zbeg] [-ze zend] [-ib ibeg] [-ie iend]\n";
      cout<<"       [-ltc ltc_fname] [-debug] [-noscale]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-debug", argv[i])==0)debug=1;
    else if(strcmp("-noflip", argv[i])==0)flip=0;
    else if(strcmp("-noscale", argv[i])==0)scale=0;
    else if(strcmp("-uchar", argv[i])==0)outtype=psyuchar;
    else if(strcmp("-short", argv[i])==0)outtype=psyshort;
    else if(strcmp("-int", argv[i])==0)outtype=psyint;
    else if(strcmp("-float", argv[i])==0)outtype=psyfloat;
    else if(strcmp("-double", argv[i])==0)outtype=psydouble;
    else if((strcmp(argv[i],"-xb")==0)&&((i+1)<argc))beg.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-xe")==0)&&((i+1)<argc))end.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-yb")==0)&&((i+1)<argc))beg.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ye")==0)&&((i+1)<argc))end.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-zb")==0)&&((i+1)<argc))beg.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ze")==0)&&((i+1)<argc))end.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ib")==0)&&((i+1)<argc))beg.i=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ie")==0)&&((i+1)<argc))end.i=atoi(argv[++i]);
    else if((strcmp("-ltc", argv[i])==0)&&((i+1)<argc))ltc_fname=argv[++i];
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }
  }
// open input files
  ecatfile inputimg(infile);
  psypgbuff inputbuffered((psyimg *)&inputimg, 1);
  if(debug) {
    cout<<"input image header:\n";
    inputimg.showhdr();
    cout<<"input psyimg stuff:\n";
    inputimg.showpsyimg();
    cout<<"input stats:\n";
    inputbuffered.showstats();
  }
  psyimg *psyimgptr=(psyimg *)&inputbuffered;
// select subregion
// select block of image
  int blockimg=0;
  psyimgblk *subregion=NULL;
  psydims in_beg=psyimgptr->getorig();
  psydims in_end=psyimgptr->getend();
  if(beg.x < 0)beg.x=in_beg.x; else blockimg=1;
  if(end.x < 0)end.x=in_end.x; else blockimg=1;
  if(beg.y < 0)beg.y=in_beg.y; else blockimg=1;
  if(end.y < 0)end.y=in_end.y; else blockimg=1;
  if(beg.z < 0)beg.z=in_beg.z; else blockimg=1;
  if(end.z < 0)end.z=in_end.z; else blockimg=1;
  if(beg.i < 0)beg.i=in_beg.i; else blockimg=1;
  if(end.i < 0)end.i=in_end.i; else blockimg=1;
  if(blockimg){
    subregion=new psyimgblk(psyimgptr, beg.x, beg.y, beg.z, beg.i,
			    end.x, end.y, end.z, end.i, 1);
    psyimgptr=(psyimg *)subregion;
  }
// apply linear transform constants
  psyltc *psyltcptr=NULL;
  if(ltc_fname != NULL) {
    psyltcptr = new psyltc(psyimgptr, ltc_fname);
    psyimgptr = (psyimg *)psyltcptr;
    if(debug) {
      cout<<"ltc transformed stats:\n";
      psyimgptr->showstats();
    }
  }
  scaleimg *inputscaled=NULL;
  if(scale) {
// scale to output type with maximum resolution
    if(outtype == psynotype) outtype = psyshort;
    inputscaled = new scaleimg(psyimgptr, outtype, 1.0, 0.0);
    inputscaled->set_scale_factor_for_max_res();
    psyimgptr = (psyimg *)inputscaled;
  }
// reverse rows
  reverserows *reverserowsptr=NULL;
  if(flip){
    reverserowsptr=new reverserows(psyimgptr);
    psyimgptr = (psyimg *)reverserowsptr;
  }
// output image
  outputanalyze outputimg(outfile, psyimgptr, outtype);
  if(debug) {
    cout<<"\noutput image info:\n";
    outputimg.showpsyimg();
    cout<<"output stats:\n";
    outputimg.showstats();
    cout<<"output analyze headar:\n";
    outputimg.showhdr();
  }
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// clean up
  delete psyltcptr;
  delete inputscaled;
  delete reverserowsptr;
  delete subregion;
}
