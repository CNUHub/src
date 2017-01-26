#include "psyhdr.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean;
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
      cout <<"       [-auto | -s scale_factor] [-set[_new_quantification]]\n";
      cout <<"       [-ge min_thresh | -gf min_thresh_factor [-minv min_value]]\n";
      cout <<"       [-le max_thresh [-maxv max_value]]\n";
      cout <<"       [-t translation]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
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
// open input files
  psyfileclass infileclass;
  psytype infiletype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &infiletype,
			      &outfileclassinfo);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;

// buffer input files
  psypgbuff inputbuffered(inimag, 1);

// store input quantification value
  double wordres=inputbuffered.getwordres();

  psyimg *inputscaled;
  scaleimg *scalednormal=NULL;
  psytype internaltype=outtype;
// must maintain type when outputting ecat files
  if(outfileclass == ecatfileclass) {
    internaltype=inputbuffered.gettype();
    if(auto_scale){
      cerr << argv[0] << ": error: auto scaling not applicable when outputting ecat files\n";
      exit(1);
    }
  }

  if((outfileclass != ecatfileclass) ||
     inputed_max_thresh || inputed_min_thresh ||
     (translation != 0) ){
// scale image
    scalednormal=new scaleimg((psyimg *)&inputbuffered, internaltype,
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
    inputscaled=(psyimg *)scalednormal;
  }
  else {
// ecat output files can be scaled just by resetting the wordres
    inputscaled=(psyimg *)&inputbuffered;
    if(set_new_quantification)
      inputscaled->setwordres(inputscaled->getwordres()*scale_factor);
    else {
      cerr << argv[0] << ": warning: scaling ignored unless \"-set\" option used when outputting ecat files\n";
    }
  }

// build new description
  desc = new char[strlen(infile) + 15];
  strcpy(desc, "scaleimg: ");
  strcat(desc, infile);
  inputscaled->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  inputscaled->setdate();
  inputscaled->settime();
// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, inputscaled,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
  delete outpsyimgptr;
  delete inimag;
  if(scalednormal!=NULL)delete scalednormal;
}
