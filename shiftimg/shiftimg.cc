#include "psyhdr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean;
  psytype outtype=psynotype;
  psyfileclass outfileclass=psynoclass; // will default to input file 1 format
  int *outfileclassinfo = NULL;
  psyimg *addedimg;
  int center=0;
  psyimg *psyimgptr;
  psydims shift;
  shift.x=shift.y=shift.z=shift.i=0;
// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile\n";
      cout <<"      [-center]\n";
      cout <<"      [-x xshift] [-y yshift] [-z zshift] [-i ishift]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-center")==0)center=1;
    else if((strcmp(argv[i],"-x")==0)&&((i+1)<argc))shift.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-y")==0)&&((i+1)<argc))shift.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-z")==0)&&((i+1)<argc))shift.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-i")==0)&&((i+1)<argc))shift.i=atoi(argv[++i]);
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input file
  psyfileclass infileclass;
  psytype infiletype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &infiletype, &outfileclassinfo);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
  psyimgptr=inimag;
// buffer the input
  psypgbuff inimagbuff(psyimgptr, 1);
  psyimgptr=(psyimg *)&inimagbuff;
  psydims in_beg=psyimgptr->getorig();
  psydims in_end=psyimgptr->getend();
  psydims in_size=psyimgptr->getsize();

// calculate extents
  if(center) {
    double max;
    psydims beg, end;
    psyimgptr->getstats(NULL, &max, NULL);
    FindBrainExtent(psyimgptr, .3 * max,
		    &beg.x, &end.x, &beg.y, &end.y, &beg.z, &end.z);
    if(beg.x > end.x){
      shift.x=shift.y=shift.z=shift.i=0;
    }
    else {
      beg.i=in_beg.i; end.i=in_end.i;
      shift = xyzidouble2int(xyziint2double((in_beg + in_end) - (beg + end))
			     * .5);
    }
  }
// shift image
  psyshiftimg shiftimg((psyimg *)&inimagbuff, shift);
  psyimgptr = (psyimg *)&shiftimg;
// build new description
  char desc[strlen(infile) + strlen("shiftimg: ") + 1];
  strcpy(desc, "shiftimg: ");
  strcat(desc, infile);
  psyimgptr->setdescription(desc);
// set date and time to current date and time
  psyimgptr->setdate();
  psyimgptr->settime();
// output result to a file
  psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr,
				     outfileclass, outtype,
				     outfileclassinfo);
// print out subregion images stats
  cout<<"output stats: ";
  outpsyimgptr->showstats();
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// clean up
  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(inimag != NULL)delete inimag;
}
