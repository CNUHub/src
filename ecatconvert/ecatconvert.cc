#include "psyhdr.h"
#include "psyecat.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean;
  char *desc;
  psytype outtype = psynotype; // will default to input type
  int outecatfiletype = -1; // will default to input type

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile"<<'\n';
      cout <<"       [-image | -scan | -norm | -attn]\n";
      cout <<"       [-uchar | -short | -int | -float]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-uchar", argv[i])==0) outtype=psyuchar;
    else if(strcmp("-short", argv[i])==0) outtype=psyshort;
    else if(strcmp("-int", argv[i])==0) outtype=psyint;
    else if(strcmp("-float", argv[i])==0) outtype=psyfloat;
    else if(strcmp("-image", argv[i])==0) outecatfiletype=IMAGE_FILE;
    else if(strcmp("-scan", argv[i])==0) outecatfiletype=SCAN_FILE;
    else if(strcmp("-norm", argv[i])==0) outecatfiletype=NORM_FILE;
    else if(strcmp("-attn", argv[i])==0) outecatfiletype=ATTN_FILE;
    else if(infile == NULL) infile=argv[i];
    else if(outfile == NULL) outfile=argv[i];
    else {
      cerr << argv[0] << ": invalid parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input files
  ecatfile *inimag= new ecatfile(infile);
  if(outtype == psynotype)
    outtype = ecattype2psytype((int) inimag->get_header_value("data_type"));
  if(outecatfiletype == -1) outecatfiletype = inimag->get_ecatfiletype();
// buffer input file
  psypgbuff inputbuffered(inimag, 1);
// output to ecat file
  psyimg *outpsyimgptr= new ecatfile(outfile, &inputbuffered,
				     outtype, outecatfiletype);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
  delete outpsyimgptr;
  delete inimag;
}
