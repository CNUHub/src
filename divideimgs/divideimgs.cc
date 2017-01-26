#include "psyhdr.h"

int main(int argc, char *argv[])
{
  char *infile1=NULL, *infile2=NULL, *outfile=NULL;
  double min, max, mean;
  psytype outtype=psynotype; // will default to input type
  psyimg *dividedimg;
  psyfileclass outfileclass=psynoclass; // will default to input file 1 format
  char *desc;
  double factor=1;
  double minimum_divisor_magnitude=1e-16;
  double default_value=0;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout <<"Usage: "<<argv[0]<<" infile1 infile2 outfile"<<'\n';
      cout <<"       [-m minimum_divisor_magnitude] [-d default_value]\n";
      cout <<"       [-f factor]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-m", argv[i])==0)&&(i+1<argc))
      minimum_divisor_magnitude=atof(argv[++i]);
    else if((strcmp("-d", argv[i])==0)&&(i+1<argc))
      default_value=atof(argv[++i]);
    else if((strcmp("-f", argv[i])==0)&&(i+1<argc))
      factor=atof(argv[++i]);
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
// internally use type that is greater of outfiletype and input image type
  psytype workingtype=inimag1->gettype();
  if(gettypebytes(outtype) > gettypebytes(workingtype))workingtype=outtype;
  psyimg *inimag2=psynewinfile(infile2, &infileclass, &infiletype);
  if(gettypebytes(inimag2->gettype()) > gettypebytes(workingtype))
    workingtype=inimag2->gettype();
// buffer input files
  psypgbuff inimag1buff((psyimg *)inimag1, 1);
  psypgbuff inimag2buff((psyimg *)inimag2, 1);

// divide input images
  dividedimg= new divideimgs((psyimg *)&inimag1buff,
			     (psyimg *)&inimag2buff,
			     factor, default_value,
			     minimum_divisor_magnitude, workingtype);
// build new description
  desc = new char[strlen(infile1) + strlen(infile2) + 25];
  strcpy(desc, "divideimgs: ");
  strcat(desc, infile1); strcat(desc, "/");
  strcat(desc, infile2);
  dividedimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  dividedimg->setdate();
  dividedimg->settime();
// output result to a file
  psyimg *outpsyimgptr=psynewoutfile(outfile, dividedimg,
				     outfileclass, outtype);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile1);
  log.loginfilelog(infile2);
// print out images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  delete outpsyimgptr;
  delete dividedimg;
  delete inimag2;
  delete inimag1;
}
