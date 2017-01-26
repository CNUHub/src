#include "psyhdr.h"

int main(int argc, char *argv[])
{
  char *infile1=NULL, *infile2=NULL, *outfile=NULL;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file 1 format
  psyimg *addedimg;
  double scalefactor1=1;
  double scalefactor2=1;
  double min, max, mean;
  char *desc;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout <<"Usage: "<<argv[0]<<" infile1 infile2 outfile"<<'\n';
      cout <<" [-s1 scale1] [-s2 scale2] ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-s1", argv[i])==0) && (i+1<argc))
      scalefactor1=atof(argv[++i]);
    else if((strcmp("-s2", argv[i])==0) && (i+1<argc))
      scalefactor2=atof(argv[++i]);
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
  psytype workingtype=outtype;
  if(gettypebytes(inimag1->gettype()) > gettypebytes(workingtype))
    workingtype=inimag1->gettype();
  psyimg *inimag2=psynewinfile(infile2, &infileclass, &infiletype);
  if(gettypebytes(inimag2->gettype()) > gettypebytes(workingtype))
    workingtype=inimag2->gettype();
// buffer input files
  psypgbuff inimag1buff(inimag1, 1);
  psypgbuff inimag2buff(inimag2, 1);
// add input files
  addedimg= new addimgs((psyimg *)&inimag1buff,
			(psyimg *)&inimag2buff,
			scalefactor1, scalefactor2,
			workingtype);
// build new description
  desc = new char[strlen(infile1) + strlen(infile2) + 25];
  strcpy(desc, "add2imgs: ");
  strcat(desc, infile1); strcat(desc, " + ");
  strcat(desc, infile2);
  addedimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  addedimg->setdate();
  addedimg->settime();
// output result to a file
  psyimg *outpsyimgptr=psynewoutfile(outfile, addedimg,
				     outfileclass, outtype);
// print out images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile1);
  log.loginfilelog(infile2);
// clean up
  delete outpsyimgptr;
  delete addedimg;
  delete inimag2;
  delete inimag1;
}
