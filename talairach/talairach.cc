#include "psyhdr.h"
#include "talairach.h"

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  char *mapfile=NULL;
  char *stalibfile=NULL;
  char *writemapfile=NULL;
  double min, max, mean;
  char *desc;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; //defaults to input class
  int *outfileclassinfo = NULL;
  int resize=0;
  int zdbl=0;
  int maxnumpgbuffs=10;
  psyimg *psyimgptr1, *psyimgptr2;
  mappingmodes mappingmode=standard_mappingmode;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-zdbl] [-r[esize]]"<<'\n';
      cout <<"       [-mf mapfile | -sta p#_sta.lib] [-wmf writemapfile] [-mon]\n";
      cout <<"       [-npgs maxnumpgbuffs]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(strncmp("-r", argv[i], 2)==0)resize=1;
    else if(strcmp("-zdbl", argv[i])==0)zdbl=1;
    else if((strcmp("-mf", argv[i])==0)&&((i+1)<argc))mapfile=argv[++i];
    else if((strcmp(argv[i], "-sta")==0)&&((i+1)<argc))stalibfile=argv[++i];
    else if((strcmp("-wmf", argv[i])==0)&&((i+1)<argc))writemapfile=argv[++i];
    else if(strcmp("-mon", argv[i])==0)mappingmode=outside_nearest;
    else if((strcmp("-npgs", argv[i])==0)&&((i+1)<argc))
      maxnumpgbuffs=atoi(argv[++i]);
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
  if(outtype==psynotype)outtype=intype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// buffer input files
  psypgbuff inputbuffered(inimag, maxnumpgbuffs);
  psyimgptr1 = (psyimg *) &inputbuffered;

// double z resolution to smooth output
  if(zdbl){ 
    cout<<"*\n*- note - zdbl REQUIRES doubling Z PIXEL mapping values *\n*\n";
    psyimgptr1 = new psyimgdblz(psyimgptr1);
  }
// convert image
  to_talairach talairachimg(psyimgptr1, psynotype, resize);
// set mapping mode
  talairachimg.setmappingmode(mappingmode);
// set conversion factors based on one of three input methods
  if(mapfile && stalibfile) {
      cerr << argv[0] << ": error both mapfile and p#_sta.lib specified\n";
      exit(1);
  }
  else if(mapfile)talairachimg.mappinginput(mapfile, standard_mapfile);
  else if(stalibfile)talairachimg.mappinginput(stalibfile, satoshi_sta_lib);
  else talairachimg.mappinginput(NULL);
// write mapping file
  talairachimg.outputmapping(writemapfile);
  cout<<"\ntalairachimg info:\n";
  talairachimg.showto_talairach();
  psyimgptr2=(psyimg *)&talairachimg;

// undouble
  if(zdbl)psyimgptr2 = new psyimghalfz(psyimgptr2);

// build new description
  desc = new char[strlen(infile) + 20];
  strcpy(desc, "talairach image: ");
  strcat(desc, infile);
  psyimgptr2->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  psyimgptr2->setdate();
  psyimgptr2->settime();

// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr2,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

// print output images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(inimag != NULL)delete inimag;
}
