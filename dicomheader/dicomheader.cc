#include "psyhdr.h"
#include "cnudicom.h"

int main(int argc, char *argv[])
{
  char *infile=NULL;
  show_mode showMode=NORMAL;
// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout<<"Usage: "<<argv[0];
      cout<<" [-help] [-terse | -concise | -brief | -normal | -lengthy | -verbose | -tedious] infile\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-terse", argv[i])==0)showMode=TERSE;
    else if(strcmp("-concise", argv[i])==0)showMode=CONCISE;
    else if(strcmp("-brief", argv[i])==0)showMode=BRIEF;
    else if(strcmp("-normal", argv[i])==0)showMode=NORMAL;
    else if(strcmp("-lengthy", argv[i])==0)showMode=LENGTHY;
    else if(strcmp("-verbose", argv[i])==0)showMode=VERBOSE;
    else if(strcmp("-tedious", argv[i])==0)showMode=TEDIOUS;
    else if(infile == NULL)infile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }

  }
// open input files
  dicomfile inputimg(infile, "r");
  cout<<"psyimg header:\n";
  inputimg.showpsyimg();
  cout<<"DICOM header:\n";
  inputimg.showhdr(&cout, showMode);
}
