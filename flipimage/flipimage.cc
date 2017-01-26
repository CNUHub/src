#include "psyhdr.h"


class reverseplanes : public psypgbuff {
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  reverseplanes() {};
  reverseplanes(psyimg *psyimgptr, int maxnumpages=1);
};

reverseplanes::reverseplanes(psyimg *psyimgptr, int maxnumpages)
    : psypgbuff(psyimgptr, maxnumpages) {
  // fix spatial transforms
  psydims dim=psyimgptr->getsize();
  matrix4X4 Factors(Identity); Factors.m.m33 = -1.0l;
  xyzidouble newvoxorig;
  newvoxorig.x=newvoxorig.y=newvoxorig.i=0; newvoxorig.z = (dim.z-1);
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix * Factors;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform(new threeDtransform(newmatrix), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix * Factors;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform2(new threeDtransform(newmatrix), getspatialtransformcode2());
  }
};

void reverseplanes::fillpage(char *buff, int z, int i)
{
// reverse z value
  z=end.z-z+orig.z;
  inputpsyimg->copyblock(buff, orig.x, orig.y, z, i,
			 end.x, end.y, z, i, inc.x, inc.y, inc.z, inc.i,
			 type);
  return;
}

class reversecols : public psypgbuff {
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  reversecols() {};
  reversecols(psyimg *psyimgptr, int maxnumpages=1);
};

reversecols::reversecols(psyimg *psyimgptr, int maxnumpages)
  : psypgbuff(psyimgptr, maxnumpages) {
  // fix spatial transforms
  psydims dim=psyimgptr->getsize();
  matrix4X4 Factors(Identity); Factors.m.m11 = -1.0l;
  xyzidouble newvoxorig;
  newvoxorig.y=newvoxorig.z=newvoxorig.i=0; newvoxorig.x = (dim.x-1);
  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix * Factors;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform(new threeDtransform(newmatrix), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    matrix4X4 oldmatrix = tdt->getMatrix();
    xyzidouble newvoxoffset = oldmatrix * newvoxorig;
    matrix4X4 newmatrix = oldmatrix * Factors;
    newmatrix.m.m14 = newvoxoffset.x; newmatrix.m.m24 = newvoxoffset.y; newmatrix.m.m34 = newvoxoffset.z;
    newmatrix.m.m44 = 1.0l;
    setspatialtransform2(new threeDtransform(newmatrix), getspatialtransformcode2());
  }
}

void reversecols::fillpage(char *buff, int z, int i)
{
// set buffer to last x column
  buff += offset(end.x, orig.y, z, i) - offset(orig.x, orig.y, z, i);
// use negative increment to fill columns backwards
  inputpsyimg->copyblock(buff, orig.x, orig.y, z, i,
			 end.x, end.y, z, i, -inc.x, inc.y, inc.z, inc.i,
			 type);
  return;
}

class switchrowsncols : public psypgbuff {
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  switchrowsncols() {};
  switchrowsncols(psyimg *psyimgptr, int maxnumpages=1);
};

switchrowsncols::switchrowsncols(psyimg *psyimgptr, int maxnumpages)
{
  psydims size=psyimgptr->getsize();
  psydims orig=psyimgptr->getorig();
  psyres res=psyimgptr->getres();
  initpgbuff(psyimgptr,
	     size.y, size.x, size.z, size.i,
	     psyimgptr->gettype(),
	     orig.y, orig.x, orig.z, orig.i, 0,
	     res.y, res.x, res.z, res.i,
	     psyimgptr->getwordres(),
	     maxnumpages);

  matrix4X4 Factors(Identity);
  Factors.m.m11 = Factors.m.m22 = 0.0l;
  Factors.m.m21 = Factors.m.m12 = 1.0l;

  threeDtransform *tdt = getspatialtransform();
  if(tdt != NULL) {
    setspatialtransform(new threeDtransform(tdt->getMatrix() * Factors), getspatialtransformcode());
  }
  tdt = getspatialtransform2();
  if(tdt != NULL) {
    setspatialtransform2(new threeDtransform(tdt->getMatrix() * Factors), getspatialtransformcode2());
  }
}

void switchrowsncols::fillpage(char *buff, int z, int i)
{
// fill incrementing y the fastest
  inputpsyimg->copyblock(buff, orig.y, orig.x, z, i,
			 end.y, end.x, z, i,
			 inc.y, inc.x, inc.z, inc.i,
			 type);
  return;
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  int xflip=0, yflip=0, zflip=0, xyswitch=0;
  double min, max, mean;
  char *desc;
  psyimg *flippedimg;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file format
  int *outfileclassinfo = NULL;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-z | -y | -x | -xy]"<<'\n';
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i], "-x")==0)xflip=1;
    else if(strcmp(argv[i], "-y")==0)yflip=1;
    else if(strcmp(argv[i], "-z")==0)zflip=1;
    else if(strcmp(argv[i], "-xy")==0)xyswitch=1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// if no flip requested set default to flip z
  if(!xflip && !yflip && !zflip && !xyswitch) {
    zflip=1;
    cout<<"defaulting to flipping z planes\n";
  }

// open input files
  psytype intype;
  psyfileclass infileclass;
  flippedimg = psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outtype==psynotype)outtype=intype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// buffer the input data in case input file doesn't support x or y negative incs
  flippedimg =  new psypgbuff(flippedimg, 1);

// reverse the x columns
  if(xflip)flippedimg = new reversecols(flippedimg);
// reverse the y rows
  if(yflip)flippedimg = new reverserows(flippedimg);
// reverse the z planes
  if(zflip)flippedimg = new reverseplanes(flippedimg);
// switch the x and y planes
  if(xyswitch)flippedimg = new switchrowsncols(flippedimg);
// build new description
  desc = new char[strlen(infile) + 25];
  desc[0]='\0'; // initialize empty description
  if(xflip)strcat(desc, "x");
  if(yflip)strcat(desc, "y");
  if(zflip)strcat(desc, "z");
  if(xyswitch)strcat(desc, "xy");
  strcat(desc, " flipped image: ");
  strcat(desc, infile);
  flippedimg->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  flippedimg->setdate();
  flippedimg->settime();
// output result
  psyimg *outpsyimgptr=psynewoutfile(outfile, flippedimg,
				     outfileclass, outtype, outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
// clean up
  delete flippedimg;
  delete outpsyimgptr;  // without this analyze header doesn't get written?
}
