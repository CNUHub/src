#include "psyhdr.h"
#include <stdio.h>
#include <math.h>

class square_img : public proc1img {
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
 public:
  square_img(psyimg *psyimgptr, psytype pixeltype);
  void initsquare_img(psyimg *psyimgptr, psytype pixeltype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::square_img";};
};

square_img::square_img(psyimg *psyimgptr, psytype pixeltype)
{
  initsquare_img(psyimgptr, pixeltype);
}

void square_img::initsquare_img(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
}

void square_img::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  type2double(in, intype, (char *)&dpixel);
  dpixel = dpixel*dpixel;
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class sqrt_img : public proc1img {
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
  double threshmin;
  double threshminvalue;
 public:
  sqrt_img(psyimg *psyimgptr, psytype pixeltype);
  void initsqrt_img(psyimg *psyimgptr, psytype pixeltype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::sqrt_img";};
  void set_min_thresh(double threshmin, double threshmin_value=0);
};

sqrt_img::sqrt_img(psyimg *psyimgptr, psytype pixeltype)
{
  initsqrt_img(psyimgptr, pixeltype);
}

void sqrt_img::initsqrt_img(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
  threshmin=0;
  threshminvalue= -1;
}

void sqrt_img::set_min_thresh(double threshmin, double threshmin_value)
{
  if(threshmin < 0) {
    output_tree(&cerr);
    cerr<<":sqrt_img::set_min_thresh - error threshmin < 0\n";
    exit(1);
  }
  threshmin=threshmin;
  threshminvalue=threshmin_value;
}

void sqrt_img::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  type2double(in, intype, (char *)&dpixel);
  if(dpixel < threshmin)dpixel=threshminvalue;
  else dpixel = sqrt(dpixel);
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class inv_img : public proc1img {
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
  double min_mag;
  double min_mag_value;
 public:
  inv_img(psyimg *psyimgptr, psytype pixeltype);
  void initinv_img(psyimg *psyimgptr, psytype pixeltype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::inv_img";};
  void set_min_mag(double min_mag, double min_mag_value=0);
};

inv_img::inv_img(psyimg *psyimgptr, psytype pixeltype)
{
  initinv_img(psyimgptr, pixeltype);
}

void inv_img::initinv_img(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
  min_mag=1e-8;
  min_mag_value=0;
}

void inv_img::set_min_mag(double min_mag, double min_mag_value)
{
  if(min_mag <= 0) {
    output_tree(&cerr);
    cerr<<":inv_img::set_min_mag - error min_mag <= 0\n";
    exit(1);
  }
  inv_img::min_mag=min_mag;
  inv_img::min_mag_value=min_mag_value;
}

void inv_img::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  type2double(in, intype, (char *)&dpixel);
  if(fabs(dpixel) < min_mag)dpixel=min_mag_value;
  else dpixel = 1.0/dpixel;
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class fabs_img : public proc1img {
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
 public:
  fabs_img(psyimg *psyimgptr, psytype pixeltype=psynotype);
  void initfabs_img(psyimg *psyimgptr, psytype pixeltype=psynotype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::fabs_img";};
};

fabs_img::fabs_img(psyimg *psyimgptr, psytype pixeltype)
{
  initfabs_img(psyimgptr, pixeltype);
}

void fabs_img::initfabs_img(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
}

void fabs_img::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  type2double(in, intype, (char *)&dpixel);
  dpixel = fabs(dpixel);
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}


int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  int do_sqrt=0;
  int do_square=0;
  int do_abs=0;
  int do_inv=0;
  double scale_factor=1.0;
  int auto_scale=0;
  int inputed_scale_factor=0;
  double min_thresh;
  int inputed_min_thresh=0;
  double min_thresh_factor=1.0;
  double min_value=0.0;
  double max_thresh;
  int inputed_max_thresh=0;
  double max_value=0.0;
  int inputed_max_value=0;
  double translation=0;
  char *desc;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file format
  int *outfileclassinfo = NULL;
  int set_new_quantification=0;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile"<<'\n';
      cout <<"       [-sqrt | -square | -abs | -inv]\n";
      cout <<"       [-auto | -s scale_factor] [-set[_new_quantification]]\n";
      cout <<"       [-ge min_thresh | -gf min_thresh_factor [-minv min_value]]\n";
      cout <<"       [-le max_thresh [-maxv max_value]]\n";
      cout <<"       [-t translation]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-sqrt", argv[i])==0) do_sqrt=1;
    else if(strcmp("-square", argv[i])==0) do_square=1;
    else if(strcmp("-abs", argv[i])==0) do_abs=1;
    else if(strcmp("-inv", argv[i])==0) do_inv=1;
    else if((strcmp("-auto", argv[i])==0) && !auto_scale &&
	    !inputed_scale_factor)auto_scale=1;
    else if((strcmp("-s", argv[i])==0) && ((i+1)<argc)){
      if(inputed_scale_factor || auto_scale ||
	 (sscanf(argv[++i],"%lf", &scale_factor)!=1)) {
	cerr<<argv[0]<<": invalid scale_factor="<<argv[i]<<'\n';
	exit(1);
      }
      inputed_scale_factor=1;
    }
    else if((strcmp("-ge", argv[i])==0) && ((i+1)<argc)){
      if(inputed_min_thresh||(sscanf(argv[++i],"%lf", &min_thresh)!=1)) {
	cerr<<argv[0]<<": invalid min_thresh="<<argv[i]<<'\n';
	exit(1);
      }
      inputed_min_thresh=1;
    }
    else if((strcmp("-gf", argv[i])==0) && ((i+1)<argc)){
      if(inputed_min_thresh||(sscanf(argv[++i],"%lf", &min_thresh_factor)!=1))
      {
	cerr<<argv[0]<<": invalid min_thresh_factor="<<argv[i]<<'\n';
	exit(1);
      }
      inputed_min_thresh=1;
    }
    else if((strcmp("-minv", argv[i])==0) && ((i+1)<argc)){
      if(sscanf(argv[++i],"%lf", &min_value)!=1)
      {
	cerr<<argv[0]<<": invalid min_value="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((strcmp("-le", argv[i])==0) && ((i+1)<argc)){
      if(inputed_max_thresh||(sscanf(argv[++i],"%lf", &max_thresh) != 1)) {
	cerr<<argv[0]<<": invalid max_thresh="<<argv[i]<<'\n';
	exit(1);
      }
      inputed_max_thresh=1;
    }
    else if((strcmp("-maxv", argv[i])==0) && ((i+1)<argc)){
      if(sscanf(argv[++i],"%lf", &max_value)!=1)
      {
	cerr<<argv[0]<<": invalid max_value="<<argv[i]<<'\n';
	exit(1);
      }
      inputed_max_value=1;
    }
    else if((strcmp("-t", argv[i])==0) && ((i+1)<argc)){
      if((translation!=0)||(sscanf(argv[++i],"%lf", &translation) != 1)) {
	cerr<<argv[0]<<": invalid translation="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(strncmp("-set_new_quantification", argv[i], 4)==0)
      set_new_quantification=1;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": invalid parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// only one operation allowed
  if((do_sqrt+do_square+do_abs+do_inv) > 1) {
    cerr << argv[0] << ": only one operation allowed\n";
    exit(1);
  }
// open input files
  psyfileclass infileclass;
  psytype infiletype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &infiletype, &outfileclassinfo);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// store input quantification value
  double wordres=inimag->getwordres();
// keep a running image pointer
  psyimg *imgptr=inimag;
// buffer input file
  psypgbuff inputbuffered(imgptr, 1);
  imgptr=(psyimg *)&inputbuffered;
// operation
  psyimg *operatedimg=NULL;
  if(do_sqrt)operatedimg=(psyimg *)new sqrt_img(imgptr, psydouble);
  else if(do_square)operatedimg=(psyimg *)new square_img(imgptr, psydouble);
  else if(do_abs)operatedimg=(psyimg *)new fabs_img(imgptr);
  else if(do_inv)operatedimg=(psyimg *)new inv_img(imgptr, psydouble);
  else {
    cerr << argv[0] << ": requires one operation\n";
    exit(1);
  }
  imgptr=operatedimg;

  psytype scaledtype=outtype;
// must maintain type when outputting ecat files
  if(outfileclass == ecatfileclass) {
    scaledtype=inimag->gettype();
    if(auto_scale){
      cerr << argv[0] << ": error: auto scaling not applicable when outputting ecat files\n";
      exit(1);
    }
  }

  scaleimg *scalednormal=NULL;
  double min, max, mean;
  if((outfileclass != ecatfileclass) ||
     inputed_max_thresh || inputed_min_thresh){
// scale image
    scalednormal=new scaleimg((psyimg *)imgptr, outtype,
			      scale_factor, translation);
// set thresholds
    if(inputed_min_thresh) {
      if(min_thresh_factor != 1.0) {
	inputbuffered.getstats(&min, &max, &mean);
	min_thresh= max * min_thresh_factor;
      }
      scalednormal->set_min_thresh(min_thresh,min_value);
    }
    if(inputed_max_thresh) {
      if(!inputed_max_value)max_value=max_thresh;
      scalednormal->set_max_thresh(max_thresh,max_value);
    }
// set best scale factor based on image min, max, thresh and translation
    if(auto_scale)scalednormal->set_scale_factor_for_max_res();
// set new quantification factor
    if(set_new_quantification) {
// this is the same as scaling the image and keeping the old word res
      scalednormal->setwordres(wordres);
    }
    imgptr=(psyimg *)scalednormal;
  }
  else {
// ecat output files can be scaled just by resetting the wordres
    if(set_new_quantification)
      imgptr->setwordres(imgptr->getwordres()*scale_factor);
    else if(scale_factor != 1.0){
      cerr << argv[0] << ": warning: scaling ignored unless \"-set\" option used when outputting ecat files\n";
    }
  }

// build new description
  desc = new char[strlen(infile) + 15];
  strcpy(desc, "imgmath: ");
  strcat(desc, infile);
  imgptr->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  imgptr->setdate();
  imgptr->settime();
// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, imgptr,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
  delete outpsyimgptr;
  if(operatedimg!=NULL)delete operatedimg;
  delete inimag;
  if(scalednormal!=NULL)delete scalednormal;
}
