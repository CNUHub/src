#include "psyhdr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *infile=NULL;
  psyimg *psyimgptr;
  double percent_max=30;
// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout<<"Usage: "<<argv[0]<<" infile ";
      cout<<"[-p percent_max(30)]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp(argv[i],"-p")==0)&&((i+1)<argc))
      percent_max=atof(argv[++i]);
    else if(infile == NULL)infile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input file
  psyimg *inimag=psynewinfile(infile);
// buffer the input
  psypgbuff inimagbuff(inimag, 1);

// calculate extents
  double max;
  psydims beg, end;
  inimagbuff.getstats(NULL, &max, NULL);
  FindBrainExtent((psyimg *)&inimagbuff, percent_max * max/100.0,
		  &beg.x, &end.x, &beg.y, &end.y, &beg.z, &end.z);

  psydims in_beg=inimagbuff.getorig();
  psydims in_end=inimagbuff.getend();
  psydims in_size=inimagbuff.getsize();
  psydims increase=xyzidouble2int(xyziint2double(in_size)*.05);

  if(beg.x > end.x){
    beg=in_beg; end=in_end;
  }
  else {
    beg = beg - increase;
    end = end + increase;
    beg=clip(beg, in_beg, in_end);
    end=clip(end, in_beg, in_end);
  }
  cout<<"-xb "<<beg.x<<" -xe "<<end.x<<'\n';
  cout<<"-yb "<<beg.y<<" -ye "<<end.y<<'\n';
  cout<<"-zb "<<beg.z<<" -ze "<<end.z<<'\n';
  cout<<"-ib "<<beg.i<<" -ie "<<end.i<<'\n';
// clean up
  if(inimag != NULL)delete inimag;
}
