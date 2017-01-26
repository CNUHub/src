#include "psyhdr.h"
//#include <strstream.h>
#include <sstream>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  char *templatefile=NULL;
  psyimg *templateptr = NULL;
  templateimg *templatedimgptr = NULL;
  psypgbuff *templatebuffered = NULL;
  double min, max, mean;
  int npixels;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file format
  int *outfileclassinfo = NULL;
  scaleimg *normedimg;
  double threshfactor=0;
  double threshold=0;
  double translation=0;
  int thresh_input_flag=0;
  int threshfactor_input_flag=0;
  int translate_flag=0;
  int zero_unused_voxels_flag=0;
  double newmean=1000;
  double scalefactor;
  char *desc;
  //  ostrstream *logostr = new ostrstream();
  std::stringstream *logostr = new std::stringstream();

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile"<<'\n';
      cout <<"      [-tf template_file |\n";
      cout <<"       -tpn threshold_percent |\n";
      cout <<"       -tn threshold]"<<'\n';
      cout <<"      [-m mean{"<<newmean<<"}]\n";
      cout <<"      [-tran] [-zero[_unused_voxels]]\n";
      cout <<"      ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"      ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-tpn", argv[i])==0)&&(i+1<argc)) {
      threshfactor_input_flag=1;
      threshfactor=0.01 * atof(argv[++i]);
    }
    else if((strcmp("-tn", argv[i])==0)&&(i+1<argc)) {
      thresh_input_flag=1;
      threshold=atof(argv[++i]);
    }
    else if((strcmp(argv[i],"-tf")==0)&&((i+1)<argc))
      templatefile = argv[++i];
    else if((strcmp("-m", argv[i])==0)&&(i+1<argc))
      newmean=atof(argv[++i]);
    else if(strcmp("-tran", argv[i])==0)translate_flag=1;
    else if(strncmp("-zero", argv[i], 5)==0)zero_unused_voxels_flag=1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// sanity checks
  if(infile == NULL) {
    cerr<<argv[0]<< ": error - no input file specified\n";
    exit(1);
  }
  if(outfile == NULL) {
    cerr<<argv[0]<< ": error - no output file specified\n";
    exit(1);
  }
  if(((templatefile != NULL) + thresh_input_flag +
      threshfactor_input_flag) > 1) {
    cerr<<argv[0]<< ": error - -tf, -tn and -tpn incompatible\n";
    exit(1);
  }
// open input file
//  analyzefile inimag(infile, "r");
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outtype==psynotype)outtype=intype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// buffer input file
  psypgbuff inimagbuff(inimag, 1);
  psyimg *imgptr = &inimagbuff;

  if(threshfactor_input_flag) {
    // get non-thresholded statistics to calculate threshold
    imgptr->getstats(&min, &max, &mean);
    threshold = max * threshfactor;
    (*logostr)<<"threshold="<<max<<"*"<<threshfactor<<"="<<threshold<<'\n';
  }

// get current mean
  if(templatefile != NULL) {
    templateptr = psynewinfile(templatefile);
    // buffer input file
    templatebuffered = new psypgbuff(templateptr, 1);
    templatedimgptr = new templateimg(imgptr, templatebuffered, 1);
    templatedimgptr->gettemplatedstats(&min, &max, &mean, &npixels);
    threshold = min; // for translation if selected
    (*logostr)<<"scale factor based on mean over template area\n";
    if(zero_unused_voxels_flag) {
      imgptr = templatedimgptr;
      (*logostr)<<"zeroing input voxels not passing template\n";
    }
  }
  else if(threshfactor_input_flag || thresh_input_flag) {
// get thresholded statistic to caclulate mean adjustment
    cout<<"scale factor based on mean for area above threshold="<<threshold<<"\n";
    imgptr->getthreshstats(&min, &max, &mean, &npixels, threshold);
    (*logostr)<<"scale factor based on threshold="<<threshold<<"\n";
  }
  else {
    imgptr->getstats(&min, &max, &mean);
    threshold = min; // for translation if selected
    (*logostr)<<"scale factor based on mean of whole image\n";
  }

// translate by negative threshold
  if(translate_flag)translation= -threshold;

  scalefactor = newmean/(mean+translation);
  (*logostr)<<"mean="<<mean<<" translation="<<translation<<" new mean="<<newmean<<"\n";
  (*logostr)<<"scale factor="<<scalefactor<<"\n";

// normalize input file
  normedimg= new scaleimg(imgptr, psydouble,
			  scalefactor, translation);
// zero thresholded unused values
  if(zero_unused_voxels_flag &&
     (threshfactor_input_flag || thresh_input_flag)) {
    (*logostr)<<"zeroing input voxels below threshold\n";
    normedimg->set_min_thresh(threshold,0);
  }
// set unitary word resolution to maintain normalized values
  normedimg->setwordres(1.0);

// build new description
  desc = new char[strlen(infile) + 20];
  strcpy(desc, "normmean: ");
  strcat(desc, infile);
  normedimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  normedimg->setdate();
  normedimg->settime();

  normedimg->getstats(&min, &max, &mean);
  cout<<"normed image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg*)normedimg,
				     outfileclass, outtype,
				     outfileclassinfo);
// print out differenced images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  (*logostr)<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';  
// log
  logfile log(outfile, argc, argv);
// log messages
  (*logostr)<<'\0';
  //logostr->freeze(1); log.logmessage(logostr->str()); logostr->freeze(0);
  log.logmessage((char *) logostr->str().c_str());
// log input files
  log.loginfilelog(infile);
  if(templatefile != NULL) log.loginfilelog(templatefile);

// clean up
  delete logostr;
  if(templatedimgptr != NULL) delete templatedimgptr;
  if(templateptr != NULL) delete templateptr;
  if(templatebuffered != NULL) delete templatebuffered;
  delete normedimg;
  delete outpsyimgptr;
  delete inimag;
}
