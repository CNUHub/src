#include "psyhdr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean;
  char *desc;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file format
  int *outfileclassinfo = NULL;
  xyzint boxsize;
  boxsize.x = boxsize.y = boxsize.z = 3;
  gradienttype grad=no_gradient;
  double factor=1.0;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile"<<'\n';
      cout <<"       [-c cubesize] [-x x_size] [-y y_size] [-z z_size]\n";
      cout <<"       [-gradmxyz | -gradmmxyz | -gradmxy | -gradmmxy]\n";
      cout <<"       [-f factor]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-c", argv[i])==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%d",&boxsize.x);
      boxsize.y = boxsize.z = boxsize.x;
    }
    else if((strcmp("-x", argv[i])==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&boxsize.x);
    else if((strcmp("-y", argv[i])==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&boxsize.y);
    else if((strcmp("-z", argv[i])==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&boxsize.z);
    else if(strcmp("-gradmxyz", argv[i])==0)grad=mag_of_xyz_gradient;
    else if(strcmp("-gradmmxyz", argv[i])==0)grad=max_mag_of_xyz_gradient;
    else if(strcmp("-gradmxy", argv[i])==0)grad=mag_of_xy_gradient;
    else if(strcmp("-gradmmxy", argv[i])==0)grad=max_mag_of_xy_gradient;
    else if((strcmp("-f", argv[i])==0)&&((i+1)<argc))
      sscanf(argv[++i],"%lf",&factor);
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
  psytype infiletype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &infiletype, &outfileclassinfo);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// internal use type that is greater of outfiletype and input image type
  psytype workingtype=inimag->gettype();
  if(gettypebytes(outtype) > gettypebytes(workingtype))workingtype=outtype;
// buffer input files
// filter image
  psyimg *filteredimg;
  if(grad != no_gradient) {
    filteredimg=(psyimg *)new gradient(inimag, grad, workingtype, factor);
// build new description
    desc = new char[strlen(infile) + strlen("gradient: ")+1];
    strcpy(desc, "gradient: ");
  }
  else {

// note pixelavg provides its own page buffer
    filteredimg=(psyimg *)new pixelavg(inimag, workingtype,
				       boxsize.x, boxsize.y, boxsize.z,
				       factor);
// build new description
    desc = new char[strlen(infile) + strlen("filterimg(NNxNNxNN): ")+1];
    sprintf(desc, "filterimg(%dx%dx%d): ", boxsize.x, boxsize.y, boxsize.z);
  }

  strcat(desc, infile);
  filteredimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  filteredimg->setdate();
  filteredimg->settime();
  cout<<"\nfilteredimg info:\n";
  filteredimg->showpsyimg();
// output result to file
  cout<<"now doing the real work while outputting the image\n";
  psyimg *outpsyimgptr=psynewoutfile(outfile, filteredimg,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// print output filtered images stats
  cout<<"output image stats:";
  outpsyimgptr->showstats();
// clean up
  delete outpsyimgptr;
  delete filteredimg;
  delete inimag;
}
