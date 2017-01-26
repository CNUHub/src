#include "psyhdr.h"
#include "psyanalyze.h"
#include "cnunifti.h"
// need ecat swap routines
#include "psyecat.h"

psytype getnifticlosesttype(psytype type) {
  switch (type) {
  case psyuchar:
    return psyuchar;
  case psychar:
    return psychar;
  case psyshortsw:
  case psyshort:
    return psyshort;
  case psyushort:
    return psyushort;
  case psyuint:
    return psyuint;
  case psyint:
    return psyint;
  case psyfloat:
    return psyfloat;
  case psydouble:
    return psydouble;
  case psyrgb:
    return(psyrgb);
  case psycomplex:
  case psydate:
  case psystring:
  case psydicomdataelement:
  case psyargb:
  case psynotype:
  default:
    return psynotype;
  }
}

int psytype2nifti(psytype pixeltype)
{
//0=unknown,1=binary,2=uchar,4=short,
//8=int,16=float,32=complex,64=double

  switch(pixeltype) {
  case psyuchar:
    return(DT_UNSIGNED_CHAR);
  case psychar:
    return(DT_INT8);
  case psyshort:
    return(DT_SIGNED_SHORT);
  case psyushort:
    return(DT_UINT16);
  case psyint:
    return(DT_SIGNED_INT);
  case psyuint:
    return(DT_UINT32);
  case psyfloat:
    return(DT_FLOAT);
  case psydouble:
    return(DT_DOUBLE);
  case psycomplex:
    return(DT_COMPLEX);
  case psyrgb:
    return(DT_RGB);
  default:
    break;
  }
  cerr<<"psytype2nifti - don't know equivalent type\n";
  exit(1);
}

psytype nifti2psytype(int niftitype, int pixelbits)
{
  //0=unknown,1=binary,2=uchar,4=short,
  //8=int,16=float,32=complex,64=double
  switch(niftitype) {
  case DT_UNKNOWN:
    // some short sighted programs(ie. makeheader)
    // don't set type so try to base it on bits
    switch(pixelbits) {
    case 8:
      return(psyuchar);
    case 16:
      return(psyshort);
    case 32:
      return(psyint);
    case 64:
      return(psydouble);
    default:
      cerr<<"nifti2psytype - don't know equivalent type\n";
    }
  case DT_UNSIGNED_CHAR:
    return(psyuchar);
  case DT_SIGNED_SHORT:
    return(psyshort);
  case DT_SIGNED_INT:
    return(psyint);
  case DT_FLOAT:
    return(psyfloat);
  case DT_DOUBLE:
    return(psydouble);
  case DT_INT8:
    return psychar;
  case DT_UINT16:
    return(psyushort);
  case DT_UINT32:
    return(psyuint);
  case DT_COMPLEX:
    return(psycomplex);
  case DT_RGB:
    return(psyrgb);
  }
  cerr<<"nifti2psytype - don't know equivalent type\n";
  exit(1);
}
/**
 * Outputs name of NIFTI datatype.
 *
 * @param datatype data type
 * @param out pointer to output stream
 */
void outputNiftiDataTypeName(int datatype, ostream *out) {
  switch (datatype) {
  default:
    *out << "unknown data type " << datatype;
    break;
  case NIFTI_TYPE_UINT8:
    *out << "NIFTI_TYPE_UINT8";
    break;
  case NIFTI_TYPE_INT16:
    *out << "NIFTI_TYPE_INT16";
    break;
  case NIFTI_TYPE_INT32:
    *out << "NIFTI_TYPE_INT32";
    break;
  case NIFTI_TYPE_FLOAT32:
    *out << "NIFTI_TYPE_FLOAT32";
    break;
  case NIFTI_TYPE_COMPLEX64:
    *out << "NIFTI_TYPE_COMPLEX64";
    break;
  case NIFTI_TYPE_FLOAT64:
    *out << "NIFTI_TYPE_FLOAT64";
    break;
  case NIFTI_TYPE_RGB24:
    *out << "NIFTI_TYPE_RGB24";
    break;
  case NIFTI_TYPE_INT8:
    *out << "NIFTI_TYPE_INT8";
    break;
  case NIFTI_TYPE_UINT16:
    *out << "NIFTI_TYPE_UINT16";
    break;
  case NIFTI_TYPE_UINT32:
    *out << "NIFTI_TYPE_UINT32";
    break;
  case NIFTI_TYPE_INT64:
    *out << "NIFTI_TYPE_INT64";
    break;
  case NIFTI_TYPE_UINT64:
    *out << "NIFTI_TYPE_UINT64";
    break;
  case NIFTI_TYPE_FLOAT128:
    *out << "NIFTI_TYPE_FLOAT12";
    break;
  case NIFTI_TYPE_COMPLEX128:
    *out << "NIFTI_TYPE_COMPLEX128";
    break;
  case NIFTI_TYPE_COMPLEX256:
    *out << "NIFTI_TYPE_COMPLEX256";
    break;
  }
}

/**
 * Outputs name of NIFTI qform code.
 *
 * @param qform_code q form code
 * @param out pointer to output stream
 */
void outputNiftiQFormCodeName(int qform_code, ostream *out) {
  switch(qform_code) {
  default:
    *out << "unknown qform_code " << qform_code;
    break;
  case NIFTI_XFORM_UNKNOWN:
    *out << "NIFTI_XFORM_UNKNOWN";
    break;
  case NIFTI_XFORM_SCANNER_ANAT:
    *out << "NIFTI_XFORM_SCANNER_ANAT";
    break;
  case NIFTI_XFORM_ALIGNED_ANAT:
    *out << "NIFTI_XFORM_ALIGNED_ANAT";
    break;
  case NIFTI_XFORM_TALAIRACH:
    *out << "NIFTI_XFORM_TALAIRACH";
    break;
  case NIFTI_XFORM_MNI_152:
    *out << "NIFTI_XFORM_MNI_152";
    break;
  }
}

imagespacecode niftiformcode2spacecode(int niftiformcode)
{
  switch(niftiformcode) {
  default:
  case NIFTI_XFORM_UNKNOWN:
    return unknown_space;
  case NIFTI_XFORM_SCANNER_ANAT:
    return scanner_anatomical_space;
  case NIFTI_XFORM_ALIGNED_ANAT:
    return aligned_anatomical_space;
  case NIFTI_XFORM_TALAIRACH:
    return talairach_space;
  case NIFTI_XFORM_MNI_152:
    return mni_152_space;
  }
}

int spacecode2niftiformcode(imagespacecode spacecode) {
  switch(spacecode) {
  default:
  case unknown_space:
    return NIFTI_XFORM_UNKNOWN;
  case scanner_anatomical_space:
    return NIFTI_XFORM_SCANNER_ANAT;
  case aligned_anatomical_space:
    return NIFTI_XFORM_ALIGNED_ANAT;
  case talairach_space:
    return NIFTI_XFORM_TALAIRACH;
  case mni_152_space:
    return NIFTI_XFORM_MNI_152;
  }
}

/**
 * Outputs name of NIFTI space units.
 *
 * @param xyzt_units combined space and time units code
 * @param out pointer to output stream
 */
void outputNiftiSpaceUnitsName(int xyzt_units, ostream *out) {
  switch(XYZT_TO_SPACE(xyzt_units)) {
  default:
    *out << "unknown space units=" << XYZT_TO_SPACE(xyzt_units);
    break;
  case NIFTI_UNITS_METER:
    *out << "NIFTI_UNITS_METER";
    break;
  case NIFTI_UNITS_MM:
    *out << "NIFTI_UNITS_MM";
    break;
  case NIFTI_UNITS_MICRON:
    *out << "NIFTI_UNITS_MICRON";
    break;
  }
}

/**
 * Outputs name of NIFTI time units.
 *
 * @param xyzt_units combined space and time units code
 * @param out pointer to output stream
 */
void outputNiftiTimeUnitsName(int xyzt_units, ostream *out) {
  switch(XYZT_TO_TIME(xyzt_units)) {
  default:
    *out << "unknown time units=" << XYZT_TO_TIME(xyzt_units);
    break;
  case NIFTI_UNITS_SEC:
    *out << "NIFTI_UNITS_SEC";
    break;
  case NIFTI_UNITS_MSEC:
    *out << "NIFTI_UNITS_MSEC";
    break;
  case NIFTI_UNITS_USEC:
    *out << "NIFTI_UNITS_USEC";
    break;
  case NIFTI_UNITS_HZ:
    *out << "NIFTI_UNITS_HZ";
    break;
  case NIFTI_UNITS_PPM:
    *out << "NIFTI_UNITS_PPM";
    break;
  case NIFTI_UNITS_RADS:
    *out << "NIFTI_UNITS_RADS";
    break;
  }
}

/**
 * Outputs name of NIFTI slice code.
 *
 * @param slice_code slice code
 * @param out pointer to output stream
 */
void outputNiftiSliceCodeName(int slice_code, ostream *out) {
  switch (slice_code) {
  default:
    *out << "invalid NIFTI slice code " << slice_code;
    break;
  case NIFTI_SLICE_UNKNOWN:
    *out << "NIFTI_SLICE_UNKNOWN";
    break;
  case NIFTI_SLICE_SEQ_INC:
    *out << "NIFTI_SLICE_SEQ_INC";
    break;
  case NIFTI_SLICE_SEQ_DEC:
    *out << "NIFTI_SLICE_SEQ_DEC";
    break;
  case NIFTI_SLICE_ALT_INC:
    *out << "NIFTI_SLICE_ALT_INC";
    break;
  case NIFTI_SLICE_ALT_DEC:
    *out << "NIFTI_SLICE_ALT_DEC";
    break;
  }
}

/**
 * Outputs name of a NIFTI intent code.
 *
 * @param niftiIntent NIFTI intent code
 * @param out pointer to output stream
 */
void outputNiftiIntentName(int niftiIntent, ostream *out) {
  switch(niftiIntent) {
  default:
    *out << "NIFTI_INTENT_UNKNOWN " << niftiIntent;
    break;
  case NIFTI_INTENT_NONE:
    *out << "NIFTI_INTENT_NONE";
    break;
  case NIFTI_INTENT_CORREL:
    *out << "NIFTI_INTENT_CORREL";
    break;
  case NIFTI_INTENT_TTEST:
    *out << "NIFTI_INTENT_TTEST";
    break;
  case NIFTI_INTENT_FTEST:
    *out << "NIFTI_INTENT_FTEST";
    break;
  case NIFTI_INTENT_ZSCORE:
    *out << "NIFTI_INTENT_ZSCORE";
    break;
  case NIFTI_INTENT_CHISQ:
    *out << "NIFTI_INTENT_CHISQ";
    break;
  case NIFTI_INTENT_BETA:
    *out << "NIFTI_INTENT_BETA";
    break;
  case NIFTI_INTENT_BINOM:
    *out << "NIFTI_INTENT_BINOM";
    break;
  case NIFTI_INTENT_GAMMA:
    *out << "NIFTI_INTENT_GAMMA";
    break;
  case NIFTI_INTENT_POISSON:
    *out << "NIFTI_INTENT_POISSON";
    break;
  case NIFTI_INTENT_NORMAL:
    *out << "NIFTI_INTENT_NORMAL";
    break;
  case NIFTI_INTENT_FTEST_NONC:
    *out << "NIFTI_INTENT_FTEST_NONC";
    break;
  case NIFTI_INTENT_CHISQ_NONC:
    *out << "NIFTI_INTENT_CHISQ_NONC";
    break;
  case NIFTI_INTENT_LOGISTIC:
    *out << "NIFTI_INTENT_LOGISTIC";
    break;
  case NIFTI_INTENT_LAPLACE:
    *out << "NIFTI_INTENT_LAPLACE";
    break;
  case NIFTI_INTENT_UNIFORM:
    *out << "NIFTI_INTENT_UNIFORM";
    break;
  case NIFTI_INTENT_TTEST_NONC:
    *out << "NIFTI_INTENT_TTEST_NONC";
    break;
  case NIFTI_INTENT_WEIBULL:
    *out << "NIFTI_INTENT_WEIBULL";
    break;
  case NIFTI_INTENT_CHI:
    *out << "NIFTI_INTENT_CHI";
    break;
  case NIFTI_INTENT_INVGAUSS:
    *out << "NIFTI_INTENT_INVGAUSS";
    break;
  case NIFTI_INTENT_EXTVAL:
    *out << "NIFTI_INTENT_EXTVAL";
    break;
  case NIFTI_INTENT_PVAL:
    *out << "NIFTI_INTENT_PVAL";
    break;
  case NIFTI_INTENT_LOGPVAL:
    *out << "NIFTI_INTENT_LOGPVAL";
    break;
  case NIFTI_INTENT_LOG10PVAL:
    *out << "NIFTI_INTENT_LOG10PVAL";
    break;
  case NIFTI_INTENT_ESTIMATE:
    *out << "NIFTI_INTENT_ESTIMATE";
    break;
  case NIFTI_INTENT_LABEL:
    *out << "NIFTI_INTENT_LABEL";
    break;
  case NIFTI_INTENT_NEURONAME:
    *out << "NIFTI_INTENT_NEURONAME";
    break;
  case NIFTI_INTENT_GENMATRIX:
    *out << "NIFTI_INTENT_GENMATRIX";
    break;
  case NIFTI_INTENT_SYMMATRIX:
    *out << "NIFTI_INTENT_SYMMATRIX";
    break;
  case NIFTI_INTENT_DISPVECT:
    *out << "NIFTI_INTENT_DISPVECT";
    break;
  case NIFTI_INTENT_VECTOR:
    *out << "NIFTI_INTENT_VECTOR";
    break;
  case NIFTI_INTENT_POINTSET:
    *out << "NIFTI_INTENT_POINTSET";
    break;
  case NIFTI_INTENT_TRIANGLE:
    *out << "NIFTI_INTENT_TRIANGLE";
    break;
  case NIFTI_INTENT_QUATERNION:
    *out << "NIFTI_INTENT_QUATERNION";
    break;
  case NIFTI_INTENT_DIMLESS:
    *out << "NIFTI_INTENT_DIMLESS";
    break;
  case NIFTI_INTENT_TIME_SERIES:
    *out << "NIFTI_INTENT_TIME_SERIES";
    break;
  case NIFTI_INTENT_NODE_INDEX:
    *out << "NIFTI_INTENT_NODE_INDEX";
    break;
  case NIFTI_INTENT_RGB_VECTOR:
    *out << "NIFTI_INTENT_RGB_VECTOR";
    break;
  case NIFTI_INTENT_RGBA_VECTOR:
    *out << "NIFTI_INTENT_RGBA_VECTOR";
    break;
  case NIFTI_INTENT_SHAPE:
    *out << "NIFTI_INTENT_SHAPE";
    break;
  }
}

cnuniftiform bldniftifilenames(string primaryname, string *imgname,
			       string *hdrname, string *combinedname)
{
  cnuniftiform localcnuniftiform = niiform;
  if(primaryname.empty()) {
    cerr<<"bldniftifilenames - empty file name\n";
    exit(1);
  }
  size_t namelength=primaryname.length();
// remove .img, .hdr, or .nii from end of name
  if(namelength > 5) {
    string ending = primaryname.substr(namelength-4, 4);
    if(ending.compare(".img") == 0) {
      primaryname.erase(namelength-4);
      localcnuniftiform = ni1form;
    }
    else if(ending.compare(".hdr") == 0) {
      primaryname.erase(namelength-4);
      localcnuniftiform = ni1form;
    }
    else if(ending.compare(".nii") == 0) {
      primaryname.erase(namelength-4);
      localcnuniftiform = niiform;
    }
  }
// return strings
  if(imgname != NULL) *imgname=primaryname + ".img";
  if(combinedname != NULL) *combinedname=primaryname + ".nii";
  if(hdrname != NULL) *hdrname=primaryname + ".hdr";
// return form
  return localcnuniftiform;
}

int validniftihdr(nifti_1_header *rawhdr)
{
  return((rawhdr->sizeof_hdr == 348)
	 && ((strcmp(rawhdr->magic, "ni1\0") == 0)
	     || (strcmp(rawhdr->magic, "n+1\0") == 0))
	 );
//	 && (rawhdr->hk.regular == 'r'));
//	 (rawhdr->hk.extents == 16384) &&
}

nifti_1_header *readNiftiHeader(string primaryname, nifti_1_header *rawhdr,
				fstream **fdptrptr,
				psyswaptype *swaptypeptr)
{
  // these moved to here because I had problem on Solaris 5.5.1 of
  // arrays being all 0 causing a core dump
  static int header_soff[] = {
    36,
    40, 42, 44, 46, 48, 50, 52, 54,
    68, 70, 72, 74,
    120,
    252, 254,
    -1 };
  static int header_ioff[] = {
    0, 32,
    140, 144,
    -1 };
  static int header_foff[] = {
    56, 60, 64,
    76, 80, 84, 88, 92, 96, 100, 104,
    108, 112, 116,
    124, 128, 132, 136,
    256, 260, 264, 268, 272, 276,
    280, 284, 288, 292,
    296, 300, 304, 308,
    312, 316, 320, 324,
    -1 };

  fstream *fd = NULL;
  nifti_1_header *localrawhdr = NULL;
  psyswaptype localswaptype = psynoswap;

  if(fdptrptr != NULL) fd = *fdptrptr;
  if(primaryname.empty() && (fd == NULL)) return NULL;

  if(fd == NULL) {
    string hdrname;
    string combinedname;
    const char *hdr1;
    const char *hdr2;
    switch(bldniftifilenames(primaryname, NULL, &hdrname, &combinedname)) {
    default:
    case niiform:
      hdr1=combinedname.c_str();
      hdr2=hdrname.c_str();
      break;
    case ni1form:
      hdr1=hdrname.c_str();
      hdr2=combinedname.c_str();
      break;
    }
    fd = new fstream(hdr1, ios::in);
    if(fd->fail() || fd->bad()) {
      delete fd;
      fd = new fstream(hdr2, ios::in);
      if(fd->fail() || fd->bad()) {
	delete fd;
	fd = NULL;
      }
    }
    if(fdptrptr != NULL) *fdptrptr = fd;
  }
  else fd->seekg(0);  // beginning of file

  if(fd != NULL) {
    localrawhdr = rawhdr;
    if(localrawhdr == NULL) localrawhdr = new nifti_1_header();
    char *ptr, *endptr;
    for(ptr=(char*)localrawhdr, endptr=ptr+sizeof(nifti_1_header); ptr<endptr; ptr++) *ptr=0;

    fd->read((char *)localrawhdr, 348);
    if(!fd->good()) {
      if(rawhdr == NULL) delete localrawhdr;
      localrawhdr = NULL;
    }
    else if(!validniftihdr(localrawhdr)) {
    // try swapping things
      swap_data((char *) localrawhdr, header_soff, header_ioff, header_foff);
      if(validniftihdr(localrawhdr)) localswaptype = psyreversewordbytes;
      else {
	if(rawhdr == NULL) delete localrawhdr;
	localrawhdr = NULL;
      }
    }
    if(fdptrptr == NULL) {
      fd->close();
      delete fd;
    }
  }

  if(swaptypeptr != NULL && localrawhdr != NULL) *swaptypeptr = localswaptype;
  return localrawhdr;
}

int writeNiftiHeader(nifti_1_header *rawhdr, string primaryname, fstream **fdptrptr) {
  if(rawhdr == NULL) return -1;

  if(rawhdr->sizeof_hdr != 348) cerr<<"warning -- writing invalid nifti header -- sizeof_hdr="<< rawhdr->sizeof_hdr<< " should be 348\n";
  if((strcmp(rawhdr->magic, "ni1\0") != 0)
    && (strcmp(rawhdr->magic, "n+1\0") != 0)) {
    cerr<<"warning -- writing invalid nifti header -- magic=\""<< rawhdr->magic<< "\" should be \"ni1\" or \"n+1\"\n";
  }

  fstream *fd = NULL;
  if(fdptrptr != NULL) fd = *fdptrptr;
  if(primaryname.empty() && (fd == NULL)) return -1;
  if(fd == NULL) {
// build complete file names
    string hdrname;
    bldniftifilenames(primaryname, NULL, &hdrname, NULL);
// open header file
    fd = new fstream(hdrname.c_str(), ios::out);
    if(fd->fail() || fd->bad()) {
      delete fd;
      cerr<<"writeNiftiHeader: error openning header file "<<hdrname;
      return -1;
    }
    if(fdptrptr != NULL) *fdptrptr = fd;
  }
  else fd->seekp(0); // beginning of file - caused problems under a version of cygwin

// write header to file
  fd->write((char *)rawhdr, sizeof(nifti_1_header));
  if(fd->bad()) {
    cerr<<"writeNiftiHeader: error writing to header file";
    return -1;
  }
  if(fdptrptr == NULL) {
// remove file stream
    fd->close();
    delete fd;
  }

  return 0;
}

int isniftifile(string filename)
{
  nifti_1_header *rawhdr = readNiftiHeader(filename);
  if(rawhdr == NULL) return 0;
  else {
    delete rawhdr;
    return 1;
  }
}

niftifile::niftifile(string fname, psyimg *psyimgptr,
		     psytype pixeltype, nifti_1_header *inheader)
     : rawfile(psyimgptr, pixeltype)
{
// open files in write mode since linked
  openniftifiles(fname, "w");

  if(inheader != NULL) {
    // initialize nifti header to inheader
    char *ptr, *endptr, *iptr;
    for(ptr=(char*)&rawhdr, endptr=ptr+sizeof(rawhdr), iptr = (char*)inheader;
	ptr<endptr; ptr++, iptr++) *ptr = *iptr;
  }
  else buildniftiheader(); // initialize nifti header based on this image
// write out image
  writetofiles();
}

niftifile::niftifile(string fname, const char *mode, int ignore_valid)
{
  char *ptr, *endptr;

  if(mode == NULL)mode="r";
// open files
  openniftifiles(fname, mode);


// initialize nifti header to zeros
  for(ptr=(char*)&rawhdr, endptr=ptr+sizeof(rawhdr);
      ptr<endptr; ptr++)*ptr=0;

// read header
  if(strcmp(mode, "r") == 0) {
    fstream *fd = hdrfdptr;
    nifti_1_header *tmphdr = readNiftiHeader("", &rawhdr, &fd, &swaptype);
    if(tmphdr == NULL) {
      cerr << "niftifile::niftifile:  error reading hdr file\n";
      exit(1);
    }
// word res
    double wres=rawhdr.scl_slope;
    if(fabs(wres)<1e-16) wres=1.0;  // default word res
// i res is time
    double ires=rawhdr.pixdim[4];
    switch(XYZT_TO_TIME(rawhdr.xyzt_units)) {
    default:
    case NIFTI_UNITS_SEC:
      break;
    case NIFTI_UNITS_MSEC:
      ires *= 1e-3;
      break;
    case NIFTI_UNITS_USEC:
      ires *= 1e-6;
      break;
    }
// unit factor
    double spaceunitfactor = 1.0;
    switch (XYZT_TO_SPACE(rawhdr.xyzt_units)) {
    default:
    case NIFTI_UNITS_METER:
      break;
    case NIFTI_UNITS_MM:
      spaceunitfactor=1e-3;
      break;
    case NIFTI_UNITS_MICRON:
      spaceunitfactor=1e-6;
      break;
    }
// initialize psyimg to values from nifti header
    initpsyimg(rawhdr.dim[1], rawhdr.dim[2], rawhdr.dim[3], rawhdr.dim[4],
	       nifti2psytype(rawhdr.datatype, rawhdr.bitpix),
	       0,0,0,0,(int)rawhdr.vox_offset,
	       rawhdr.pixdim[1]*spaceunitfactor,rawhdr.pixdim[2]*spaceunitfactor,
	       rawhdr.pixdim[3]*spaceunitfactor,ires,wres);
// also initialize patientid, date, time and description
// note - if a string is not null terminated no problem except added garbage
    setpatientid("");
    //setdate(rawhdr.hist.exp_date);
    //settime(rawhdr.hist.exp_time);
    setdescription(rawhdr.descrip);
    //    setorient(analyze2psyorient(rawhdr.orient));
    if(getbytesperpixel() == 1) swaptype = psynoswap;
    if(rawhdr.sform_code > 0.0) {
      xyzidouble srow1;
      srow1.x=rawhdr.srow_x[0]; srow1.y=rawhdr.srow_x[1]; srow1.z=rawhdr.srow_x[2]; srow1.i=rawhdr.srow_x[3];
      xyzidouble srow2;
      srow2.x=rawhdr.srow_y[0]; srow2.y=rawhdr.srow_y[1]; srow2.z=rawhdr.srow_y[2]; srow2.i=rawhdr.srow_y[3];
      xyzidouble srow3;
      srow3.x=rawhdr.srow_z[0]; srow3.y=rawhdr.srow_z[1]; srow3.z=rawhdr.srow_z[2]; srow3.i=rawhdr.srow_z[3];
      threeDtransform *ts = new threeDtransform(srow1, srow2, srow3);
      setspatialtransform(ts, niftiformcode2spacecode(rawhdr.sform_code));
    }
    if(rawhdr.qform_code > 0.0) {
      cnuquatern q;
      q.b = rawhdr.quatern_b;
      q.c = rawhdr.quatern_c;
      q.d = rawhdr.quatern_d;
      q.qfac = rawhdr.pixdim[0];
      q.xoff = rawhdr.qoffset_x;
      q.yoff = rawhdr.qoffset_y;
      q.zoff = rawhdr.qoffset_z;
      q.xfactor = rawhdr.pixdim[1];
      q.yfactor = rawhdr.pixdim[2];
      q.zfactor = rawhdr.pixdim[3];
      threeDtransform *tq = new threeDtransform(q);
      setspatialtransform2(tq, niftiformcode2spacecode(rawhdr.qform_code));
    }
  }
}

niftifile::~niftifile()
{
  if(hdrfdptr != NULL) {
    if(*hdrfdptr != imgfd) {
      hdrfdptr->close();
      delete hdrfdptr;
    }
  }
}

void niftifile::openniftifiles(string fname, const char *mode)
{
  string hdr; string img; string cmb;
  int status;
// build complete file names
  niftiform = bldniftifilenames(fname, &img, &hdr, &cmb);
  switch(niftiform) {
  default:
  case niiform:
    hdrfilename = cmb; imgfilename = cmb;
    hdrfdptr= &imgfd;
    break;
  case ni1form:
    hdrfilename=hdr; imgfilename=img;
    hdrfdptr= new fstream();
    break;
  }
  // try first header possibility
  psyopenfile(hdrfilename, mode, hdrfdptr, &status);
  if(status != 0) {
    if(niftiform == ni1form) {
      niftiform = niiform;
      hdrfilename=cmb; imgfilename=cmb;
      delete hdrfdptr;
      hdrfdptr= &imgfd;
    }
    else {
      niftiform = ni1form;
      hdrfilename=hdr; imgfilename=img;
      hdrfdptr= new fstream();
      imgfd.clear(); // unset bad flag from trying to open combined file
    }
    // try second header possibility
    psyopenfile(hdrfilename, mode, hdrfdptr); // bad status will abort
  }
  if(hdrfilename.compare(cmb) != 0) {
    // open image file - not same file as header
    psyopenfile(imgfilename, mode, &imgfd); // bad status will abort
  }
}

void niftifile::showhdr()
{
  showhdr(&rawhdr);
}

void niftifile::showhdr(nifti_1_header rawhdr) {
  showhdr(&rawhdr);
}


void niftifile::showhdr(nifti_1_header *rawhdr, ostream *out) {
  int i;
// print out header_key
  *out << "nifti header:\n";
  *out << "sizeof_hdr=" << rawhdr->sizeof_hdr << '\n';
  *out << "data_type=\"" << rawhdr->data_type << "\"\n";
  *out << "db_name=\"" << rawhdr->db_name << "\"\n";
  *out << "extents=" << rawhdr->extents << '\n';
  *out << "session_error=" << rawhdr->session_error << '\n';
  *out << "regular=" << rawhdr->regular << '\n';
  *out << "dim_info=" << (int) rawhdr->dim_info << '\n';
  *out << "#  freq_dim=" << DIM_INFO_TO_FREQ_DIM(rawhdr->dim_info) << '\n';
  *out << "#  phase_dim=" << DIM_INFO_TO_PHASE_DIM(rawhdr->dim_info) << '\n';
  *out << "#  slice_dim=" << DIM_INFO_TO_SLICE_DIM(rawhdr->dim_info) << '\n';
// print out image_dimension
  for(i=0; i<8; i++)
    *out << "dim[" << i << "]=" << rawhdr->dim[i] << '\n';
  *out << "intent_p1=" << rawhdr->intent_p1 << '\n';
  *out << "intent_p2=" << rawhdr->intent_p2 << '\n';
  *out << "intent_p3=" << rawhdr->intent_p3 << '\n';
  *out << "intent_code=" << rawhdr->intent_code << '\n';
  *out << "#  "; outputNiftiIntentName(rawhdr->intent_code, out); *out <<'\n';
  *out << "datatype=" << rawhdr->datatype << '\n';
  *out << "#  "; outputNiftiDataTypeName(rawhdr->datatype, out); *out <<'\n';
  *out << "bitpix=" << rawhdr->bitpix << '\n';
  *out << "slice_start=" << rawhdr->slice_start << '\n';
  for(i=0; i<8; i++)
    *out << "pixdim[" << i << "]=" << rawhdr->pixdim[i] << '\n';
  *out << "#  qfac=" << ((rawhdr->pixdim[0] >= -0.5) ? 1.0 : -1.0) << '\n';
  *out << "vox_offset=" << rawhdr->vox_offset << '\n';
  *out << "scl_slope=" << rawhdr->scl_slope << '\n';
  *out << "scl_inter=" << rawhdr->scl_inter << '\n';
  *out << "slice_end=" << rawhdr->slice_end << '\n';
  *out << "slice_code=" << (int) rawhdr->slice_code << '\n';
  *out << "#  "; outputNiftiSliceCodeName(rawhdr->slice_code, out); *out <<'\n';
  *out << "xyzt_units=" << (int) rawhdr->xyzt_units << '\n';
  *out << "#  "; outputNiftiSpaceUnitsName(rawhdr->xyzt_units, out); *out <<'\n';
  *out << "#  "; outputNiftiTimeUnitsName(rawhdr->xyzt_units, out); *out <<'\n';
  
  *out << "cal_max=" << rawhdr->cal_max << '\n';
  *out << "cal_min=" << rawhdr->cal_min << '\n';
  *out << "slice_duration=" << rawhdr->slice_duration << '\n';
  *out << "toffset=" << rawhdr->toffset << '\n';
  *out << "glmax=" << rawhdr->glmax << '\n';
  *out << "glmin=" << rawhdr->glmin << '\n';
// print out data_history
  *out << "descrip=\"" << rawhdr->descrip << "\"\n";
  *out << "aux_file=\"" << rawhdr->aux_file << "\"\n";
  *out << "qform_code=" << (int)rawhdr->qform_code << '\n';
  *out << "#  "; outputNiftiQFormCodeName(rawhdr->qform_code, out); *out <<'\n';
  *out << "sform_code=" << rawhdr->sform_code << '\n';
  *out << "#  "; outputNiftiQFormCodeName(rawhdr->sform_code, out); *out <<'\n';
  *out << "quatern_b=" << rawhdr->quatern_b << '\n';
  *out << "quatern_c=" << rawhdr->quatern_c << '\n';
  *out << "quatern_d=" << rawhdr->quatern_d << '\n';
  *out << "qoffset_x=" << rawhdr->qoffset_x << '\n';
  *out << "qoffset_y=" << rawhdr->qoffset_y << '\n';
  *out << "qoffset_z=" << rawhdr->qoffset_z << '\n';


  for(i=0; i<4; i++) *out << "srow_x[" << i << "]=" << rawhdr->srow_x[i] << '\n';
  for(i=0; i<4; i++) *out << "srow_y[" << i << "]=" << rawhdr->srow_y[i] << '\n';
  for(i=0; i<4; i++) *out << "srow_z[" << i << "]=" << rawhdr->srow_z[i] << '\n';

  *out << "intent_name=\"" << rawhdr->intent_name << "\"\n";
  *out << "magic=\"" << rawhdr->magic << "\"\n";
}

void niftifile::buildniftiheader(nifti_1_header *niftihdrptr) {
  if(niftihdrptr == NULL) niftihdrptr= &rawhdr;
  // initialize nifti header to zeros
  char *ptr, *endptr;
  for(ptr=(char*)niftihdrptr, endptr=ptr+sizeof(nifti_1_header);
      ptr<endptr; ptr++)*ptr=0;
  // initialize nifti header based on this image
  niftihdrptr->sizeof_hdr = 348;
  for(int i=0; i<10; i++) niftihdrptr->data_type[i]='\0';
  for(int i=0; i<18; i++) niftihdrptr->db_name[i]='\0';
  niftihdrptr->extents=16384;
  niftihdrptr->regular='r';
  int freq_dim = 0;  int phase_dim = 0;  int slice_dim = 3;
  niftihdrptr->dim_info=FPS_INTO_DIM_INFO(freq_dim, phase_dim, slice_dim);
  if(size.i > 1) niftihdrptr->dim[0]=4;
  else niftihdrptr->dim[0]=3;
  niftihdrptr->dim[1]=size.x;
  niftihdrptr->dim[2]=size.y;
  niftihdrptr->dim[3]=size.z;
  niftihdrptr->dim[4]=size.i;
  niftihdrptr->dim[5]=1;
  niftihdrptr->dim[6]=1;
  niftihdrptr->dim[7]=1;
  if((niftihdrptr->dim[1] != size.x) || (niftihdrptr->dim[2] != size.y) ||
     (niftihdrptr->dim[3] != size.z) || (niftihdrptr->dim[4] != size.i) ) {
    output_tree(&cerr);
    cerr<<"::writetofiles: aborting - unable to store one or more dimensions as short - ";
    cerr<< "size=(" << size.x << "," << size.y << "," << size.z << ","
    << size.i << ")\n";;
    exit(1);
  }
  niftihdrptr->intent_p1=0.0;
  niftihdrptr->intent_p2=0.0;
  niftihdrptr->intent_p3=0.0;
  niftihdrptr->intent_code=0;
  niftihdrptr->datatype=psytype2nifti(type);
                          //0=unknown,1=binary,2=uchar,4=short,
	                  //8=int,16=float,32=complex,64=double
  niftihdrptr->bitpix=getbytesperpixel()*8;
  niftihdrptr->slice_start=0;

  //?? word resolution not really defined by analyze
  // store wordres in cnu location
  niftihdrptr->pixdim[0]=getwordres();
  // store wordres in spm location also
  //  niftihdrptr->dime.funused1=getwordres();
  niftihdrptr->pixdim[1]=res.x*1000; // real world voxel width in mm
  niftihdrptr->pixdim[2]=res.y*1000;
  niftihdrptr->pixdim[3]=res.z*1000; // slice thickness in mm
  niftihdrptr->pixdim[4]=res.i; //?? interval between images not really defined
  niftihdrptr->pixdim[5]=0.0;
  niftihdrptr->pixdim[6]=0.0;
  niftihdrptr->pixdim[7]=0.0;
  if(hdrfdptr == &imgfd) niftihdrptr->vox_offset=sizeof(nifti_1_header);
  else niftihdrptr->vox_offset=0;
  niftihdrptr->scl_slope=getwordres();
  niftihdrptr->scl_inter=0.0;
  niftihdrptr->slice_end=niftihdrptr->dim[slice_dim] - 1;
  niftihdrptr->slice_code=0;
  niftihdrptr->xyzt_units=SPACE_TIME_TO_XYZT(NIFTI_UNITS_MM, NIFTI_UNITS_SEC);
  double max, min;
  inputpsyimg->getstats(&min, &max, NULL, NULL, NULL);
  niftihdrptr->cal_max=max; //max value for file
  niftihdrptr->cal_min=min; //min value for file
  niftihdrptr->slice_duration=niftihdrptr->pixdim[slice_dim];
  niftihdrptr->toffset=0.0;
  niftihdrptr->glmax=irint(max);
  niftihdrptr->glmin=irint(min);
  for(int i=0; i<80; i++) niftihdrptr->descrip[i]='\0';
  strncpy(niftihdrptr->descrip, getdescription().c_str(), 80);
  niftihdrptr->descrip[79]='\0';
  for(int i=0; i<24; i++) niftihdrptr->aux_file[i]='\0';
  niftihdrptr->qform_code=0;
  niftihdrptr->sform_code=0;
  niftihdrptr->quatern_b=0;
  niftihdrptr->quatern_c=0;
  niftihdrptr->quatern_d=0;
  niftihdrptr->qoffset_x=0;
  niftihdrptr->qoffset_y=0;
  niftihdrptr->qoffset_z=0;
  for(int i=0; i<4; i++) {
    niftihdrptr->srow_x[i]=0;
    niftihdrptr->srow_y[i]=0;
    niftihdrptr->srow_z[i]=0;
  }
  threeDtransform *transform = getspatialtransform();
  if(transform != NULL) {
    xyzidouble row = transform->getRow1();
    niftihdrptr->srow_x[0] = row.x; niftihdrptr->srow_x[1] = row.y; niftihdrptr->srow_x[2] = row.z; niftihdrptr->srow_x[3] = row.i;
    row = transform->getRow2();
    niftihdrptr->srow_y[0] = row.x; niftihdrptr->srow_y[1] = row.y; niftihdrptr->srow_y[2] = row.z; niftihdrptr->srow_y[3] = row.i;
    row = transform->getRow3();
    niftihdrptr->srow_z[0] = row.x; niftihdrptr->srow_z[1] = row.y; niftihdrptr->srow_z[2] = row.z; niftihdrptr->srow_z[3] = row.i;
    niftihdrptr->sform_code=spacecode2niftiformcode(getspatialtransformcode());
  }
  transform = getspatialtransform2();
  if(transform != NULL) {
    cnuquatern *quatern = transform->getQuatern();
    if(quatern != NULL) {
       niftihdrptr->qform_code=spacecode2niftiformcode(getspatialtransformcode2());
       niftihdrptr->quatern_b=quatern->b;
       niftihdrptr->quatern_c=quatern->c;
       niftihdrptr->quatern_d=quatern->d;
       niftihdrptr->qoffset_x=quatern->xoff;
       niftihdrptr->qoffset_y=quatern->yoff;
       niftihdrptr->qoffset_z=quatern->zoff;
       niftihdrptr->pixdim[0] = quatern->qfac;
       delete quatern;
    }
  }

  for(int i=0; i<16; i++) niftihdrptr->intent_name[i]='\0';
  niftihdrptr->magic[0]='n';
  if(niftiform == ni1form) niftihdrptr->magic[1]='i';
  else niftihdrptr->magic[1]='+';
  niftihdrptr->magic[2]='1';
  niftihdrptr->magic[3]='\0';
  for(int i=0; i<4; i++) niftihdrptr->extender[i]='\0';
}

nifti_1_header *niftifile::getniftiheader(nifti_1_header *niftihdrptr) {
  if(niftihdrptr == NULL) niftihdrptr = new nifti_1_header();
  char *ptr, *endptr, *optr;
  for(ptr=(char*)&rawhdr, endptr=ptr+sizeof(rawhdr), optr = (char*)niftihdrptr;
      ptr<endptr; ptr++, optr++) *optr = *ptr;
  return niftihdrptr;
}

void niftifile::writetofiles()
{
// write header to file
  if(writeNiftiHeader(&rawhdr, "", &hdrfdptr) != 0) {
    cerr<<"niftifiles::writetofiles: error writing to file: "
      <<hdrfilename<<'\n';
    exit(1);
  }
  //imgfd.seekp(rawhdr.vox_offset, ios::beg);
  writedata();
}

