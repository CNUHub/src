#include "psyhdr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  int *outfileclassinfo = NULL;
  psytype outtype=psynotype; //defaults to input type
  psyfileclass outfileclass=psynoclass; //defaults to in file format
  double min, max=0.0, mean;
  char *desc;
  char *template_file=NULL;
  int defaultThresholding = 1;
  int threshold_percent_set = 0;
  double threshold_percent=40;
  int thresholdset=0;
  double threshold=0;
  int nofill=0;
  psyimg *templateptr=NULL;
  psyimg *templateimgptr=NULL;
  psyimg *template_img_buff_ptr=NULL;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile\n";
      cout <<"       [-tf template_file]"<<'\n';
      cout <<"       [-tp threshold_percent | -t threshold]"<<'\n';
      cout <<"       [-nofill]"<<'\n';
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-nofill")==0) nofill = 1;
    else if((strcmp(argv[i],"-tf")==0)&&((i+1)<argc)) {
      template_file=argv[++i];
      defaultThresholding = 0;
    }
    else if((strcmp(argv[i],"-tp")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%lf",&threshold_percent);
      threshold_percent_set = 1;
    }
    else if((strcmp(argv[i],"-t")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%lf",&threshold);
      thresholdset=1;
    }
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input file
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outtype == psynotype)outtype=intype;
  if(outfileclass == psynoclass)outfileclass=infileclass;

// buffer the input
  psypgbuff inimagbuff(inimag, 1);

// select the template source
  if(template_file != NULL) templateptr = (psyimg *) psynewinfile(template_file);
  else templateptr = inimag;
// buffer the template source
  templateptr= (psyimg *) new psypgbuff(templateptr, 1);
// determine theshold from percent of max
  if((defaultThresholding || threshold_percent_set) && !thresholdset) {
    templateptr->getstats(NULL, &max, NULL);
    cout<<"max="<<max<<"\n";
    cout<<"calculating threshold as "<< threshold_percent <<"% of max\n";
    threshold=threshold_percent * .01 * max;
    thresholdset = 1;
  }
// rework template according to thresholds
  if(thresholdset) {
    cout<<"threshold="<<threshold<<"\n";
    if(template_file != NULL) cout<<"thresholding template file\n";
    if(nofill) {
      cout<<"thresholding with no fill\n";
      scaleimg *scalethresholdimg = new scaleimg(templateptr, psyuchar);
      // template pixels >= threshold set to 1
      // and pixels < threshold set to 0
      // Note - scaleimg compares min thresh first so
      // all pixels < thresholded set to 0
      scalethresholdimg->set_min_thresh(threshold, 0);
      // reduce the threshold for max so that 
      // all pixels not thresholded to 0 are set to 1
      scalethresholdimg->set_max_thresh(threshold-1, 1);
      templateptr = (psyimg *) scalethresholdimg;
    }
    else {
      cout<<"thresholding with x and y inward filling\n";
      templateptr=(psyimg *) new bldtemplate(templateptr, threshold, 1);
    }
  }

// template the input image
  templateimg templated((psyimg *)&inimagbuff, templateptr, 1);

// build new description
  desc = new char[strlen(infile) + 15];
  strcpy(desc, "templateimg: ");
  strcat(desc, infile);
  templated.setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  templated.setdate();
  templated.settime();
// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg*)&templated,
				     outfileclass, outtype, outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
  if(template_file != NULL)log.loginfilelog(template_file);
// print out templated images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';

// clean up
  if(templateptr)delete templateptr;
  if(template_img_buff_ptr)delete template_img_buff_ptr;
  if(templateimgptr)delete templateimgptr;
  delete outpsyimgptr;
  delete inimag;
}
