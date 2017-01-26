#include "psyhdr.h"

class diff2imgs : public proc2img {
  void proc2pixels(char *in1, psytype in1type,
		   char *in2, psytype in2type,
		   char *out, psytype outtype);
 protected:
  double thresh1;
  double thresh2;
  double scale1;
  double scale2;
 public:
  diff2imgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
	    double threshfactor, psytype pixeltype=psynotype);
};

diff2imgs::diff2imgs(psyimg *psy1imgptr, psyimg *psy2imgptr,
		     double threshfactor, psytype pixeltype) :
       proc2img(psy1imgptr, psy2imgptr, pixeltype)
{
  double min, max, mean;
  int pixelsused;
// get image stats to set threshold
  psy1imgptr->getstats(&min, &max, &mean);
  cout<<"image 1 min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  thresh1= threshfactor * max;
// get image stats using threshold to get scale factor
  psy1imgptr->getthreshstats(&min, &max, &mean, &pixelsused, thresh1);
  if(fabs(mean) < 1e-16)scale1=1.0; else scale1 = 1000/mean;
  cout<<"image 1 min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  cout<<"thresh1="<<thresh1<<" scale1="<<scale1<<'\n';

// get image stats to set threshold
  psy2imgptr->getstats(&min, &max, &mean);
  cout<<"image 2 min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  thresh2= threshfactor * max;
// get image stats using threshold to get scale factor
  psy2imgptr->getthreshstats(&min, &max, &mean, &pixelsused, thresh2);
  if(fabs(mean) < 1e-16)scale2=1.0; else scale2 = 1000/mean;
  cout<<"image 2 min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  cout<<"thresh2="<<thresh2<<" scale2="<<scale2<<'\n';
}

void diff2imgs::proc2pixels(char *in1, psytype in1type,
			    char *in2, psytype in2type,
			    char *out, psytype outtype)
{
  double dpixel, dpixel1, dpixel2;
// convert to double to simplify math
  type2double(in1, in1type, (char *)&dpixel1);
  type2double(in2, in2type, (char *)&dpixel2);

// do actual processing - threshold, multiply and subtract
  if(dpixel1 > thresh1 && dpixel2 > thresh2) {
    dpixel1 *= scale1;
    dpixel2 *= scale2;
    dpixel=dpixel1-dpixel2;
  }else dpixel=0;

// convert back to desired output type
    pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

class diff2imgswdivide : public diff2imgs
{
  void proc2pixels(char *in1, psytype in1type,
		   char *in2, psytype in2type,
		   char *out, psytype outtype);
 public:
  diff2imgswdivide(psyimg *psy1imgptr, psyimg *psy2imgptr,
		   double threshfactor,
		   psytype pixeltype=psynotype)
    : diff2imgs(psy1imgptr, psy2imgptr, threshfactor, pixeltype) {};
};

void diff2imgswdivide::proc2pixels(char *in1, psytype in1type,
				   char *in2, psytype in2type,
				   char *out, psytype outtype)
{
  double dpixel, dpixel1, dpixel2;
// convert to double to simplify math
  type2double(in1, in1type, (char *)&dpixel1);
  type2double(in2, in2type, (char *)&dpixel2);

/* old thresholding 4/4/95
  if(dpixel2 <= thresh2) dpixel=0; // avoid divide by zero
  else {
    dpixel1 = (dpixel1 < thresh1)? 0:(dpixel1*scale1);
*/
// do actual processing - note thresh2 >= 0 avoids divide by zero
  if(dpixel1 > thresh1 && dpixel2 > thresh2) {
    dpixel1 *= scale1;
    dpixel2 *= scale2;
    dpixel = 1000*(dpixel1 - dpixel2)/dpixel2;
  } else dpixel=0;
// convert back to desired output type
    pixeltypechg((char *)&dpixel, psydouble, out, outtype);
}

int main(int argc, char *argv[])
{
  char *infile1=NULL, *infile2=NULL, *outfile=NULL;
  double min, max, mean;
  int divide=0;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file format
  psyimg *diffedimg;
  double threshfactor=.4;
  char *desc;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout <<"Usage: "<<argv[0]<<" [-d[ivide]] infile1 infile2 outfile"<<'\n';
      cout <<"       [-t threshfactor]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strncmp("-d", argv[i], 2)==0)divide=1;
    else if(strcmp("-t", argv[i])==0){
      if(++i < argc)threshfactor=atof(argv[i]);
      else {
	cerr<<argv[0]<<" -t option requires float number\n";
	exit(1);
      }
    }
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile1 == NULL)infile1=argv[i];
    else if(infile2 == NULL)infile2=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input files
  psyfileclass infileclass;
  psytype infiletype;
  psyimg *inimag1=psynewinfile(infile1, &infileclass, &infiletype);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
  psyimg *inimag2=psynewinfile(infile2, &infileclass, &infiletype);
// buffer input files
  psypgbuff inimag1buff(inimag1, 1);
  psypgbuff inimag2buff(inimag2, 1);
  if(divide)
// difference input files with division
    diffedimg = new diff2imgswdivide((psyimg *)&inimag1buff,
				     (psyimg *)&inimag2buff,
				     threshfactor, outtype);
  else
// difference input files
    diffedimg= new diff2imgs((psyimg *)&inimag1buff,
			     (psyimg *)&inimag2buff,
			     threshfactor, outtype);
// build new description
  desc = new char[strlen(infile1) + strlen(infile2) + 25];
  if(divide)strcpy(desc, "diff2imgs divided: ");
  else strcpy(desc, "diff2imgs: ");
  strcat(desc, infile1); strcat(desc, " - ");
  strcat(desc, infile2);
  diffedimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  diffedimg->setdate();
  diffedimg->settime();
// output result to a file
  psyimg *outpsyimgptr=psynewoutfile(outfile, diffedimg,
				     outfileclass, outtype);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile1);
  log.loginfilelog(infile2);
// print out differenced images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';

  delete outpsyimgptr;
  delete diffedimg;
  delete inimag2;
  delete inimag1;
}
