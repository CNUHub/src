#include "psyhdr.h"
#include "psyecat.h"

int main(int argc, char *argv[])
{
  char *infile=NULL;
  int showall=0;
  int mainonly=0;
  int frame=-1;
  int plane=-1;
// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile [-all | -main | [-frame frame] [-plane plane]]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-all", argv[i])==0)showall = 1;
    else if(strcmp("-main", argv[i])==0)mainonly = 1;
    else if((strcmp("-frame", argv[i])==0) && ((i+1)<argc)){
      if(sscanf(argv[++i],"%d", &frame)!=1)
      {
	cerr<<argv[0]<<": invalid frame="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((strcmp("-plane", argv[i])==0) && ((i+1)<argc)){
      if(sscanf(argv[++i],"%d", &plane)!=1)
      {
	cerr<<argv[0]<<": invalid plane="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if(infile == NULL)infile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }
  }
// open input files
  ecatfile inputimg(infile);
  cout<<"ecat header:\n";
  if(showall || (frame > 0) || (plane > 0)) {
    if(showall) inputimg.showmainhdr();

    int zdim, idim;
    inputimg.getsize(NULL, NULL, &zdim, &idim);

    int planeend;
    if(plane > 0) planeend=plane;
    else { plane=1; planeend=zdim; }

    int frameend;
    if(frame > 0) frameend=frame;
    else { frame=1; frameend=idim; }

    for(; frame<=frameend; frame++) {
      for(; plane<=planeend; plane++) {
	int xdim=(int)inputimg.get_header_value("dimension_1", frame, plane);
	inputimg.showsubhdr();
      }
    }
  }
  else if(mainonly) {
    inputimg.showmainhdr();
  }
  else {
    inputimg.showmainhdr();
    inputimg.showsubhdr();
  }
}
