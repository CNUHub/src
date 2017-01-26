#include "psyhdr.h"

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
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
  int translate_flag=1;
  double newmean=1000;
  double scalefactor;
  char *desc;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile"<<'\n';
      cout <<"       [-t thresh{"<<threshold<<"} | -tf threshfactor]\n";
      cout <<"       [-m mean{"<<newmean<<"}] [-notran]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-tf", argv[i])==0)&&(i+1<argc)) {
      threshfactor_input_flag=0;
      threshfactor=atof(argv[++i]);
    }
    else if((strcmp("-t", argv[i])==0)&&(i+1<argc)) {
      thresh_input_flag=0;
      threshold=atof(argv[++i]);
    }
    else if((strcmp("-m", argv[i])==0)&&(i+1<argc))
      newmean=atof(argv[++i]);
    else if(strcmp("-notran", argv[i])==0)translate_flag=0;
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
  if(thresh_input_flag && threshfactor_input_flag) {
    cerr<<argv[0]<< ": error - -t and -tf incompatible\n";
    exit(1);
  }
// open input file
//  analyzefile inimag(infile, "r");
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outtype==psynotype)outtype=intype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// buffer input files
  psypgbuff inimagbuff(inimag, 1);

  if(threshfactor_input_flag) {
// get non-thresholded statistics to calculate threshold
    inimagbuff.getstats(&min, &max, &mean);
    threshold = max * threshfactor;
  }

// translate by negative threshold
  if(translate_flag)translation= -threshold;

// get thresholded statistic to caclulate mean adjustment
  inimagbuff.getthreshstats(&min, &max, &mean, &npixels, threshold);

  scalefactor = newmean/(mean+translation);
// normalize input file
  normedimg= new scaleimg((psyimg *)&inimagbuff, outtype,
			  scalefactor, translation);
// zero values below threshold
  if(threshfactor_input_flag || thresh_input_flag)
    normedimg->set_min_thresh(threshold,0);
// set unitary word resolution to maintain normalized values
  normedimg->setwordres(1.0);

// build new description
  desc = new char[strlen(infile) + 20];
  strcpy(desc, "normalizemean: ");
  strcat(desc, infile);
  normedimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  normedimg->setdate();
  normedimg->settime();

  normedimg->getthreshstats(&min, &max, &mean, &npixels, threshold+translation);
  cout<<"output threshold image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg*)normedimg,
				     outfileclass, outtype,
				     outfileclassinfo);
//  outputanalyze outputimag(outfile, (psyimg *)normedimg);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// print out differenced images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"whole output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';

// clean up
  delete normedimg;
  delete outpsyimgptr;
  delete inimag;
}
