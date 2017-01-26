#include "psyhdr.h"
#include "psyanalyze.h"
#include "psyecat.h"

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  psytype outtype=psynotype; // default to input type
  //  double min, max, mean;
  //  double scale_factor;
  int flip=1;
  //  int firstframe=1, lastframe=0;
  //  char *ptr;
  int debug=0;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile outfile [-noflip]"<<'\n';
      cout<<"       [-short | -uchar | -int | -float]\n";
      cout<<"       [-debug]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-debug", argv[i])==0)debug=1;
    else if(strcmp("-noflip", argv[i])==0)flip=0;
    else if(strcmp("-uchar", argv[i])==0)outtype=psyuchar;
    else if(strcmp("-short", argv[i])==0)outtype=psyshort;
    else if(strcmp("-int", argv[i])==0)outtype=psyint;
    else if(strcmp("-float", argv[i])==0)outtype=psyfloat;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }
  }
// open input files
  analyzefile inputimg(infile, "r");
  psypgbuff inputbuffered((psyimg *)&inputimg, 1);
  if(debug) {
    cout<<"input image header:\n";
    inputimg.showhdr();
    cout<<"input psyimg stuff:\n";
    inputimg.showpsyimg();
    cout<<"input stats:\n";
    inputbuffered.showstats();
  }
  psyimg *psyimgptr=(psyimg *)&inputbuffered;
  reverserows *reverserowsptr=NULL;
  if(flip){
    reverserowsptr=new reverserows(psyimgptr);
    psyimgptr = (psyimg *)reverserowsptr;
  }
// output image
  ecatfile outputimg(outfile, psyimgptr, outtype);
  if(debug) {
    cout<<"\noutput image info:\n";
    outputimg.showpsyimg();
    cout<<"output stats:\n";
    outputimg.showstats();
    cout<<"output ecat headar:\n";
    outputimg.showhdr();
  }
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// clean up
  if(reverserowsptr!=NULL)delete reverserowsptr;
  if(debug) cout<<"exiting analyze2ecat normally\n";
}
