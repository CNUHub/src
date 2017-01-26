#include "psyhdr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  psyfileclass outfileclass=psynoclass;
  psytype outtype=psynotype;
  int *outfileclassinfo = NULL;
  psyimg *psyimgptr;
  double padvalue=0.0;
  xyziint pre, post;
  pre.x = pre.y = pre.z = pre.i = 0;
  post.x = post.y = post.z = post.i = 0;
// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" infile outfile\n";
      cout <<"      [-xpre x_prepad] [-xpost x_postpad]\n";
      cout <<"      [-ypre y_prepad] [-ypost y_postpad]\n";
      cout <<"      [-zpre z_prepad] [-zpost x_postpad]\n";
      cout <<"      [-ipre i_prepad] [-ipost i_postpad]\n";
      cout <<"      [-value padvalue]\n";
      cout <<"      ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"      ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp(argv[i],"-value")==0)&&((i+1)<argc))padvalue=atof(argv[++i]);
    else if((strcmp(argv[i],"-xpre")==0)&&((i+1)<argc))pre.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-xpost")==0)&&((i+1)<argc))post.x=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ypre")==0)&&((i+1)<argc))pre.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ypost")==0)&&((i+1)<argc))post.y=atoi(argv[++i]);
    else if((strcmp(argv[i],"-zpre")==0)&&((i+1)<argc))pre.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-zpost")==0)&&((i+1)<argc))post.z=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ipre")==0)&&((i+1)<argc))pre.i=atoi(argv[++i]);
    else if((strcmp(argv[i],"-ipost")==0)&&((i+1)<argc))post.i=atoi(argv[++i]);
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
  if(infile == NULL) {
    cerr << argv[0] << ": no input file\n";
    exit(1);
  }
  if(outfile == NULL) {
    cerr << argv[0] << ": no output file\n";
    exit(1);
  }
// open input file
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outfileclass == psynoclass) outfileclass=infileclass;
  if(outtype == psynotype) outtype=intype;
  psyimgptr = (psyimg *) inimag;
// buffer the input
  psypgbuff inimagbuff(psyimgptr, 1);
  psyimgptr = &inimagbuff;

  padimage padimgptr(psyimgptr, pre, post, padvalue);
  psyimgptr = &padimgptr;

// build new description
  char desc[strlen(infile) + strlen("padimg: ") + 1];
  strcpy(desc, "padimg: ");
  strcat(desc, infile);
  psyimgptr->setdescription(desc);
// set date and time to current date and time
  psyimgptr->setdate();
  psyimgptr->settime();
  psyimg *outimgptr = psynewoutfile(outfile, psyimgptr, outfileclass, outtype, outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// outfile clean up
  if(outimgptr != NULL)delete outimgptr;
// clean up
  if(inimag != NULL)delete inimag;
}
