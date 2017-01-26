#include "psyhdr.h"
#include "cnusdt.h"

void bldsdtfilenames(string primaryname, string *imgname,
			 string *hdrname)
{
  size_t namelength = primaryname.length();
  if(namelength == 0) {
    cerr<<"bldsdtfilenames - empty file name\n";
    exit(1);
  }
// remove .sdt from end of name
  if(namelength > 5) {
    string ending = primaryname.substr(namelength-4, 4);
    if(ending.compare(".sdt") == 0 ||
       ending.compare(".spr") == 0) {
      primaryname.erase(namelength-4);
    }
  }
  // return strings
  if(hdrname != NULL)*hdrname=primaryname + ".sdt";
  if(imgname != NULL)*imgname=primaryname + ".spr";
}

sdtheader::sdtheader() {
  numDim=0;
  dim=NULL;
  origin=NULL;
  extent=NULL;
  fov=NULL;
  interval=NULL;
  dataType="";
  displayRange[0] = 0; displayRange[1] = 0;
  fidName="";
  sdtOrient = "";
  real2WordScale=1.0;
}

sdtheader::~sdtheader() {
  delete[] dim;
  delete[] origin;
  delete[] extent;
  delete[] fov;
  delete[] interval;
}


int sdtheader::getSize(int i) {
  if(i < numDim) return dim[i];
  else return 1;
}

double sdtheader::getOrigin(int i) {
  if(i < numDim) return origin[i];
  else return 0.0;
}

double sdtheader::getExtent(int i) {
  if(i < numDim) return extent[i];
  else return 0.0;
}

double sdtheader::getFov(int i) {
  if(i < numDim) return fov[i];
  else return 0.0;
}

double sdtheader::getInterval(int i) {
  if(i < numDim) return interval[i];
  else return 0.0;
}

psytype sdtheader::getPsyType() {
  const char *tmp_dataType_str = dataType.c_str();
  if(strncasecmp(tmp_dataType_str, "BYTE", 4) == 0) return psyuchar;
  else if(strncasecmp(tmp_dataType_str, "WORD", 4) == 0) return psyushort;
  else if(strncasecmp(tmp_dataType_str, "LWORD", 4) == 0) return psyint;
  else if(strncasecmp(tmp_dataType_str, "REAL", 4) == 0) return psyfloat;
  cerr<<"sdtheader::getPsyType - unknown data type="<<dataType<<"\n";
  return psynotype;
}

string psytype2sdtname(psytype type) {
  switch (type) {
  case psyuchar:
    return "BYTE";
  case psyushort:
    return "WORD";
  case psyint:
    return "LWORD";
  case psyfloat:
    return "REAL";
  default:
    return "UNKNOWN";
  }
}

psytype getsdtclosesttype(psytype type) {
  switch (type) {
  case psyuchar:
    return psyuchar;
  case psyushort:
    return psyushort;
  case psychar:
  case psyshort:
  case psyshortsw:
  case psyuint:
  case psyint:
    return psyint;
  case psydouble:
  case psyfloat:
    return psyfloat;
  default:
    return psynotype;
  }
}

double sdtheader::getWordRes() {
  if(fabs(real2WordScale) > 1e-16) return 1.0/real2WordScale;
  else return 0.0;
}

psyorient sdtheader::getPsyOrient() {
  if(strcasecmp(sdtOrient.c_str(), "ax") == 0) return psytransverse;
  return psynoorient;
}

string psyorient2sdtname(psyorient orient) {
  switch(orient) {
  case psytransverse:
  default:
    return "ax";
  }
}

sdtheader *readSdtHeader(string name, sdtheader *rawhdr,
			 fstream **fdptrptr)
{
  fstream *fd = NULL;
  if(fdptrptr != NULL) fd = *fdptrptr;
  if((name.empty()) && (fd == NULL)) return NULL;
  if(fd == NULL) {
    string hdrname;
    bldsdtfilenames(name, NULL, &hdrname);
    fd = new fstream(hdrname.c_str(), ios::in);
    if(fd->fail() || fd->bad()) {
      delete fd;
      return NULL;
    }
    if(fdptrptr != NULL) *fdptrptr = fd;
  }
  else fd->seekg(0);  // beginning of file

  sdtheader *localrawhdr = rawhdr;
  if(localrawhdr == NULL) localrawhdr = new sdtheader();

  // read the file
  char linebuff[1024], c;
  int error=0;
  while((! fd->eof()) && fd->good() && (! error)) {
    //remove left over carriage return
    if(fd->peek() == '\n') fd->get(c);
    fd->get(linebuff, 1024-1);
    if(fd->good()) {
      // parse the line
      char *cptr = strchr(linebuff, ':');
      if(cptr == NULL) {
      }
      else {
	// convert ':' to end of string character '\0'
	*cptr='\0'; cptr++;
	// now parse value
	int itmp; double dtmp;
	while(*cptr == ' ') cptr++;
	if(strcasecmp(linebuff, "numDim") == 0) {

	  if(sscanf(cptr, "%d", &itmp) != 1) {
	    cerr<<"error reading numDim\n";
	    error=1;
	  }
	  else if(itmp < 1) {
	    cerr<<"invalid numDim=" << itmp;
	    cerr<<" should be positive\n";
	    error=1;
	  }
	  else {
	    localrawhdr->numDim = itmp;
	  }
	}
	else if(strcasecmp(linebuff, "dim") == 0) {
	  if(localrawhdr->numDim < 1) {
	    cerr<<"trying to read dim and numDim not set yet";
	    error=1;
	  }
	  else if(localrawhdr->dim != NULL) {
	    cerr<<"dim already set\n";
	    error=1;
	  }
	  else {
	    localrawhdr->dim = new int[localrawhdr->numDim];
	    for(int i=0; (i<localrawhdr->numDim) && (!error); i++) {
	      if(cptr == NULL) {
		cerr<<"missing dim values\n";
		error=1;
	      }
	      else if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading dim["<<i<<"]\n";
		error=1;
	      }
	      else if(itmp < 1) {
		cerr<<"invalid dim["<<i<<"=" << itmp;
		cerr<<" should be positive\n";
		error=1;
	      }
	      else {
		localrawhdr->dim[i] = itmp;
		cptr = strchr(cptr, ' ');
		if(cptr != NULL) while(*cptr == ' ') cptr++;
	      }
	    }
	  }
	}
	else if(strcasecmp(linebuff, "origin") == 0) {
	  if(localrawhdr->numDim < 1) {
	    cerr<<"trying to read origin and numDim not set yet";
	    error=1;
	  }
	  else if(localrawhdr->origin != NULL) {
	    cerr<<"origin already set";
	    error=1;
	  }
	  else {
	    localrawhdr->origin = new double[localrawhdr->numDim];
	    for(int i=0; (i<localrawhdr->numDim) && (!error); i++) {
	      if(cptr == NULL) {
		cerr<<"missing origin values\n";
		error=1;
	      }
	      else if(sscanf(cptr, "%lf", &dtmp) != 1) {
		cerr<<"error reading origin["<<i<<"]\n";
		error=1;
	      }
	      else {
		localrawhdr->origin[i] = dtmp;
		cptr = strchr(cptr, ' ');
		if(cptr != NULL) while(*cptr == ' ')cptr++;
	      }
	    }
	  }
	}
	else if(strcasecmp(linebuff, "extent") == 0) {
	  if(localrawhdr->numDim < 1) {
	    cerr<<"trying to read extent and numDim not set yet";
	    error=1;
	  }
	  else if(localrawhdr->extent != NULL) {
	    cerr<<"extent already set";
	    error=1;
	  }
	  else {
	    localrawhdr->extent = new double[localrawhdr->numDim];
	    for(int i=0; (i<localrawhdr->numDim) && (!error); i++) {
	      if(cptr == NULL) {
		cerr<<"missing extent values\n";
		error=1;
	      }
	      else if(sscanf(cptr, "%lf", &dtmp) != 1) {
		cerr<<"error reading extent["<<i<<"]\n";
		error=1;
	      }
	      else {
		localrawhdr->extent[i] = dtmp;
		cptr = strchr(cptr, ' ');
		if(cptr != NULL) while(*cptr == ' ')cptr++;
	      }
	    }
	  }
	}
	else if(strcasecmp(linebuff, "fov") == 0) {
	  if(localrawhdr->numDim < 1) {
	    cerr<<"trying to read fov and numDim not set yet";
	    error=1;
	  }
	  else if(localrawhdr->fov != NULL) {
	    cerr<<"fov already set";
	    error=1;
	  }
	  else {
	    localrawhdr->fov = new double[localrawhdr->numDim];
	    for(int i=0; (i<localrawhdr->numDim) && (!error); i++) {
	      if(cptr == NULL) {
		cerr<<"missing fov values\n";
		error=1;
	      }
	      else if(sscanf(cptr, "%lf", &dtmp) != 1) {
		cerr<<"error reading fov["<<i<<"]\n";
		error=1;
	      }
	      else {
		localrawhdr->fov[i] = dtmp;
		cptr = strchr(cptr, ' ');
		if(cptr != NULL) while(*cptr == ' ')cptr++;
	      }
	    }
	  }
	}
	else if(strcasecmp(linebuff, "interval") == 0) {
	  if(localrawhdr->numDim < 1) {
	    cerr<<"trying to read interval and numDim not set yet";
	    error=1;
	  }
	  else if(localrawhdr->interval != NULL) {
	    cerr<<"interval already set";
	    error=1;
	  }
	  else {
	    localrawhdr->interval = new double[localrawhdr->numDim];
	    for(int i=0; (i<localrawhdr->numDim) && (!error); i++) {
	      if(cptr == NULL) {
		cerr<<"missing interval values\n";
		error=1;
	      }
	      else if(sscanf(cptr, "%lf", &dtmp) != 1) {
		cerr<<"error reading interval["<<i<<"]\n";
		error=1;
	      }
	      else {
		localrawhdr->interval[i] = dtmp;
		cptr = strchr(cptr, ' ');
		if(cptr != NULL) while(*cptr == ' ')cptr++;
	      }
	    }
	  }
	}
	else if(strcasecmp(linebuff, "dataType") == 0) {
	  localrawhdr->dataType = cptr;
	}
	else if(strcasecmp(linebuff, "displayRange") == 0) {
	  if(sscanf(cptr, "%lf %lf",
		    localrawhdr->displayRange, localrawhdr->displayRange+1)
	     != 2) {
	    cerr<<"error reading displayRange\n";
	    error=1;
	  }
	}
	else if(strcasecmp(linebuff, "fidName") == 0) {
	  localrawhdr->fidName = cptr;
	}
	else if(strcasecmp(linebuff, "sdtOrient") == 0) {
	  localrawhdr->sdtOrient = cptr;
	}
	else if(strcasecmp(linebuff, "Real2WordScale") == 0) {
	  if(sscanf(cptr, "%lf", &dtmp) != 1) {
	    cerr<<"error reading Real2WordScale\n";
	    error=1;
	  }
	  else localrawhdr->real2WordScale = dtmp;
	}
	else {
	  cerr<<"unknown sdt header="<<linebuff<<":"<<cptr<<"\n";
	}
      }
    }  // if good
  } // end while

  if(error || (localrawhdr->numDim == 0)) {
    if(rawhdr == NULL) delete localrawhdr;
    localrawhdr = NULL;
  }
  if(fdptrptr == NULL) {
    fd->close();
    delete fd;
  }
  return localrawhdr;
}

int issdtfile(string name)
{
  sdtheader *rawhdr = readSdtHeader(name);
  if(rawhdr == NULL) return 0;
  else {
    delete rawhdr;
    return 1;
  }
}

void sdtfile::opensdtfiles(string fname, const char *mode) {
  // build complete file names
  bldsdtfilenames(fname, &imgfilename, &hdrfilename);
  // open header file
  psyopenfile(hdrfilename, mode, &hdrfd);
  // open image file
  psyopenfile(imgfilename, mode, &imgfd);

#ifdef USES_LITTLE_ENDIAN
  swaptype = psyreversewordbytes;
#else
  swaptype = psynoswap;
#endif

}

sdtfile::sdtfile(string fname, const char *mode) {
  if(mode == NULL) mode = "r";

  opensdtfiles(fname, mode);

  if(strcmp(mode, "r") == 0) {
    fstream *fd = &hdrfd;
    sdtheader *tmphdr = readSdtHeader(fname, &rawhdr, &fd);
    if(tmphdr == NULL) {
      output_tree(&cerr);
      cerr << ":  error reading header(.spr) file\n";
      exit(1);
    }
    initpsyimg(rawhdr.getSize(0), rawhdr.getSize(1), rawhdr.getSize(2),
	       rawhdr.getSize(3), rawhdr.getPsyType(),
	       0,0,0,0,0,
	       rawhdr.getInterval(0)*1e-2, rawhdr.getInterval(1)*1e-2,
	       rawhdr.getInterval(2)*1e-2, rawhdr.getInterval(3),
	       rawhdr.getWordRes());
    setorient(rawhdr.getPsyOrient());
  }
}

sdtfile::sdtfile(string fname, psyimg *psyimgptr,
		 psytype pixeltype)
  : rawfile(psyimgptr, pixeltype)
{
  //  char *ptr, *endptr;

  // open files in write mode since linked
  opensdtfiles(fname, "w");

  // build the header info from the input image
  psydims dims = getsize();
  rawhdr.dim = new int[4];
  rawhdr.dim[0] = dims.x; rawhdr.dim[1] = dims.y;
  rawhdr.dim[2] = dims.z; rawhdr.dim[3] = dims.i;
  if(dims.i > 1) rawhdr.numDim = 4;
  else if(dims.z > 1) rawhdr.numDim = 3;
  else if(dims.y > 1) rawhdr.numDim = 2;
  else rawhdr.numDim = 1;

  psyres res = getres();

  rawhdr.origin = new double[rawhdr.numDim];
  rawhdr.extent = new double[rawhdr.numDim];
  rawhdr.fov = new double[rawhdr.numDim];
  rawhdr.interval = new double[rawhdr.numDim];

  rawhdr.interval[0] = res.x;
  rawhdr.fov[0] = dims.x * res.x;
  rawhdr.extent[0] = rawhdr.fov[0]/2.0;
  rawhdr.origin[0] = -rawhdr.fov[0];
  if(rawhdr.numDim > 1) {
    rawhdr.interval[1] = res.y;
    rawhdr.fov[1] = dims.y * res.y;
    rawhdr.extent[1] = rawhdr.fov[1]/2.0;
    rawhdr.origin[1] = -rawhdr.fov[1];
  }
  if(rawhdr.numDim > 2) {
    rawhdr.interval[2] = res.z;
    rawhdr.fov[2] = dims.z * res.z;
    rawhdr.extent[2] = rawhdr.fov[2]/2.0;
    rawhdr.origin[2] = -rawhdr.fov[2];
  }
  if(rawhdr.numDim > 3) {
    rawhdr.interval[3] = res.i;
    rawhdr.fov[3] = dims.i * res.i;
    rawhdr.extent[3] = rawhdr.fov[3]/2.0;
    rawhdr.origin[3] = -rawhdr.fov[3];
  }

  rawhdr.dataType = psytype2sdtname(gettype());
  rawhdr.displayRange[0] = 0;
  rawhdr.displayRange[1] = 0;
  rawhdr.fidName="unknown";
  rawhdr.sdtOrient=psyorient2sdtname(getorient());
  double dtmp = getwordres();
  rawhdr.real2WordScale = (fabs(dtmp) < 1e-16)? 1.0 : 1.0/dtmp;


  // write header file
  writeheader();
  // write the raw data
  writedata();
}

sdtfile::~sdtfile() {
  hdrfd.close();
}

void sdtfile::showhdr(sdtheader *rawhdr, ostream *out) {
  int i;
  *out << "numDim: " << rawhdr->numDim;
  *out << "\ndim:";
  for(i=0; i<rawhdr->numDim; i++) *out << " " << rawhdr->dim[i];
  *out << "\norigin:";
  for(i=0; i<rawhdr->numDim; i++) *out << " " << rawhdr->origin[i];
  *out << "\nextent:";
  for(i=0; i<rawhdr->numDim; i++) *out << " " << rawhdr->extent[i];
  *out << "\nfov:";
  for(i=0; i<rawhdr->numDim; i++) *out << " " << rawhdr->fov[i];
  *out << "\ninterval:";
  for(i=0; i<rawhdr->numDim; i++) *out << " " << rawhdr->interval[i];
  *out << "\ndataType: " << rawhdr->dataType;
  *out << "\ndisplayRange: " << rawhdr->displayRange[0];
  *out << " " << rawhdr->displayRange[1];
  *out << "\nfidName:" << rawhdr->fidName;
  *out << "\nsdtOrient:" << rawhdr->sdtOrient;
  *out << "\nReal2WordScale:" << rawhdr->real2WordScale;
  *out << '\n';
}

void sdtfile::writeheader()
{
  showhdr(&rawhdr, &hdrfd);
}
