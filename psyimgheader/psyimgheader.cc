#include "psyhdr.h"

int main(int argc, char *argv[])
{
  char *infile=NULL;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" infile"<<'\n';
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(infile == NULL)infile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
  psyimg *inimag=psynewinfile(infile);
// print header
  inimag->showpsyimg();
  delete inimag;
}
