#include "psyhdr.h"
#include "psyanalyze.h"

struct datafilehead
{
    long nblocks;        /*0 number of blocks in file     */
    long ntraces;        /*4 number of traces per block   */
    long np;             /*8 number of elements per trace */
    long ebytes;         /*12 number of bytes per element  */
    long tbytes;         /*16 number of bytes per trace    */
    long bbytes;         /*20 number of bytes per block    */
    short transf;        /*24 transposed storage flag      */
    short status;        /*26 status of whole file         */
    long spare1;         /*28 reserved for future use      */
};

struct datablockhead
{
    short scale;         /*0 scaling factor               */
    short status;        /*2 status of data in block      */
    short index;         /*4 block index                  */
    short spare3;        /*6 reserved for future use      */
    long  ctcount;       /*8 completed transients in fids */
    float lpval;         /*12 left phase in phasefile      */
    float rpval;         /*16 right phase in phasefile     */
    float lvl;           /*20 level drift correction       */
    float tlt;           /*24 tilt drift correction        */
};

class umrifile : public rawfile {
 protected:
  struct datafilehead mrihdr;
  struct datablockhead blkhdr;
 public:
  umrifile(char *fname);
  void  showhdr();
};

umrifile::umrifile(char *fname) : rawfile(fname, "r")
{
  int xinc, yinc, zinc, iinc;

  imgfd.seekg(0);
  if((sizeof(mrihdr) != 32) || (sizeof(blkhdr) != 28)) {
    cerr<<"umrifile::umrifile - error in size of header structures\n";
    exit(1);
  }
  imgfd.read((char *) &mrihdr, sizeof(mrihdr));
  if(imgfd.bad() | imgfd.eof()) {
    cerr<<"umrifile::umrifile - error reading header from "<<fname<<'\n';
    exit(1);
  }
  imgfd.read((char *) &blkhdr, sizeof(blkhdr));
  if(imgfd.bad() | imgfd.eof()) {
    cerr<<"umrifile::umrifile - error reading block header from "<<fname<<'\n';
    exit(1);
  }
  size.x=mrihdr.np; size.y=mrihdr.ntraces, size.z=mrihdr.nblocks; size.i=1;
  if(mrihdr.ebytes != sizeof(float)) {
    cerr<<"umrifile::umrifile - number of bytes/element != sizeof(float)\n";
    exit(1);
  }
  type=psyfloat;
  setorig(0,0,0,0);
  setend(orig.x+size.x-1, orig.y+size.y-1, orig.z+size.z-1, orig.i+size.i-1);
  skip=sizeof(mrihdr)+sizeof(blkhdr);
  statstatus=StatsUninitialized;
  setres(1.0, 1.0, 1.0, 1.0);
  xinc=mrihdr.ebytes; yinc=mrihdr.tbytes;
  zinc=mrihdr.bbytes; iinc=zinc*size.z;
  setinc(xinc, yinc, zinc, iinc);
  length=inc.i*size.i;
  setwordres(blkhdr.scale);
}

void umrifile::showhdr()
{
  cout<<"mrihdr.nblocks="<<mrihdr.nblocks<<'\n';
  cout<<"mrihdr.ntraces="<<mrihdr.ntraces<<'\n';
  cout<<"mrihdr.np="<<mrihdr.np<<'\n';
  cout<<"mrihdr.ebytes="<<mrihdr.ebytes<<'\n';
  cout<<"mrihdr.tbytes="<<mrihdr.tbytes<<'\n';
  cout<<"mrihdr.bbytes="<<mrihdr.bbytes<<'\n';
  cout<<"mrihdr.transf="<<mrihdr.transf<<'\n';
  cout<<"mrihdr.status="<<mrihdr.status<<'\n';
  cout<<"blkhdr.scale="<<blkhdr.scale<<'\n';
  cout<<"blkhdr.status="<<blkhdr.status<<'\n';
  cout<<"blkhdr.index="<<blkhdr.index<<'\n';
  cout<<"blkhdr.spare3="<<blkhdr.spare3<<'\n';
  cout<<"blkhdr.ctcount="<<blkhdr.ctcount<<'\n';
  cout<<"blkhdr.lpval="<<blkhdr.lpval<<'\n';
  cout<<"blkhdr.rpval="<<blkhdr.rpval<<'\n';
  cout<<"blkhdr.lvl="<<blkhdr.lvl<<'\n';
  cout<<"blkhdr.tlt="<<blkhdr.tlt<<'\n';
}


int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
//  psytype pixeltype=psyuchar; // default to input type
  double min, max, mean;
  int xdim=256, ydim=256, zdim=1, idim=1;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile outfile"<<'\n';
//      cout<<"       [-uchar | -char | -short | -int | -float | -double]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
/*
    else if(strcmp("-uchar", argv[i])==0)pixeltype=psyuchar;
    else if(strcmp("-char", argv[i])==0)pixeltype=psychar;
    else if(strcmp("-short", argv[i])==0)pixeltype=psyshort;
    else if(strcmp("-int", argv[i])==0)pixeltype=psyint;
    else if(strcmp("-float", argv[i])==0)pixeltype=psyfloat;
    else if(strcmp("-double", argv[i])==0)pixeltype=psydouble;
*/
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr<<argv[0]<<": unknown parameter: "<<argv[i]<<'\n';
      exit(1);
    }
  }
// open input files
  umrifile inputimg(infile);
  cout<<"input header:\n";
  inputimg.showhdr();
  cout<<"input psyimg stuff:\n";
  inputimg.showpsyimg();
// output image
  outputanalyze outputimg(outfile, (psyimg *)&inputimg);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);

  cout<<"\noutput image info:\n";
  outputimg.showpsyimg();
  cout<<"output stats:\n";
  outputimg.showstats();
  cout<<"output analyze headar:\n";
  outputimg.showhdr();
}
