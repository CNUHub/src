#include "psyhdr.h"

class psyNimglnk : public psyimg {
 public:
  int N;
  psyimg **inpsyimgs;
  psyNimglnk();
  ~psyNimglnk();
  psyNimglnk(int N, psyimg **psyimgptrs, psytype pixeltype=psynotype);
  void initpsyNimglnk(int N, psyimg **psyimgptrs,
		      psytype pixeltype=psynotype);
  void output_tree(ostream *out) {psyimg::output_tree(out);*out<<"::psyNimglnk";};
  psyimg *getlink(int n);
};

psyNimglnk::psyNimglnk() : psyimg()
{
  N=0;
  inpsyimgs=NULL;
}

psyNimglnk::~psyNimglnk()
{
  if(inpsyimgs != NULL)delete[] inpsyimgs;
}

psyNimglnk::psyNimglnk(int N, psyimg **psyimgptrs,
		       psytype pixeltype)
{
  initpsyNimglnk(N, psyimgptrs, pixeltype);
}

void psyNimglnk::initpsyNimglnk(int N, psyimg **psyimgptrs,
				psytype pixeltype)
{
  psydims dims;
  xyziint orig;
  psyres res;
  if(N==0 || psyimgptrs==NULL){
    output_tree(&cerr);
    cerr<<":psyNimglnk::initpsy2imglnk - error no psyimgptrs\n";
    exit(1);
  }
// get sizes
  dims=psyimgptrs[0]->getsize();
  orig=psyimgptrs[0]->getorig();
  res=psyimgptrs[0]->getres();
// use requested output pixel type or default to input 1 image type
  if(pixeltype==psynotype)pixeltype=psyimgptrs[0]->gettype();
// set extents to same as first input image
  initpsyimg(dims.x,dims.y,dims.z,dims.i,pixeltype,orig.x,orig.y,orig.z,orig.i,
	     0,res.x,res.y,res.z,res.i,psyimgptrs[0]->getwordres());
// set date and time from first input image
  setdate(psyimgptrs[0]->getdate());
  settime(psyimgptrs[0]->gettime());
// set orientation form first input image
  setorient(psyimgptrs[0]->getorient());
// set patient id from first input image
  setpatientid(psyimgptrs[0]->getpatientid());
// set description from first input image
  setdescription(psyimgptrs[0]->getdescription());

// now allocate array and store pointers to all input images
  psyNimglnk::N=N;
  inpsyimgs=new psyimg * [N];
  for(int n=0; n<N; n++){
    inpsyimgs[n]=psyimgptrs[n];
    psydims curdim=psyimgptrs[n]->getsize();
// verify all images have the same dimensions
    if(! inpsyimgs[n]->samedim(dims.x, dims.y, dims.z, dims.i)) {
      output_tree(&cerr);
      cerr<<":psyNimglnk::initpsyNimglnk - images must be the same dimensions\n";
      exit(1);
    }
  }
}

psyimg *psyNimglnk::getlink(int n)
{
  if((inpsyimgs != NULL) && (n < psyNimglnk::N))return(inpsyimgs[n]);
  return NULL;
}

class procNimgs : public psyNimglnk {
 protected:
  psytype *intypes;
  char_or_largest_pixel *inpixels;
  virtual void procNpixels(char *out, psytype outtype)=0;
 public:
  procNimgs();
  ~procNimgs();
  procNimgs(int N, psyimg **psyimgptrs, psytype pixeltype=psynotype);
  void initprocNimgs(int N, psyimg **psyimgptrs, psytype pixeltype=psynotype);
  void output_tree(ostream *out) {psyNimglnk::output_tree(out);*out<<"::procNimgs";};
  void copyblock(char *outbuff, int xorig, int yorig, int zorig,
		 int iorig, int xend, int yend, int zend, int iend,
		 int out_xinc, int out_yinc,
		 int out_zinc, int out_iinc,
		 psytype pixeltype=psyuchar);
  void getpixel(char *pixel, psytype pixeltype, int x, int y, int z, int i);
  void initgetpixel(psytype pixeltype, int x, int y, int z, int i);
  void freegetpixel();
  void getnextpixel(char *pixel);
};

procNimgs::procNimgs() {
  intypes=NULL;
  inpixels=NULL;
}

procNimgs::~procNimgs() {
  if(intypes != NULL)delete[] intypes;
  if(inpixels != NULL)delete[] inpixels;
}

procNimgs::procNimgs(int N, psyimg **psyimgptrs,
		     psytype pixeltype) {
  initprocNimgs(N, psyimgptrs, pixeltype);
}

void procNimgs::initprocNimgs(int N, psyimg **psyimgptrs,
			      psytype pixeltype) {
  initpsyNimglnk(N, psyimgptrs, pixeltype);
  intypes=new psytype[N];
  inpixels=new char_or_largest_pixel[N];
  for(int n=0; n<N; n++)intypes[n]=inpsyimgs[n]->gettype();
}

void procNimgs::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			  int iorig, int xend, int yend, int zend, int iend,
			  int out_xinc, int out_yinc,
			  int out_zinc, int out_iinc,
			  psytype pixeltype)
{
  int x, y, z, i;
  int n;
  char *xoptr, *yoptr, *zoptr, *ioptr;
// select most efficient loop to use
  ioptr=outbuff;
  if(xorig != orig.x || xend != end.x) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(n=0; n<N; n++)
	    inpsyimgs[n]->initgetpixel(intypes[n], xorig, y, z, i);
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
	    for(n=0; n<N; n++)inpsyimgs[n]->getnextpixel(inpixels[n].c);
            procNpixels(xoptr, pixeltype);
	  }//end for(x=
	  for(n=0; n<N; n++)
	    inpsyimgs[n]->freegetpixel();
        }//end for(y=
      }//end for(z=
    }//end for(i=
  } else if(yorig != orig.y || yend != end.y) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(n=0; n<N; n++)
	  inpsyimgs[n]->initgetpixel(intypes[n], xorig, yorig, z, i);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
	    for(n=0; n<N; n++)inpsyimgs[n]->getnextpixel(inpixels[n].c);
            procNpixels(xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
	for(n=0; n<N; n++)
	  inpsyimgs[n]->freegetpixel();
      }//end for(z=
    }//end for(i=
  } else if(zorig != orig.z || zend != end.z) {
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(n=0; n<N; n++)
	inpsyimgs[n]->initgetpixel(intypes[n], xorig, yorig, zorig, i);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
	    for(n=0; n<N; n++)inpsyimgs[n]->getnextpixel(inpixels[n].c);
            procNpixels(xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
      for(n=0; n<N; n++)
	inpsyimgs[n]->freegetpixel();
    }//end for(i=
  } else {
    for(n=0; n<N; n++)
      inpsyimgs[n]->initgetpixel(intypes[n], xorig, yorig, zorig, iorig);
    for(i=iorig; i<=iend; i++, ioptr+=out_iinc) {
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=out_zinc) {
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=out_yinc) {
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=out_xinc) {
// use sequential getpixel routines
	    for(n=0; n<N; n++)inpsyimgs[n]->getnextpixel(inpixels[n].c);
            procNpixels(xoptr, pixeltype);
	  }//end for(x=
        }//end for(y=
      }//end for(z=
    }//end for(i=
    for(n=0; n<N; n++)
      inpsyimgs[n]->freegetpixel();
  }
}

void procNimgs::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
// sequential get pixel will use the same type
  type=pixeltype;
  for(int n=0; n<N; n++)
    inpsyimgs[n]->initgetpixel(intypes[n], x, y, z, i);
}

void procNimgs::freegetpixel(){
  for(int n=0; n<N; n++)inpsyimgs[n]->freegetpixel();
};

void procNimgs::getpixel(char *outpixel, psytype pixeltype,
			int x, int y, int z, int i)
{
// sequential get pixel will use the same type
  type=pixeltype;
  for(int n=0; n<N; n++)
    inpsyimgs[n]->getpixel(inpixels[n].c, intypes[n], x, y, z, i);
  procNpixels(outpixel, pixeltype);
}

void procNimgs::getnextpixel(char *outpixel)
{
  for(int n=0; n<N; n++)inpsyimgs[n]->getnextpixel(inpixels[n].c);
  procNpixels(outpixel, type);
}

class ttestimgs : public procNimgs {
  void procNpixels(char *out, psytype outtype);
  int mincnt1, mincnt2;
 public:
  ttestimgs(psyimg *meanimg1, psyimg *meanimg2,
	    psyimg *stdimg1, psyimg *stdimg2,
	    psyimg *cntimg1, psyimg *cntimg2,
	    int mincnt=2, psytype pixeltype=psynotype);
  ttestimgs(psyimg *meanimg1, psyimg *meanimg2,
	    psyimg *stdimg1, psyimg *stdimg2,
	    psyimg *cntimg1, psyimg *cntimg2,
	    int mincnt1, int mincnt2,
	    psytype pixeltype=psynotype);
  void output_tree(ostream *out) {procNimgs::output_tree(out);*out<<"::ttestimgs";};
};

ttestimgs::ttestimgs(psyimg *meanimg1, psyimg *meanimg2,
		     psyimg *stdimg1, psyimg *stdimg2,
		     psyimg *cntimg1, psyimg *cntimg2,
		     int mincnt, psytype pixeltype) {
  psyimg *imgs[6];
  imgs[0]=meanimg1;
  imgs[1]=meanimg2;
  imgs[2]=stdimg1;
  imgs[3]=stdimg2;
  imgs[4]=cntimg1;
  imgs[5]=cntimg2;
  if(mincnt < 2) {
    output_tree(&cerr);
    cerr<<":ttestimgs::ttestimgs - minimum count must be at least 2\n";
    cerr<<"mincnt="<<mincnt<<'\n';
    exit(1);
  }
  ttestimgs::mincnt1=mincnt;
  ttestimgs::mincnt2=mincnt;
  initprocNimgs(6, imgs, pixeltype);
}

ttestimgs::ttestimgs(psyimg *meanimg1, psyimg *meanimg2,
		     psyimg *stdimg1, psyimg *stdimg2,
		     psyimg *cntimg1, psyimg *cntimg2,
		     int mincnt1, int mincnt2,
		     psytype pixeltype) {
  psyimg *imgs[6];
  imgs[0]=meanimg1;
  imgs[1]=meanimg2;
  imgs[2]=stdimg1;
  imgs[3]=stdimg2;
  imgs[4]=cntimg1;
  imgs[5]=cntimg2;
  if(mincnt1 < 2) {
    output_tree(&cerr);
    cerr<<":ttestimgs::ttestimgs - minimum count must be at least 2\n";
    cerr<<"mincnt1="<<mincnt1<<'\n';
    exit(1);
  }
  if(mincnt2 < 2) {
    output_tree(&cerr);
    cerr<<":ttestimgs::ttestimgs - minimum count must be at least 2\n";
    cerr<<"mincnt2="<<mincnt2<<'\n';
    exit(1);
  }
  ttestimgs::mincnt1=mincnt1;
  ttestimgs::mincnt2=mincnt2;
  initprocNimgs(6, imgs, pixeltype);
}


void ttestimgs::procNpixels(char *out, psytype outtype) {
  double mean1, mean2;
  type2double(inpixels[0].c, intypes[0], (char *)&mean1);
  type2double(inpixels[1].c, intypes[1], (char *)&mean2);
  double std1, std2;
  type2double(inpixels[2].c, intypes[2], (char *)&std1);
  type2double(inpixels[3].c, intypes[3], (char *)&std2);
  int cnt1, cnt2;
  type2int(inpixels[4].c, intypes[4], (char *)&cnt1);
  type2int(inpixels[5].c, intypes[5], (char *)&cnt2);

  double tvalue;
  if(cnt1 < mincnt1 || cnt2 < mincnt2)tvalue=0;
  else {
    int df=cnt1+cnt2-2;
    double svar=((cnt1-1)*(std1*std1) + (cnt2-1)*(std2*std2))/df;
    tvalue=(mean1-mean2)/sqrt(svar*((1.0/cnt1) + (1.0/cnt2)));
  }
  // convert to desired output type
  pixeltypechg((char *)&tvalue, psydouble, out, outtype);
}

int main(int argc, char *argv[])
{
  char *meanfile1=NULL, *meanfile2=NULL;
  char *stdfile1=NULL, *stdfile2=NULL;
  char *cntfile1=NULL, *cntfile2=NULL;
  char *outfile=NULL;
  int mincnt1=2;
  int mincnt2=2;
  double min, max, mean;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file 1 format
  int *outfileclassinfo = NULL;
  double default_value=0;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout <<"Usage: "<<argv[0]<<" -m1 meanfile1 -s1 stdfile1 -n1 cntfile1\n";
      cout <<"       -m2 meanfile2 -s2 stdfile2 -n2 cntfile2 outfile\n";
      cout <<"       [-mc mincnt | -mc1 mincnt1 -mc2 mincnt2]\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-m1", argv[i])==0) && ((i+1)<argc))meanfile1=argv[++i];
    else if((strcmp("-m2", argv[i])==0) && ((i+1)<argc))meanfile2=argv[++i];
    else if((strcmp("-s1", argv[i])==0) && ((i+1)<argc))stdfile1=argv[++i];
    else if((strcmp("-s2", argv[i])==0) && ((i+1)<argc))stdfile2=argv[++i];
    else if((strcmp("-n1", argv[i])==0) && ((i+1)<argc))cntfile1=argv[++i];
    else if((strcmp("-n2", argv[i])==0) && ((i+1)<argc))cntfile2=argv[++i];
    else if((strcmp(argv[i],"-mc")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%d",&mincnt1);
      mincnt2=mincnt1;
    }
    else if((strcmp(argv[i],"-mc1")==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&mincnt1);
    else if((strcmp(argv[i],"-mc2")==0)&&((i+1)<argc))
      sscanf(argv[++i],"%d",&mincnt2);
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
  if(meanfile1==NULL || meanfile2==NULL) {
    cerr << argv[0] << ": missing a mean file\n";
    exit(1);
  }
  if(stdfile1==NULL || stdfile2==NULL) {
    cerr << argv[0] << ": missing a std file\n";
    exit(1);
  }
  if(cntfile1==NULL || cntfile2==NULL) {
    cerr << argv[0] << ": missing a cnt file\n";
    exit(1);
  }
  if(outfile==NULL) {
    cerr << argv[0] << ": missing output file\n";
    exit(1);
  }


// open input files
  psyfileclass infileclass;
  psytype infiletype;
  psyimg *meanimg1=psynewinfile(meanfile1, &infileclass, &infiletype, &outfileclassinfo);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;
// internally use type that is greater of outfiletype and input image type
  psytype workingtype=meanimg1->gettype();
  if(gettypebytes(outtype) > gettypebytes(workingtype))workingtype=outtype;
  psyimg *meanimg2=psynewinfile(meanfile2, &infileclass, &infiletype);
  if(gettypebytes(meanimg2->gettype()) > gettypebytes(workingtype))
    workingtype=meanimg2->gettype();
// open standard deviation files
  psyimg *stdimg1=psynewinfile(stdfile1, &infileclass, &infiletype);
  psyimg *stdimg2=psynewinfile(stdfile2, &infileclass, &infiletype);
// open count files
  psyimg *cntimg1=psynewinfile(cntfile1, &infileclass, &infiletype);
  psyimg *cntimg2=psynewinfile(cntfile2, &infileclass, &infiletype);
// page buffer inputs
  psypgbuff meanimg1buffered(meanimg1, 1);
  psypgbuff meanimg2buffered(meanimg2, 1);
  psypgbuff stdimg1buffered(stdimg1, 1);
  psypgbuff stdimg2buffered(stdimg2, 1);
  psypgbuff cntimg1buffered(cntimg1, 1);
  psypgbuff cntimg2buffered(cntimg2, 1);

// generate t test image
  ttestimgs ttestimg((psyimg *)&meanimg1buffered, (psyimg *)&meanimg2buffered,
		     (psyimg *)&stdimg1buffered, (psyimg *)&stdimg2buffered,
		     (psyimg *)&cntimg1buffered, (psyimg *)&cntimg2buffered,
		     mincnt1, mincnt2, outtype);
// build new description
  char desc[strlen(meanfile1) + strlen(meanfile2) + 25];
  strcpy(desc, "ttestimg: ");
  strcat(desc, meanfile1); strcat(desc, "-");
  strcat(desc, meanfile2);
  ttestimg.setdescription(desc);
// set date and time to current date and time
  ttestimg.setdate();
  ttestimg.settime();
// output result to a file
  psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg *)&ttestimg,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(meanfile1);
  log.loginfilelog(meanfile2);
  log.loginfilelog(stdfile1);
  log.loginfilelog(stdfile2);
  log.loginfilelog(cntfile1);
  log.loginfilelog(cntfile2);
// print out images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
  delete outpsyimgptr;
  delete meanimg2;
  delete meanimg1;
  delete stdimg1;
  delete stdimg2;
  delete cntimg1;
  delete cntimg2;
}
