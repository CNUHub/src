#include "psyhdr.h"

int main(int argc, char *argv[])
{
  char *outfile=NULL;
  double min, max, mean;
  psyimg *newinputfile=NULL;
  psypgbuff *newpsypgbuff=NULL;
  concatimgs *concatimgptr=NULL, *firstconcatimgptr=NULL;
  psyimg *firstimgptr=NULL;
  psyimg *psyimgptr=NULL;
  int numinputfiles=0;
  double scalefactor;
  double threshold= -1e10;
  int catdim=2;
  int inputted_res=0;
  double dim_res=1.0;
  psytype outtype=psynotype; // will default to first input type
  psyfileclass outfileclass=psynoclass; // will default to 1st in file format
  psyorient orientation=psynoorient;  // jtlee 04/03/13 changed to default to 1st input file format

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" [-dim x|y|z|i (z)] [-res dimres]\n";
      cout<<" infile1 infile2 [infile3 [...]]";
      cout<<" -o outfile\n";
      cout<<" [-tran | -sag | -cor]\n";
      cout<<" ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout<<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(strcmp("-tran", argv[i])==0)orientation=psytransverse;
    else if(strcmp("-sag", argv[i])==0)orientation=psysagittal;
    else if(strcmp("-cor", argv[i])==0)orientation=psycoronal;
    else if(strcmp("-dim", argv[i])==0 && ((i+1)<argc)) {
      switch(argv[++i][0]) {
      case 'x':
      case '0':
	catdim=0;
	break;
      case 'y':
      case '1':
	catdim=1;
	break;
      case 'z':
      case '2':
	catdim=2;
	break;
      case 'i':
      case '3':
	catdim=3;
	break;
      default:
	cerr<<argv[0]<<" - invalid dimension given\n";
	exit(1);
      }
    }
    else if(strcmp("-res", argv[i])==0 && ((i+1)<argc)) {
      if(inputted_res||(sscanf(argv[++i],"%lf", &dim_res)!=1))
      {
	cerr<<argv[0]<<": invalid dimres="<<argv[i]<<'\n';
	exit(1);
      }
      inputted_res=1;
    }
    else if((strcmp("-o", argv[i])==0) && ((i+1)<argc)){
      outfile=argv[++i];
    }
    else {
      numinputfiles++;
// open and link each input file as we come across it
      psyfileclass infileclass;
      psytype intype;
      newinputfile=psynewinfile(argv[i], &infileclass, &intype);
// first image file
      if(firstimgptr == NULL) {
	firstimgptr=newinputfile;
	if(outfileclass == psynoclass)outfileclass=infileclass;
// jtlee 04/03/13 changed to default to 1st input file format
        if(orientation == psynoorient) orientation = firstimgptr->getorient();
	if(outtype == psynotype)outtype=intype;
	psyimgptr=firstimgptr;
      }
      else {
// cat it onto previous images
	if(inputted_res)
	  concatimgptr=new concatimgs(psyimgptr, newinputfile, catdim, dim_res);
	else concatimgptr=new concatimgs(psyimgptr, newinputfile, catdim);
// keep track of first concatimg pointer
	if(firstconcatimgptr==NULL)firstconcatimgptr=concatimgptr;
        psyimgptr=(psyimg *)concatimgptr;
      }
    }
  }
  //cout<<"showing psyimg hdr after input loop\n";
  //psyimgptr->showpsyimg();
// make sure output file name set
  if(outfile == NULL) {
    cerr<<argv[0]<<" - no output file specified\n";
    exit(1);
  }
  if(psyimgptr == NULL) {
    cerr<<argv[0]<<" - no input file(s) specified\n";
    exit(1);
  }
  if(outfileclass == ecatfileclass || outfileclass == niftifileclass) {
    // need to buffer because psyimg::psy2imglnk::concatimgs:concatimgs::initgetpixel - not implimented
    newpsypgbuff = new psypgbuff(psyimgptr, 1);
    psyimgptr = newpsypgbuff;
  }
// set orientation
// jtlee 04/03/13 changed to default to 1st input file format
  if(orientation != psynoorient) psyimgptr->setorient(orientation);
// set description for output image
//  jtlee 12/12/17 setting description should be done better in concatimgs object now
//  psyimgptr->setdescription("concatenated images");
// set date and time to current date and time
  psyimgptr->settime();
  psyimgptr->setdate();
// output result to image file
  psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr,
				     outfileclass, outtype);
  //cout<<"showing outpsyimgptr header\n";
  //outpsyimgptr->showpsyimg();
// log
  logfile log(outfile, argc, argv);
// print out images stats
  psydims size =  outpsyimgptr->getsize();
  cout<<"out image size=("<<size.x<<','<<size.y<<','<<size.z<<','<<size.i<<")\n";
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';

// clean up
  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(newpsypgbuff != NULL) delete newpsypgbuff;
  if(concatimgptr != NULL) {
    concatimgs *prevconcatimgptr;
    while(concatimgptr != firstconcatimgptr) {
      prevconcatimgptr=concatimgptr;
      concatimgptr=(concatimgs *)concatimgptr->getlink1();
      delete prevconcatimgptr->getlink2();
      delete prevconcatimgptr;
    }
    delete firstconcatimgptr->getlink1();
    delete firstconcatimgptr->getlink2();
    delete firstconcatimgptr;
  }
  else if(firstimgptr != NULL) delete firstimgptr; // only one input file no concatenation
}
