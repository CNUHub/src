#include "psyhdr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean;
  psyfileclass outfileclass=psynoclass;
  psytype outtype=psynotype;
  int *outfileclassinfo = NULL;
  int resize=0;
  int center=0;
  int extents=0;
  psyimg *psyimgptr;
  padimage *padimgptr=NULL;
  double padvalue=0.0;
  psydims beg, end;
  beg.x = beg.y = beg.z = beg.i = -1;
  end.x = end.y = end.z = end.i = -1;
// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" infile [outfile]\n";
      cout <<"      [-resize] [-center] [-p padvalue]\n";
      cout <<"      [-xb xbeg] [-xe xend] [-yb ybeg] [-ye yend]\n";
      cout <<"      [-zb zbeg] [-ze zend] [-ib ibeg] [-ie iend]\n";
      cout <<"      ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"      ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-resize")==0)resize=1;
    else if(strcmp(argv[i],"-center")==0)center=1;
    else if((strcmp(argv[i],"-p")==0)&&((i+1)<argc))padvalue=atof(argv[++i]);
    else if((strcmp(argv[i],"-xb")==0)&&((i+1)<argc))beg.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-xe")==0)&&((i+1)<argc))end.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-yb")==0)&&((i+1)<argc))beg.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ye")==0)&&((i+1)<argc))end.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-zb")==0)&&((i+1)<argc))beg.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ze")==0)&&((i+1)<argc))end.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ib")==0)&&((i+1)<argc))beg.i=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ie")==0)&&((i+1)<argc))end.i=atoi(argv[++i]);
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input file
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outfileclass==psynoclass)outfileclass=infileclass;
  if(outtype==psynotype)outtype=intype;
// buffer the input
  psypgbuff inimagbuff(inimag, 1);
  psydims in_beg=inimagbuff.getorig();
  psydims in_end=inimagbuff.getend();
  psydims in_size=inimagbuff.getsize();

// use input values or defaults
  if(beg.x < 0)beg.x=in_beg.x; if(end.x < 0)end.x=in_end.x;
  if(beg.y < 0)beg.y=in_beg.y; if(end.y < 0)end.y=in_end.y;
  if(beg.z < 0)beg.z=in_beg.z; if(end.z < 0)end.z=in_end.z;
  if(beg.i < 0)beg.i=in_beg.i; if(end.i < 0)end.i=in_end.i;

// select block of image
  psyimgblk subregion((psyimg *)&inimagbuff, beg.x, beg.y, beg.z, beg.i,
		      end.x, end.y, end.z, end.i, 0);
  psyimgptr= &subregion;
// pad image if needed
  if(!resize){
    psydims prepad = beg - in_beg;
    psydims postpad = in_end - end;
    if(center) {
      psydims totalpad = prepad + postpad;
      prepad = xyzidouble2int(xyziint2double(totalpad) * .5);
      postpad = totalpad - prepad;
    }
    padimgptr=new padimage(psyimgptr, prepad, postpad, padvalue);
    psyimgptr=(psyimg *)padimgptr;
  }
// caclulate shifts from input image
  xyzidouble shft=
    xyziint2double(psyimgptr->getorig() - inimag->getorig())
      * psyimgptr->getres();
  cout<<"shifted distance=("<<shft.x<<','<<shft.y<<','<<shft.z<<','<<shft.i<<")\n";
// build new description
  char desc[strlen(infile) + strlen("subregion: ") + 1];
  strcpy(desc, "subregion: ");
  strcat(desc, infile);
  psyimgptr->setdescription(desc);
// set date and time to current date and time
  psyimgptr->setdate();
  psyimgptr->settime();
  if(outfile == NULL) {
      cerr << argv[0] << ": no output file\n";
  }
  else {
    psyimg *outimgptr=psynewoutfile(outfile, psyimgptr, outfileclass, outtype, outfileclassinfo);
// log
    logfile log(outfile, argc, argv);
    log.loginfilelog(infile);
// outfile clean up
    if(outimgptr != NULL)delete outimgptr;
  }
// clean up
  if(padimgptr != NULL)delete padimgptr;
  if(inimag != NULL)delete inimag;
}
