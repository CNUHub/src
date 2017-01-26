#include "psyhdr.h"

int main(int argc, char *argv[])
{
  char *outfile=NULL;
  double min, max, mean;
  psytype outtype=psynotype; // will default to largest input type
  psytype biggestintype=psynotype;
  psyfileclass outfileclass=psynoclass; // defaults to first input class
  psyimg *newimgptr=NULL;
  psypgbuff *newpsypgbuff=NULL;
  psyimg *psyimgptr;
  int numinputfiles=0;
  int avg_flag=0;
  int template_flag=0;
  int divide_template_flag=0;
  int numtemplates=0;
  accumulateimgs accumulate_images(psydouble);
  accumulateimgs accumulate_templates(psyshort);
// allocate a psyimg list large enough to hold maximum possible number of files
  psypgbuff *pgbuffarray[argc];

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  int i;
  for(i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile1 [infile2 [infile3 [...]]] -o outfile\n";
      cout<<" [-t|-td templatefile1 [templatefile2 [...]]]"<<'\n';
      cout<<" [-avg] ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout<<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-avg", argv[i])==0)avg_flag=1;
    else if(strcmp("-t", argv[i])==0)template_flag=1;
    else if(strcmp("-td", argv[i])==0)divide_template_flag=1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if((strcmp("-o", argv[i])==0) && ((i+1)<argc))outfile=argv[++i];
    else if(strncmp("-", argv[i], 1) == 0) {
      cerr<<argv[0]<<": error invalid parameter="<<argv[i]<<'\n';
      exit(1);
    }
    else {
// open and link each input file as we come across it
      psyfileclass infileclass;
      psytype intype;
      newimgptr=psynewinfile(argv[i], &infileclass, &intype);
// buffer each input file to improve accumulation speed
      newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
      if(!(template_flag || divide_template_flag)){
// keep a pointer to this buffer
        pgbuffarray[numinputfiles]=newpsypgbuff;
// note largest type
        if(biggestintype == psynotype)biggestintype=intype;
        else if(gettypebytes(biggestintype) < gettypebytes(intype))
	  biggestintype=intype;
        if(outfileclass == psynoclass)outfileclass=infileclass;
// add it to the image accumulator
	accumulate_images.addimg((psyimg *)newpsypgbuff);
	numinputfiles++;
      }
      else {
// add it to the template accumulator
	accumulate_templates.addimg((psyimg *)newpsypgbuff);
	numtemplates++;
      }
    }
  }

// verify at least one input file
  if(numinputfiles < 1) {
    cerr<<argv[0]<<" - no input files specified\n";
    exit(1);
  }

// verify output file name set
  if(outfile == NULL) {
    cerr<<argv[0]<<" - no output file specified\n";
    exit(1);
  }

  if(template_flag && divide_template_flag) {
    cerr<<argv[0]<<" - -t option not allowed with -td option\n";
    exit(1);
  }

  if(avg_flag && divide_template_flag) {
    cerr<<argv[0]<<" - -avg option not allowed with -td option\n";
    exit(1);
  }

// verify templates
  if((template_flag || divide_template_flag) && (numtemplates < 1)) {
    cerr<<argv[0]<<" - -t & -td options require template files\n";
    exit(1);
  }

// if average option set scale to divide by number of files
  if(avg_flag)accumulate_images.setscale(1.0/(double)numinputfiles);

// set pointer to this image to allow replacing it with further processes
  psyimgptr=(psyimg *)&accumulate_images;

  if(template_flag) {
// template image with threshold to requiring all templates to be set
// assumes template values set to 1 for pass and 0 for reject
    psyimgptr=(psyimg *)new templateimg(psyimgptr,
					(psyimg *)&accumulate_templates,
					1, (double)numtemplates-.5);
  }

// divide image by number of accumulated template values
  if(divide_template_flag) {
    psyimgptr=(psyimg *)new divideimgs(psyimgptr,
				       (psyimg *)&accumulate_templates,
				       1.0, 0.0, .5);
  }

// set description for output image
  if(avg_flag && template_flag)
    psyimgptr->setdescription("averaged templated images\n");
  else if(avg_flag)psyimgptr->setdescription("averaged images\n");
  else if(template_flag)
    psyimgptr->setdescription("added templated images\n");
  else psyimgptr->setdescription("added images\n");

// set date and time to current date and time
  psyimgptr->settime();
  psyimgptr->setdate();

// output result to file
  if(outtype==psynotype)outtype=biggestintype;
  psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr,
				     outfileclass, outtype);
// log
  logfile log(outfile, argc, argv);

// print out images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';

// clean up
  for(i=0; i<numinputfiles; i++){
    delete pgbuffarray[i]->inputpsyimg;
    delete pgbuffarray[i];
  }
  delete outpsyimgptr;
}
