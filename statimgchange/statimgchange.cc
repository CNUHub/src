#include "psyhdr.h"
#include "cnustats.h"
#include <stdio.h>
#include <math.h>

class convertz_to_prob : public proc1img {
  int keepNeg;
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
 public:
  convertz_to_prob(psyimg *psyimgptr, psytype pixeltype=psydouble, int keepNeg=0);
  void initconvertz_to_prob(psyimg *psyimgptr, psytype pixeltype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::convertz_to_prob";};
};

convertz_to_prob::convertz_to_prob(psyimg *psyimgptr, psytype pixeltype, int keepNeg)
{
  initconvertz_to_prob(psyimgptr, pixeltype);
  convertz_to_prob::keepNeg = keepNeg;
}

void convertz_to_prob::initconvertz_to_prob(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
}

void convertz_to_prob::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  double ddpixel;
  double signFactor;
  type2double(in, intype, (char *)&dpixel);
  dpixel *= wordres;
  signFactor = 1.0;
  if(keepNeg && (dpixel < 0.0)) signFactor = -1.0;
  ddpixel = dpixel/(1.237+0.0249*dpixel);
  dpixel = (1-sqrt(1-exp(-1*pow(ddpixel, 2))));// /2;
  dpixel *= signFactor;
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class convertprob_to_z : public proc1img {
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
 public:
  convertprob_to_z(psyimg *psyimgptr, psytype pixeltype=psydouble);
  void initconvertprob_to_z(psyimg *psyimgptr, psytype pixeltype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::convertprob_to_z";};
};

convertprob_to_z::convertprob_to_z(psyimg *psyimgptr, psytype pixeltype)
{
  initconvertprob_to_z(psyimgptr, pixeltype);
}

void convertprob_to_z::initconvertprob_to_z(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
}

void convertprob_to_z::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  double ddpixel;
  double signFactor;

  type2double(in, intype, (char *)&dpixel);
  dpixel *= wordres;
  dpixel *= 0.5;
  signFactor = 1.0;
  if(dpixel < 0.0) {
    signFactor = -1.0;
    dpixel = fabs(dpixel);
  }
  if(dpixel < 1e-16) dpixel = 500.0;
  else if((1.0 - dpixel) < 1e-4) dpixel = 0.0;
  else {
    ddpixel = sqrt(-1*log(1-pow((2*dpixel-1), 2)));
    dpixel = 1.237*ddpixel/(1-0.0249*ddpixel);
  }
  dpixel *= signFactor;
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class raise_to_power : public proc1img {
  double power;
  void convertpixel(char *in, psytype intype, char *out, psytype outtype);
 public:
  raise_to_power(psyimg *psyimgptr, double power, psytype pixeltype=psydouble);
  void initraise_to_power(psyimg *psyimgptr, psytype pixeltype);
  void output_tree(ostream *out) {proc1img::output_tree(out);*out<<"::raise_to_power";};
};

raise_to_power::raise_to_power(psyimg *psyimgptr, double power, psytype pixeltype)
{
  raise_to_power::power = power;
  initraise_to_power(psyimgptr, pixeltype);
}

void raise_to_power::initraise_to_power(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
}

void raise_to_power::convertpixel(char *in, psytype intype, char *out,
			   psytype outtype)
{
  double dpixel;
  type2double(in, intype, (char *)&dpixel);
  dpixel = pow(dpixel, power);
  pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class pcorr_to_tvalue : public proc2img {
  void proc2pixels(char *in1, psytype in1type,
		   char *in2, psytype in2type,
		   char *out, psytype outtype);
  double additionToDF;
 public:
  pcorr_to_tvalue(psyimg *pcorrimgptr, psyimg *dfimgptr, double additionToDF,
		  psytype pixeltype=psynotype);
  void output_tree(ostream *out) {proc2img::output_tree(out);*out<<"::pcorr_to_tvalue";};
};

pcorr_to_tvalue::pcorr_to_tvalue(psyimg *pcorrimgptr, psyimg *dfimgptr,
				     double additionToDF,
				     psytype pixeltype) :
				     proc2img(pcorrimgptr, dfimgptr, pixeltype)
{
  pcorr_to_tvalue::additionToDF = additionToDF;
}

#define TINY 1.0e-20
void pcorr_to_tvalue::proc2pixels(char *in1, psytype in1type,
			  char *in2, psytype in2type,
			  char *out, psytype outtype)
{
  double tvalue, pcorr, df;
  // convert to double to simplify math
  type2double(in1, in1type, (char *)&pcorr);
  type2double(in2, in2type, (char *)&df);
  // do actual processing
  df += additionToDF;
  if(df <= 0.0) tvalue = 0;
  else tvalue = pcorr * sqrt(df/( (1.0-pcorr+TINY)*(1.0+pcorr+TINY) ));
  // convert back to desired output type
  pixeltypechg((char *)&tvalue, psydouble, out, outtype);
}


int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  int do_z_to_prob=0;
  int do_prob_to_z=0;
  int do_power=0;
  int do_conjunct=0;
  double power=0;

  int do_t_to_prob=0;
  int do_t_to_z=0;
  int do_pcorr_to_t=0;
  int do_pcorr_to_prob=0;
  double constant_df=-1;
  char *cntfile=NULL;

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
  double testvalue=0;
  int test=0;
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
      cout <<"Usage: "<<argv[0]<<" infile | -test testvalue [outfile]"<<'\n';
      cout <<"       -z_to_prob | -prob_to_z | -pow power | -conjunct power \n";
      cout <<"       | -t_to_prob | -t_to_z | -pcorr_to_t | -pcorr_to_prob\n";
      cout <<"       [-df df | -cnt countfile]\n";
      cout <<"       [-auto | -s scale_factor] [-set[_new_quantification]]\n";
      cout <<"       [-ge min_thresh | -gf min_thresh_factor [-minv min_value]]\n";
      cout <<"       [-le max_thresh [-maxv max_value]]\n";
      cout <<"       [-t translation]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-test", argv[i])==0) && ((i+1)<argc)){
      test = 1;
      if(sscanf(argv[++i],"%lf", &testvalue)!=1)
      {
	cerr<<argv[0]<<": invalid test value="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if(strcmp("-z_to_prob", argv[i])==0) do_z_to_prob=1;
    else if(strcmp("-prob_to_z", argv[i])==0) do_prob_to_z=1;
    else if((strcmp("-pow", argv[i])==0) && ((i+1)<argc)){
      do_power = 1;
      if(sscanf(argv[++i],"%lf", &power)!=1)
      {
	cerr<<argv[0]<<": invalid power="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((strcmp("-conjunct", argv[i])==0) && ((i+1)<argc)){
      do_conjunct = 1;
      if(sscanf(argv[++i],"%lf", &power)!=1)
      {
	cerr<<argv[0]<<": invalid conjunct power="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if(strcmp("-t_to_prob", argv[i])==0) do_t_to_prob=1;
    else if(strcmp("-t_to_z", argv[i])==0) { do_t_to_prob=1; do_t_to_z=1; }
    else if(strcmp("-pcorr_to_prob", argv[i])==0) do_pcorr_to_prob=1;
    else if(strcmp("-pcorr_to_t", argv[i])==0) do_pcorr_to_t=1;
    else if((strcmp("-cnt", argv[i])==0) && ((i+1)<argc))
      cntfile = argv[++i];
    else if((strcmp("-df", argv[i])==0) && ((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf", &constant_df)!=1) {
	cerr<<argv[0]<<": invalid degrees of freedom="<<argv[i]<<'\n';
	exit(1);
      }
    }
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
  if((do_z_to_prob + do_prob_to_z) > 1) {
    cerr << argv[0] << ": only one operation allowed\n";
    exit(1);
  }
// only count file or df allowed
  if((cntfile != NULL) && (constant_df > -1)) {
    cerr << argv[0] << ": df not compatible with count file\n";
    exit(1);
  }

// open input file
  psyfileclass infileclass;
  psytype infiletype;
  psyimg *inimag;

  if(infile != NULL) inimag=psynewinfile(infile, &infileclass, &infiletype, &outfileclassinfo);
  else if(test) {
    inimag=new psyimgconstant(testvalue, 1, 1, 1, 1, psydouble);
    infileclass=ecatfileclass;
    infiletype=psydouble;
  }

  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// store input quantification value
  double wordres=inimag->getwordres();
// keep a running image pointer
  psyimg *imgptr=inimag;
// buffer input file
  psypgbuff inputbuffered(imgptr, 1);
  imgptr=(psyimg *)&inputbuffered;

  psyimg *dfimgptr = NULL;
  int dfaddition = 0;
  if(cntfile != NULL) {
// open count file
    psyimg *cntimg=psynewinfile(cntfile);
    dfimgptr=new psypgbuff(cntimg);
    dfaddition = -2;
  }

// operation
  psyimg *operatedimg=NULL;
  if(do_z_to_prob)
    operatedimg=(psyimg *)new convertz_to_prob(imgptr, psydouble);
  else if(do_prob_to_z)
    operatedimg=(psyimg *)new convertprob_to_z(imgptr, psydouble);
  else if(do_power)
    operatedimg=(psyimg *)new raise_to_power(imgptr, power, psydouble);
  else if(do_conjunct) {
    operatedimg=(psyimg *)new convertz_to_prob(imgptr, psydouble);
    operatedimg=(psyimg *)new raise_to_power(operatedimg, power, psydouble);
    operatedimg=(psyimg *)new convertprob_to_z(operatedimg, psydouble);
  }
  else if(do_t_to_prob) {
    if(dfimgptr != NULL) ;
    else if(constant_df > -1) {
      dfimgptr = new psyimgconstant(imgptr, constant_df, psydouble);
      dfaddition = 0;
    }
    else {
      cerr << argv[0] << ": t_to_prob/t_to_z require count file or df\n";
      exit(1);
    }
    if(do_t_to_z) {
      operatedimg=(psyimg *)new probfromtvalueimg(imgptr, dfimgptr,
						  dfaddition, psydouble, 1);
      operatedimg=(psyimg *)new convertprob_to_z(operatedimg, psydouble);
    }
    else operatedimg=(psyimg *)new probfromtvalueimg(imgptr, dfimgptr,
						     dfaddition, psydouble);
  }
  else if(do_pcorr_to_t || do_pcorr_to_prob) {
    if(dfimgptr != NULL) ;
    else if(constant_df > -1) {
      dfimgptr = new psyimgconstant(imgptr, constant_df, psydouble);
      dfaddition = 0;
    }
    else {
      cerr << argv[0] << ": pcorr_to_t or pcorr_to_prob require count file or df\n";
      exit(1);
    }
    operatedimg=(psyimg *)new pcorr_to_tvalue(imgptr, dfimgptr,
					      dfaddition, psydouble);
    if(do_pcorr_to_prob) {
      psypgbuff *dfimgbuffed = new psypgbuff(dfimgptr, 1);
      operatedimg=(psyimg *)new probfromtvalueimg(operatedimg, dfimgbuffed,
						  dfaddition, psydouble);
    }
  }
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
  if(infile != NULL) {
    desc = new char[strlen(infile) + 20];
    strcpy(desc, "statimgchange: ");
    strcat(desc, infile);
    imgptr->setdescription(desc);
    delete[] desc;
  }
// set date and time to current date and time
  imgptr->setdate();
  imgptr->settime();
// output result to file
  if(outfile != NULL) {
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
  }
  else {
    imgptr->getstats(&min, &max, &mean);
    cout<<"**** no file output **************\n";
    cout<<"results min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
    cout<<"**** no file output **************\n";
  }
  if(operatedimg!=NULL)delete operatedimg;
  delete inimag;
  if(scalednormal!=NULL)delete scalednormal;
}
