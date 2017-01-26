#include "psyhdr.h"
#include "psyanalyze.h"
// need ecat swap routines
#include "psyecat.h"

psytype getanalyzeclosesttype(psytype type) {
  switch (type) {
  case psyuchar:
    return psyuchar;
  case psychar:
  case psyshortsw:
  case psyshort:
    return psyshort;
  case psyushort:
  case psyuint:
  case psyint:
    return psyint;
  case psyfloat:
    return psyfloat;
  case psydouble:
    return psydouble;
  case psycomplex:
  case psydate:
  case psystring:
  case psydicomdataelement:
  case psyrgb:
  case psyargb:
  case psynotype:
  default:
    return psynotype;
  }
}

int psytype2analyze(psytype pixeltype)
{
//0=unknown,1=binary,2=uchar,4=short,
//8=int,16=float,32=complex,64=double

  switch(pixeltype) {
  case psyuchar:
    return(DT_UNSIGNED_CHAR);
  case psyshort:
    return(DT_SIGNED_SHORT);
  case psyint:
    return(DT_SIGNED_INT);
  case psyfloat:
    return(DT_FLOAT);
  case psydouble:
    return(DT_DOUBLE);
  default:
    break;
  }
  cerr<<"psytype2analyze - don't know equivalent type\n";
  exit(1);
}

psytype analyze2psytype(int analyzetype, int pixelbits)
{
//0=unknown,1=binary,2=uchar,4=short,
//8=int,16=float,32=complex,64=double
  switch(analyzetype) {
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
      cerr<<"analyze2psytype - don't know equivalent type\n";
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
  }
  cerr<<"analyze2psytype - don't know equivalent type\n";
  exit(1);
}

int psyorient2analyze(psyorient orientation)
{
//0=transverse,1=coronal,2=sagittal,
//3=transverse flipped,4=coronal flipped,5=sagittal flipped
  switch(orientation) {
  case psynoorient:
  case psytransverse:
    return(0);
  case psycoronal:
    return(1);
  case psysagittal:
    return(2);
  }
  cerr<<"psyorient2analyze - don't know equivalent orientation\n";
  exit(1);
}

psyorient analyze2psyorient(int analyzeorient)
{
//0=transverse,1=coronal,2=sagittal,
//3=transverse flipped,4=coronal flipped,5=sagittal flipped
  switch(analyzeorient) {
  case 0:
    return(psytransverse);
  case 1:
    return(psycoronal);
  case 2:
    return(psysagittal);
  }
  cerr<<"analyze2psyorient - don't know equivalent orientation\n";
  cerr<<"analyze2psyorient - input analyze orientation="<<analyzeorient;
  cerr<<" assuming psytransverse\n";
  return(psytransverse);
}

void bldanalyzefilenames(string primaryname, string *imgname,
			 string *hdrname)
{
  size_t namelength=primaryname.length();
  if(namelength == 0) {
    cerr<<"bldanalyzefilenames - empty file name\n";
    exit(1);
  }
  if(namelength > 5) {
    string ending = primaryname.substr(namelength-4, 4);
    if(ending.compare(".img") == 0 ||
       ending.compare(".hdr") == 0) {
      primaryname.erase(namelength-4);
    }
  }
// return strings
  if(hdrname != NULL)*hdrname=primaryname + ".hdr";
  if(imgname != NULL)*imgname=primaryname + ".img";
}

int validanalyzehdr(dsr *rawhdr)
{
  return((rawhdr->hk.sizeof_hdr == sizeof(*rawhdr)));
//	 && (rawhdr->hk.regular == 'r'));
//	 (rawhdr->hk.extents == 16384) &&
}

dsr *readAnalyzeHeader(string name, dsr *rawhdr, fstream **fdptrptr,
		       psyswaptype *swaptypeptr)
{
  // these moved to here because I had problem on Solaris 5.5.1 of
  // arrays being all 0 causing a core dump
  static int hk_off = 0;
  static int dime_off = 40;
  static int hist_off = 148;
  static int header_soff[] = {
    hk_off+36,
    dime_off+0, dime_off+2, dime_off+4, dime_off+6, dime_off+8, dime_off+10,
    dime_off+12, dime_off+14, dime_off+24, dime_off+30, dime_off+32,
    dime_off+34,
    -1 };
  static int header_ioff[] = {
    hk_off+0, hk_off+32,
    dime_off+92, dime_off+96, dime_off+100, dime_off+104,
    -1 };
  static int header_foff[] = {
    dime_off+36, dime_off+40, dime_off+44, dime_off+48, dime_off+52,
    dime_off+56, dime_off+60, dime_off+64, dime_off+68, dime_off+72,
    dime_off+76, dime_off+80, dime_off+84, dime_off+88,
    hist_off+168, hist_off+172, hist_off+176, hist_off+180, hist_off+184,
    hist_off+188, hist_off+192, hist_off+196,
    -1 };

  fstream *fd = NULL;
  psyswaptype localswaptype = psynoswap;
  if(fdptrptr != NULL) fd = *fdptrptr;
  if((name.length() == 0) && (fd == NULL)) return NULL;

  if(fd == NULL) {
    string hdrname;
    bldanalyzefilenames(name, NULL, &hdrname);
    fd = new fstream(hdrname.c_str(), ios::in);
    if(fd->fail() || fd->bad()) {
      delete fd;
      return NULL;
    }
    if(fdptrptr != NULL) *fdptrptr = fd;
  }
  else fd->seekg(0);  // beginning of file

  dsr *localrawhdr = rawhdr;
  if(localrawhdr == NULL) localrawhdr = new dsr();

  fd->read((char *)localrawhdr, sizeof(*localrawhdr));
  if(!fd->good()) {
    if(rawhdr == NULL) delete localrawhdr;
    localrawhdr = NULL;
  }
  else if(!validanalyzehdr(localrawhdr)) {
    // try swapping things
    swap_data((char *) localrawhdr, header_soff, header_ioff, header_foff);
    if(validanalyzehdr(localrawhdr)) localswaptype = psyreversewordbytes;
    else {
      if(rawhdr == NULL) delete localrawhdr;
      localrawhdr = NULL;
    }
  }
  if(fdptrptr == NULL) {
    fd->close();
    delete fd;
  }
  if(swaptypeptr != NULL) *swaptypeptr = localswaptype;

  return localrawhdr;
}

int writeAnalyzeHeader(dsr *rawhdr, string name, fstream *fdptr) {

  if(rawhdr == NULL) return -1;
  fstream *localfdptr = NULL;

  if(fdptr == NULL) {
    if(name.length() == 0) return -1;
    fdptr = localfdptr = new fstream();
  }

  if(! fdptr->is_open()) {
    if(name.length() == 0) return -1;
// build complete file names
    string hdrname;
    bldanalyzefilenames(name, NULL, &hdrname);
// open header file
    fdptr->open(hdrname.c_str(), ios::out);
    if(fdptr->fail() || fdptr->bad()) {
      delete localfdptr; // remove file stream if created locally
      cerr<<"writeAnalyzeHeader: error openning header file";
      return -1;
    }
  }
  else fdptr->seekp(0); // beginning of file - caused problems under a version of cygwin

// write header to file
  fdptr->write((char *)rawhdr, sizeof(*rawhdr));
  if(fdptr->bad()) {
    cerr<<"writeAnalyzeHeader: error writing to header file";
    delete localfdptr; // remove file stream if created locally
    return -1;
  }
// remove file stream if created locally
  if(localfdptr != NULL) {
    localfdptr->close();
    delete localfdptr;
  }
  return 0;
}

int isanalyzefile(string name)
{
  dsr *rawhdr = readAnalyzeHeader(name);
  if(rawhdr == NULL) return 0;
  else {
    delete rawhdr;
    return 1;
  }
}

analyzefile::analyzefile(string fname, psyimg *psyimgptr,
			 psytype pixeltype)
     : rawfile(psyimgptr, pixeltype)
{
  char *ptr, *endptr;
// open files in write mode since linked
  openanalyzefiles(fname, "w");

// initialize analyze header to zeros
  for(ptr=(char*)&rawhdr, endptr=ptr+sizeof(rawhdr);
      ptr<endptr; ptr++)*ptr=0;
// write out image
  writetofiles();
}

analyzefile::analyzefile(string fname, const char *mode, int ignore_valid)
{
  char *ptr, *endptr;

  if(mode == NULL)mode="r";
// open files
  openanalyzefiles(fname, mode);

// initialize analyze header to zeros
  for(ptr=(char*)&rawhdr, endptr=ptr+sizeof(rawhdr);
      ptr<endptr; ptr++)*ptr=0;

// read header
  if(strcmp(mode, "r") == 0) {
    fstream *fd = &hdrfd;
    dsr *tmphdr = readAnalyzeHeader("", &rawhdr, &fd, &swaptype);
    if(tmphdr == NULL) {
      cerr << "analyzefile::analyzefile:  error reading hdr file\n";
      exit(1);
    }
// note - pixdims 1-3 are valid but 4 and 0 not really defined by analyze
// word res may not be set properly in header
    double wres=rawhdr.dime.pixdim[0];  // try cnu location for word res
    if(fabs(wres)<1e-16) {
      wres=rawhdr.dime.funused1;  // try spm location for word res
      if(fabs(wres)<1e-16) wres=1.0;  // default word res
    }
// i res may not be set properly in header
    double ires=(fabs(rawhdr.dime.pixdim[4])<1e-16)? 1.0: rawhdr.dime.pixdim[4];
// initialize psyimg to values from analyze header
    initpsyimg(rawhdr.dime.dim[1], rawhdr.dime.dim[2], rawhdr.dime.dim[3],
		rawhdr.dime.dim[4],
	       analyze2psytype(rawhdr.dime.datatype, rawhdr.dime.bitpix),
	       0,0,0,0,(int)rawhdr.dime.vox_offset,
	       rawhdr.dime.pixdim[1]/1000,rawhdr.dime.pixdim[2]/1000,
	       rawhdr.dime.pixdim[3]/1000,ires,wres);
// also initialize patientid, date, time and description
// note - if a string is not null terminated no problem except added garbage
    setpatientid(rawhdr.hist.patient_id);
    setdate(rawhdr.hist.exp_date);
    settime(rawhdr.hist.exp_time);
    setdescription(rawhdr.hist.descrip);
    setorient(analyze2psyorient(rawhdr.hist.orient));
    if(getbytesperpixel() == 1) swaptype = psynoswap;
  }
}

analyzefile::~analyzefile()
{
  hdrfd.close();
}

void analyzefile::openanalyzefiles(string fname, const char *mode)
{
// build complete file names
  bldanalyzefilenames(fname, &imgfilename, &hdrfilename);
// open header file
  psyopenfile(hdrfilename, mode, &hdrfd);
// open image file
  psyopenfile(imgfilename, mode, &imgfd);
}

void analyzefile::showhdr()
{
  showhdr(&rawhdr);
}

void analyzefile::showhdr(dsr rawhdr) {
  showhdr(&rawhdr);
}

void analyzefile::showhdr(dsr *rawhdr, ostream *out) {
  int i;
// print out header_key
  *out << "analyze header:\n";
  *out << "header_key.sizeof_hdr=" << rawhdr->hk.sizeof_hdr << '\n';
  *out << "header_key.data_type=" << rawhdr->hk.data_type << '\n';
  *out << "header_key.db_name=" << rawhdr->hk.db_name << '\n';
  *out << "header_key.extents=" << rawhdr->hk.extents << '\n';
  *out << "header_key.session_error=" << rawhdr->hk.session_error << '\n';
  *out << "header_key.regular=" << rawhdr->hk.regular << '\n';
  *out << "header_key.hkey_un0=" << rawhdr->hk.hkey_un0 << '\n';
// print out image_dimension
  for(i=0; i<8; i++)
    *out << "image_dimension.dim[" << i << "]=" << rawhdr->dime.dim[i] << '\n';
  *out << "image_dimension.vox_units=" << rawhdr->dime.vox_units << '\n';
  *out << "image_dimension.cal_units=" << rawhdr->dime.cal_units << '\n';
  *out << "image_dimension.unused1=" << rawhdr->dime.unused1 << '\n';
  *out << "image_dimension.datatype=" << rawhdr->dime.datatype << '\n';
  *out << "image_dimension.bitpix=" << rawhdr->dime.bitpix << '\n';
  *out << "image_dimension.dim_un0=" << rawhdr->dime.dim_un0 << '\n';
  for(i=0; i<8; i++)
    *out << "image_dimension.pixdim[" << i << "]=" << rawhdr->dime.pixdim[i] << '\n';
  *out << "image_dimension.vox_offset=" << rawhdr->dime.vox_offset << '\n';
  *out << "image_dimension.vox_offset=" << rawhdr->dime.vox_offset << '\n';
  *out << "image_dimension.funused1=" << rawhdr->dime.funused1 << '\n';
  *out << "image_dimension.funused2=" << rawhdr->dime.funused2 << '\n';
  *out << "image_dimension.funused3=" << rawhdr->dime.funused3 << '\n';
  *out << "image_dimension.cal_max=" << rawhdr->dime.cal_max << '\n';
  *out << "image_dimension.cal_min=" << rawhdr->dime.cal_min << '\n';
  *out << "image_dimension.compressed=" << rawhdr->dime.compressed << '\n';
  *out << "image_dimension.verified=" << rawhdr->dime.verified << '\n';
  *out << "image_dimension.glmax=" << rawhdr->dime.glmax << '\n';
  *out << "image_dimension.glmin=" << rawhdr->dime.glmin << '\n';
// print out data_history
  *out << "data_history.descrip=" << rawhdr->hist.descrip << '\n';
  *out << "data_history.aux_file=" << rawhdr->hist.aux_file << '\n';
  *out << "data_history.orient=" << (int)rawhdr->hist.orient << '\n';
  *out << "data_history.originator=" << rawhdr->hist.originator << '\n';
  union { char c[10]; short s[5]; } c_or_s;
  for(i = 0; i<10; i++) c_or_s.c[i] = rawhdr->hist.originator[i];
  for(i=0; i<5; i++) *out << "spm_origin[" << i << "]=" << c_or_s.s[i] << '\n';
  *out << "data_history.generated=" << rawhdr->hist.generated << '\n';
  *out << "data_history.scannum=" << rawhdr->hist.scannum << '\n';
  *out << "data_history.patient_id=" << rawhdr->hist.patient_id << '\n';
  *out << "data_history.exp_date=" << rawhdr->hist.exp_date << '\n';
  *out << "data_history.exp_time=" << rawhdr->hist.exp_time << '\n';
  *out << "data_history.hist_un0=" << rawhdr->hist.hist_un0 << '\n';
  *out << "data_history.views=" << rawhdr->hist.views << '\n';
  *out << "data_history.vols_added=" << rawhdr->hist.vols_added << '\n';
  *out << "data_history.start_field=" << rawhdr->hist.start_field << '\n';
  *out << "data_history.field_skip=" << rawhdr->hist.field_skip << '\n';
  *out << "data_history.omax=" << rawhdr->hist.omax << '\n';
  *out << "data_history.omin=" << rawhdr->hist.omin << '\n';
  *out << "data_history.smax=" << rawhdr->hist.smax << '\n';
  *out << "data_history.smin=" << rawhdr->hist.smin << '\n';
}

void analyzefile::writetofiles()
{
  writedata();

// initialize analyze header file
  rawhdr.hk.sizeof_hdr = sizeof(rawhdr);
  rawhdr.hk.extents=16384;
  rawhdr.hk.regular='r';
  rawhdr.hk.hkey_un0='h';
  rawhdr.dime.dim[0]=4;
  rawhdr.dime.dim[1]=size.x;
  rawhdr.dime.dim[2]=size.y;
  rawhdr.dime.dim[3]=size.z;
  rawhdr.dime.dim[4]=size.i;
  if((rawhdr.dime.dim[1] != size.x) || (rawhdr.dime.dim[2] != size.y) ||
     (rawhdr.dime.dim[3] != size.z) || (rawhdr.dime.dim[4] != size.i) ) {
    output_tree(&cerr);
    cerr<<"::writetofiles: aborting - unable to store one or more dimensions as short - ";
    cerr<< "size=(" << size.x << "," << size.y << "," << size.z << ","
    << size.i << ")\n";;
    exit(1);
  }
  strcpy(rawhdr.dime.vox_units, "mm.");
  rawhdr.dime.datatype=psytype2analyze(type);
                          //0=unknown,1=binary,2=uchar,4=short,
	                  //8=int,16=float,32=complex,64=double
  rawhdr.dime.bitpix=getbytesperpixel()*8;
  //?? word resolution not really defined by analyze
  // store wordres in cnu location
  rawhdr.dime.pixdim[0]=getwordres();
  // store wordres in spm location also
  rawhdr.dime.funused1=getwordres();
  rawhdr.dime.pixdim[1]=res.x*1000; // real world voxel width in mm
  rawhdr.dime.pixdim[2]=res.y*1000;
  rawhdr.dime.pixdim[3]=res.z*1000; // slice thickness in mm
  rawhdr.dime.pixdim[4]=res.i; //?? interval between images not really defined
  rawhdr.dime.vox_offset=skip;
  double max, min;
  getstats(&min, &max, NULL, NULL, NULL); // depends on stats being set by call to writedata() above
  rawhdr.dime.glmax=irint(max); //max value for file
  rawhdr.dime.glmin=irint(min); //min value for file
  strncpy(rawhdr.hist.descrip, getdescription().c_str(), 80);
  rawhdr.hist.descrip[79]='\0';
  rawhdr.hist.orient=psyorient2analyze(getorient());
  //0=transverse,1=coronal,2=sagittal,
  //3=transverse flipped,4=coronal flipped,5=sagittal flipped
  strncpy(rawhdr.hist.patient_id, getpatientid().c_str(), 10);
  rawhdr.hist.patient_id[9]='\0'; // analyze displays till null character
  strncpy(rawhdr.hist.exp_date, getdate().c_str(), 10);
  rawhdr.hist.exp_date[9]='\0';
  strncpy(rawhdr.hist.exp_time, gettime().c_str(), 10);
  rawhdr.hist.exp_time[9]='\0';
// write header to file
  if(writeAnalyzeHeader(&rawhdr, "", &hdrfd) != 0) {
    cerr<<"analyzefiles::writetofiles: error writing to file: "
      <<hdrfilename<<'\n';
    exit(1);
  }
/*
  hdrfd.write((char *)&rawhdr, sizeof(rawhdr));
  if(hdrfd.bad()) {
    cerr<<"analyzefiles::writetofiles: error writing to file: "
      <<hdrfilename<<'\n';
    exit(1);
  }
*/
// clean up
}
