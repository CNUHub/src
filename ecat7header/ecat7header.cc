#include "psyhdr.h"
#include "psyecat7.h"

int main(int argc, char *argv[])
{
  char *infile=NULL;
// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(infile == NULL)infile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }
  }
// open input files
  ecat7file inputimg(infile);
  cout<<"psyimg header:\n";
  inputimg.showpsyimg();
  cout<<"ecat 7 header:\n";
  inputimg.showhdr();
}
