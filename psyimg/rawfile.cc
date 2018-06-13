#include "psyhdr.h"
#include "psyanalyze.h"
#include "psyecat.h"
#include "psyecat7.h"
#include "cnusdt.h"
#include "cnudicom.h"
#include "cnunifti.h"

void swap_data(char *data, const int *soff, const int *loff, const int *foff) {
  const int *ptr;
  for (ptr = soff; *ptr >= 0; ptr++) swaper((unsigned char *) data + *ptr);
  for (ptr = loff; *ptr >= 0; ptr++) lswaper((unsigned char *) data + *ptr);
  for (ptr = foff; *ptr >= 0; ptr++) lswaper((unsigned char *) data + *ptr);
}

void writeouttypeargchoices(ostream *out) {
  *out << "-uchar | -char | -ushort | -short | -uint | -int | -float | -double";
}

psytype checkouttypearg(char *arg) {
  psytype localtype = psynotype;
  if(strcmp("-uchar", arg)==0) localtype=psyuchar;
  if(strcmp("-char", arg)==0) localtype=psychar;
  else if(strcmp("-short", arg)==0) localtype=psyshort;
  else if(strcmp("-ushort", arg)==0) localtype=psyushort;
  else if(strcmp("-int", arg)==0) localtype=psyint;
  else if(strcmp("-uint", arg)==0) localtype=psyuint;
  else if(strcmp("-float", arg)==0) localtype=psyfloat;
  else if(strcmp("-double", arg)==0) localtype=psydouble;
  return localtype;
}

void writeoutfileclassargchoices(ostream *out) {
  *out << "-analyze | -nifti | -ecat | -ecat7 | -dicom | -sdt";
}

psyfileclass checkoutfileclassarg(char *arg) {
  psyfileclass localfileclass = psynoclass;
  if(strcmp("-analyze", arg)==0) localfileclass=analyzefileclass;
  else if(strcmp("-nifti", arg)==0) localfileclass=niftifileclass;
  else if(strcmp("-ecat", arg)==0) localfileclass=ecatfileclass;
  else if(strcmp("-ecat7", arg)==0) localfileclass=ecat7fileclass;
  else if(strcmp("-dicom", arg)==0) localfileclass=dicomfileclass;
  else if(strcmp("-sdt", arg)==0) localfileclass=sdtfileclass;
  return localfileclass;
}

psyimg *psynewinfile(string filename, psyfileclass *infileclass,
		     psytype *intype, int **infileclassinfo)
{
  psyimg *psyimgptr;
  psytype localintype;
  psyfileclass localinfileclass;
  int *localinfileclassinfo=NULL;
  if(isniftifile(filename)) {
    localinfileclass=niftifileclass;
    psyimgptr=(psyimg *)new niftifile(filename, "r");
    localintype=psyimgptr->gettype();
  }
  else if(isanalyzefile(filename)) {
    localinfileclass=analyzefileclass;
    psyimgptr=(psyimg *)new analyzefile(filename, "r");
    localintype=psyimgptr->gettype();
  }
  else if(isdicomfile(filename)) {
    localinfileclass=dicomfileclass;
    psyimgptr=(psyimg *)new dicomfile(filename, "r");
    localintype=psyimgptr->gettype();
  }
  else if(isecatfile(filename)){
    localinfileclass=ecatfileclass;
    ecatfile *ecatfileptr=new ecatfile(filename);
    psyimgptr=(psyimg *)ecatfileptr;
    int ecattype=(int)ecatfileptr->get_header_value("data_type");
    localintype=ecattype2psytype(ecattype);
    localinfileclassinfo = new int[1];
    localinfileclassinfo[0] = ecatfileptr->get_ecatfiletype();
  }
  else if(isecat7file(filename)){
    localinfileclass=ecat7fileclass;
    psyimgptr=(psyimg *)new ecat7file(filename);
    localintype=psyimgptr->gettype();
  }
  else if(issdtfile(filename)){
    localinfileclass=sdtfileclass;
    psyimgptr=(psyimg *)new sdtfile(filename);
    localintype=psyimgptr->gettype();
  }
  else {
    cerr<<"psynewinfile - error unreadable file=\""<<filename<<"\"\n";
    exit(1);
  }
  if(infileclass != NULL) *infileclass=localinfileclass;
  if(intype != NULL) *intype=localintype;
  if(infileclassinfo != NULL) *infileclassinfo = localinfileclassinfo;
  else if(localinfileclassinfo != NULL) delete[] localinfileclassinfo;
  return(psyimgptr);
}

psytype getclosesttype(psyfileclass fileclass, psytype type) {
  psytype returntype;
  switch(fileclass) {
  case analyzefileclass:
    returntype =  getanalyzeclosesttype(type);
    break;
  case niftifileclass:
    returntype =  getnifticlosesttype(type);
    break;
  case ecatfileclass:
    returntype =  getecatclosesttype(type);
    break;
  case ecat7fileclass:
    returntype =  getecat7closesttype(type);
    break;
  case sdtfileclass:
    returntype =  getsdtclosesttype(type);
    break;
  case dicomfileclass:
    returntype = getdicomclosesttype(type);
    break;
  default:
    returntype = psynotype;
    break;
  }
  if(returntype == psynotype) cerr<<"getclosesttype:  unsupported data type("<<psytypenames[type]<<") for fileclass("<<psyfileclassnames[fileclass]<<")\n";
  return returntype;
}

psyimg *psynewoutfile(string filename, psyimg *psyimgptr,
		   psyfileclass outfileclass,
		   psytype outtype, int *fileclassinfo)
{
  psyimg *outimgptr;
  int ecatfiletype = IMAGE_FILE;

  if(outtype == psynotype) outtype = psyimgptr->gettype();
  psytype closesttype = getclosesttype(outfileclass, outtype);
  if(closesttype == psynotype) {
    cerr<<"psynewoutfile:  error - exitting because no appropriate output data type found\n";
    exit(1);
  }
  else if(closesttype != outtype) {
    cerr<<"psynewoutfile:  warning - outputting closest type("<<psytypenames[closesttype];
    cerr<<") supported by file class("<<psyfileclassnames[outfileclass];
    cerr<<") - different then requested type("<<psytypenames[outtype]<<")";
    cerr<<" - outfile name="<<filename<<"\n";
  }

  switch(outfileclass) {
  case analyzefileclass:
    outimgptr=(psyimg *)new outputanalyze(filename, psyimgptr, closesttype);
    break;
  case niftifileclass:
    outimgptr=(psyimg *)new niftifile(filename, psyimgptr, closesttype);
    break;
  case ecatfileclass:
    if(fileclassinfo != NULL) ecatfiletype = *fileclassinfo;
    outimgptr=(psyimg *)new ecatfile(filename, psyimgptr, closesttype,
				     ecatfiletype);
    break;
  case ecat7fileclass:
    outimgptr=(psyimg *)new ecat7file(filename, psyimgptr, closesttype,
				     ecatfiletype);
    break;
  case sdtfileclass:
    outimgptr=(psyimg *)new sdtfile(filename, psyimgptr, closesttype);
    break;
  case dicomfileclass:
    outimgptr=(psyimg *)new dicomfile(filename, psyimgptr, closesttype);
    break;
  default:
    cerr<<"psynewoutfile - unknown output file class\n";
    exit(1);
  }
  return(outimgptr);
}

void psyopenfile(string fname, const char *mode, fstream *fd, int *status)
{
  int localstatus=0;
  if(fname.length() == 0) {
    cerr << "psyopenfile - blank file name\n";
    localstatus=1;
  }
  // open the file
  if(localstatus != 1){
    if(strcmp(mode, "r") == 0) fd->open(fname.c_str(), ios::in);
    else if(strcmp(mode, "w") == 0)fd->open(fname.c_str(), ios::out);
    else {
      cerr << "psyopenfile - unknown mode for opening file\n";
      localstatus=1;
    }
    if(fd->fail() || fd->bad())localstatus=1;
  }
  if(status != NULL) *status=localstatus; // just return status don't exit
  else if(localstatus) {
    cerr << "psyopenfile - error opening file: " << fname << '\n';
    exit(1);
  }
}

rawfile::rawfile(psyimg *psyimgptr, psytype pixeltype)
     : psyimglnk(psyimgptr, pixeltype)
{
  swaptype = psynoswap;
  getnextpixel_lock=0;
}

rawfile::rawfile(string fname, const char *mode)
{
  psyopenfile(fname, mode, &imgfd);
  imgfilename=fname;
  swaptype = psynoswap;
  getnextpixel_lock=0;
}

rawfile::rawfile(string fname, const char *mode, psyimg *psyimgptr) :
       psyimglnk(psyimgptr)
{
  psyopenfile(fname, mode, &imgfd);
  imgfilename=fname;
  swaptype = psynoswap;
  getnextpixel_lock=0;
}

rawfile::rawfile(string fname, const char *mode, int xdim, int ydim, int zdim,
		 int idim, psytype pixeltype,
		 int xorig, int yorig, int zorig, int iorig,
		 int skippixels, double xres, double yres,
		 double zres, double ires, double wres)
{
  initpsyimg(xdim, ydim, zdim, idim, pixeltype, xorig, yorig, zorig,
	     iorig, skippixels, xres, yres, zres, ires, wres);
  psyopenfile(fname, mode, &imgfd);
  imgfilename=fname;
  swaptype = psynoswap;
  getnextpixel_lock=0;
}

rawfile::~rawfile()
{
  imgfd.close();
}

inline void reversebytes(char *c, int bytes) {
  // reverse bytes
  // 0,1->1,0; 0,1,2,3->3,2,1,0; 0,1,2,3,4,5,6,7->7,6,5,4,3,2,1,0
  int b1 = 0;
  int b2 = bytes - 1;
  for(; b2 > b1; b2--, b1++) {
    char tmp = c[b2]; c[b2] = c[b1]; c[b1] = tmp;
  }
}

void rawfile::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			int iorig, int xend, int yend, int zend, int iend,
			int xinc, int yinc, int zinc, int iinc,
			psytype pixeltype)
{
  int x, y, z, i;
  int xbytes, ybytes, zbytes, ibytes;
  int seekx, seeky, seekz, seeki;
  char *xoptr, *yoptr, *zoptr, *ioptr;
  char_or_largest_pixel temp;

  if(!inside(xorig, yorig, zorig, iorig) || 
     !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr << ":analfile::copyblock - block area not contained in file\n";
    exit(1);
  }
  if(getnextpixel_lock) {
    output_tree(&cerr);
    cerr << ":rawfile::copyblock - programming error - get pixel locked\n";
    cerr << "read buffer locked by initgetpixels\n";
    exit(1);
  }
// calculate bytes moved in loops
  xbytes = getbytesperpixel();
  ybytes = (xend-xorig + 1)*inc.x;
  zbytes = (yend-yorig + 1)*inc.y;
  ibytes = (zend-zorig + 1)*inc.z;
// calculate seeks needed in loops
  seekx = inc.x - xbytes;
  seeky = inc.y - ybytes;
  seekz = inc.z - zbytes;
  seeki = inc.i - ibytes;
// seek to initial position in file
  imgfd.clear(); // clear any existing io errors from things like EOF
  imgfd.seekg(offset(xorig, yorig, zorig, iorig), ios::beg);
  // 12/23/02 - moved seeks to beginning of loops to avoid reading
  // beyond end of file while correcting previous logic error
  // but don't perform a seek the first time through the loop
  int doseek=0;
// transfer data the most efficient way possible
  ioptr=outbuff;
  if((swaptype != psynoswap) && (xbytes > 1)) {
    switch (swaptype) {
    default:
      output_tree(&cerr);
      cerr << ":rawfile::copyblock - unimplement swap type="<<swaptype<<"\n";
      exit(1);
    case psyreversewordbytes:
      for(i=iorig; i<=iend; i++, ioptr+=iinc) {
	if(doseek)imgfd.seekg(seeki, ios::cur);
	for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	  if(doseek)imgfd.seekg(seekz, ios::cur);
	  for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=yinc) {
	    if(doseek)imgfd.seekg(seeky, ios::cur);
	    for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=xinc) {
	      if(doseek)imgfd.seekg(seekx, ios::cur);
	      else doseek = 1; // perform seeks next time thru loops
	      imgfd.read(temp.c, xbytes);
	      // reverse input word bytes
	      // 0,1->1,0; 0,1,2,3->3,2,1,0; 0,1,2,3,4,5,6,7->7,6,5,4,3,2,1,0
	      reversebytes(temp.c, xbytes);
	      pixeltypechg(temp.c, type, xoptr, pixeltype);
	    } // end for(x=0
	  }  // end for(y=0
	} // end for(z=0
      } // end for(i=0
      break;
    }
  }
  else if(pixeltype != type) {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      if(doseek)imgfd.seekg(seeki, ios::cur);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	if(doseek)imgfd.seekg(seekz, ios::cur);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=yinc) {
	  if(doseek)imgfd.seekg(seeky, ios::cur);
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=xinc) {
	    if(doseek)imgfd.seekg(seekx, ios::cur);
	    else doseek = 1; // perform seeks next time thru loops
	    imgfd.read(temp.c, xbytes);
	    pixeltypechg(temp.c, type, xoptr, pixeltype);
	  } // end for(x=0
	}  // end for(y=0
      } // end for(z=0
    } // end for(i=0
  }
  else if(seekx != 0 || xinc != inc.x) {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      if(doseek)imgfd.seekg(seeki, ios::cur);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	if(doseek)imgfd.seekg(seekz, ios::cur);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=yinc) {
	  if(doseek)imgfd.seekg(seeky, ios::cur);
	  for(x=xorig, xoptr=yoptr; x<=xend; x++, xoptr+=xinc) {
	    if(doseek)imgfd.seekg(seekx, ios::cur);
	    else doseek = 1; // perform seeks next time thru loops
	    imgfd.read(xoptr, xbytes);
	  } // end for(x=0
	}  // end for(y=0
      } // end for(z=0
    } // end for(i=0
  }
  else if(seeky != 0 || yinc != inc.y) {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      if(doseek)imgfd.seekg(seeki, ios::cur);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	if(doseek)imgfd.seekg(seekz, ios::cur);
	for(y=yorig, yoptr=zoptr; y<=yend; y++, yoptr+=yinc) {
	  if(doseek)imgfd.seekg(seeky, ios::cur);
	  else doseek = 1; // perform seeks next time thru loops
	  imgfd.read(yoptr, ybytes);
	}  // end for(y=0
      } // end for(z=0
    } // end for(i=0
  }
  else if(seekz != 0 || zinc != inc.z) {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      if(doseek)imgfd.seekg(seeki, ios::cur);
      for(z=zorig, zoptr=ioptr; z<=zend; z++, zoptr+=zinc) {
	if(doseek)imgfd.seekg(seekz, ios::cur);
	else doseek = 1; // perform seeks next time thru loops
	imgfd.read(zoptr, zbytes);
      } // end for(z=0
    } // end for(i=0
  }
  else {
    for(i=iorig; i<=iend; i++, ioptr+=iinc) {
      if(doseek)imgfd.seekg(seeki, ios::cur);
      else doseek = 1; // perform seeks next time thru loops
      imgfd.read(ioptr, ibytes);
    } // end for(i=0
  } 
}

void rawfile::initgetpixel(psytype pixeltype, int x, int y, int z, int i)
{
  if(!inside(x, y, z, i)) { 
    output_tree(&cerr);
    cerr << ":rawfile::copyblock - pixel location not in file\n";
    exit(1);
  }
  if(type != pixeltype) {
    output_tree(&cerr);
    cerr << ":rawfile::initgetpixel - unequal pixel types\n";
    exit(1);
  }
  if(getnextpixel_lock) {
    output_tree(&cerr);
    cerr << ":rawfile::initgetpixel - programming error - get pixel locked\n";
    cerr << "add a buffer or check for missing call to freegetpixel\n";
    exit(1);
  }
  imgfd.seekg(offset(x,y,z,i), ios::beg);
  getnextpixel_lock=1;
}

void rawfile::getpixel(char *pixel, psytype pixeltype,
			int x, int y, int z, int i)
{
  initgetpixel(pixeltype, x, y, z, i);
  int nbytes = getbytesperpixel();
  imgfd.read(pixel, nbytes);
  if(swaptype == psyreversewordbytes) reversebytes(pixel, nbytes);
  freegetpixel();
}

void rawfile::getnextpixel(char *pixel)
{
  // no checks assumes programmer called initgetpixel and knows image size
  int nbytes = getbytesperpixel();
  imgfd.read(pixel, nbytes);
  if(swaptype == psyreversewordbytes) reversebytes(pixel, nbytes);
}

void rawfile::writedata()
{
  int xinc, yinc, zinc, iinc;
  int z, i;
  double min, max, mean, sum, sqrsum;
  min=max=mean=sum=sqrsum=0;
  double tmpmin, tmpmax, tmpmean, tmpsum, tmpsqrsum;
  int count;
  convertimg *convertimgptr=NULL;
  psyimg *psyimgptr=inputpsyimg;
// convert to type if not same as input
  if(psyimgptr->gettype() != type) {
    convertimgptr = new convertimg(psyimgptr, type);
    psyimgptr = (psyimg *)convertimgptr;
  }
// initialize output buffer for one plane at a time
  psybuff buffimage(size.x, size.y, 1, 1, type,
		    orig.x, orig.y, orig.z, orig.i);
// get increment values for buffer
  buffimage.getinc(&xinc, &yinc, &zinc, &iinc);
// loop through planes
  count=0;
  for(i=orig.i; i<=end.i; i++) {
    for(z=orig.z; z<=end.z; z++) {
// transfer one plane of data to output buffer
      psyimgptr->copyblock(buffimage.getbuff(), orig.x, orig.y, z, i,
			   end.x, end.y, z, i, xinc, yinc, zinc, iinc,
			   buffimage.gettype());
// keep running stats
      buffimage.unsetstats();
      buffimage.getstats(&tmpmin, &tmpmax, &tmpmean, &tmpsum, &tmpsqrsum);
      if(count==0){
	min=tmpmin; max=tmpmax; mean=tmpmean;
	sum=tmpsum; sqrsum=tmpsqrsum;
      }
      else {
	min = (tmpmin < min)? tmpmin : min;
	max = (tmpmax > max)? tmpmax : max;
	mean += tmpmean;
	sum += tmpsum;
	sqrsum += tmpsqrsum;
      }
      count++;
// swap data if needed
      if(swaptype != psynoswap) {
	int xbytes = buffimage.getbytesperpixel();
	if(xbytes > 1) {
	  switch (swaptype) {
	  default:
	    output_tree(&cerr);
	    cerr << ":rawfile::writedata - unimplement swap type\n";
	    exit(1);
	  case psyreversewordbytes:
	    int x, y;
	    char *yptr, *xptr;
	    for(y=orig.y, yptr=buffimage.getbuff(); y<=end.y;
		y++, yptr+=yinc) {
	      for(x=orig.x, xptr=yptr; x<=end.x; x++, xptr+=xinc)
		reversebytes(xptr, xbytes);
	    }  // end for(y=yorig
	  } // end switch
	}
      }
// write this plane to the img file
      imgfd.write(buffimage.getbuff(), buffimage.getlength());
      if(imgfd.bad()) {
	cerr<<"rawfile::writedata: error writing to file: "
	  <<imgfilename<<'\n';
	exit(1);
      }
    } // end for(z=
  } // end for(i=

// set stats of output image
  if(count != 0)mean /= count;
  setstats(min, max, mean, sum, sqrsum);
// clean up
  delete convertimgptr;
}
