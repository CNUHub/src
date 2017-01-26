#include "psyhdr.h"
#include "cnudicom.h"
#include <ctype.h>

const int DIRECTORY_RECORD_SEQUENCE = 0x00041220;
const unsigned int ITEM = 0xFFFEE000;
const unsigned int ITEM_DELIMITATION_ITEM = 0xFFFEE00D;

const int PIXEL_REPRESENTATION = 0x00280103;
const int SAMPLES_PER_PIXEL = 0x00280002;
const int PHOTOMETRIC_INTERPRETATION = 0x00280004;
const int PLANER_CONFIGURATION = 0x00280006;
const int TRANSFER_SYNTAX_UID = 0x00020010;
const int SLICE_SPACING = 0x00180088;
const int SLICE_THICKNESS = 0x00180050;
const int NUMBER_OF_FRAMES = 0x00280008;
const int ROWS = 0x00280010;
const int COLUMNS = 0x00280011;
const int PIXEL_SPACING = 0x00280030;
const int BITS_ALLOCATED = 0x00280100;
const int BITS_STORED = 0x00280101;
const int HIGH_BIT = 0x00280102;
const int SMALLEST_PIXEL_VALUE = 0x00280106;
const int LARGEST_PIXEL_VALUE = 0x00280107;
const int RED_PALETTE_DESCRIPTOR = 0x00281101;
const int GREEN_PALETTE_DESCRIPTOR = 0x00281102;
const int BLUE_PALETTE_DESCRIPTOR = 0x00281103;
const int RED_PALETTE = 0x00281201;
const int GREEN_PALETTE = 0x00281202;
const int BLUE_PALETTE = 0x00281203;
const int PIXEL_DATA = 0x7FE00010;
const int RESCALE_INTERCEPT = 0x00281052;
const int RESCALE_SLOPE = 0x00281053;
const int RESCALE_TYPE = 0x00281054;

const int FILE_SET_ID = 0x00041130;
const int DIRECTORY_RECORD_TYPE = 0x00041430;
const int OFFSET_NEXT_DIRECTORY_RECORD = 0x00041400;
const int PATIENT_NAME = 0x00100010;
const int PATIENT_ID = 0x00100020;
const int SERIES_NUMBER = 0x00200011;
const int SERIES_DESCRIPTION = 0x0008103E;
const int SERIES_INSTANCE_UID = 0x0020000E;
const int IMAGE_NUMBER = 0x00200013;
const int IMAGE_POSITION = 0x00200032;
const int IMAGE_ORIENTATION_PATIENT = 0x00200037;
const int TEMPORAL_POSITION_IDENTIFIER = 0x00200100;
const int REFERENCED_FILE_ID = 0x00041500;

char* vrModeNames[] = {
  "UNKNOWN_VR_MODE", "IMPLICIT_VR", "EXPLICIT_VR" };

DICOM_VR AE(0x4145, "Application Entity", psystring);//16 bytes max
DICOM_VR AS(0x4153, "Age String", psystring);//4 bytes fixed
DICOM_VR AT(0x4154, "Attribute Tag", psyushort);
DICOM_VR CS(0x4353, "Code String", psystring);
DICOM_VR DA(0x4441, "Date", psystring);
DICOM_VR DS(0x4453, "Decimal String", psystring);
DICOM_VR DT(0x4454, "Date Time", psystring);
DICOM_VR FL(0x464C, "Floating Point Single", psyfloat);
DICOM_VR FD(0x4644, "Floating Point Double", psydouble);
DICOM_VR IS(0x4953, "Integer String", psystring); //12 bytes max
DICOM_VR LO(0x4C4F, "Long String", psystring); //64 chars max
DICOM_VR LT(0x4C54, "Long Text", psystring);//10240 chars max
DICOM_VR OB(0x4F42, "Other Byte String", psychar);
DICOM_VR OW(0x4F57, "Other Word String", psyshort);
DICOM_VR PN(0x504E, "Person Name", psystring);//5 groups X 64max
DICOM_VR SH(0x5348, "Short String", psystring);//16 chars max
DICOM_VR SL(0x534C, "Signed Long", psyint);
DICOM_VR SQ(0x5351, "Value Sequence", psydicomdataelement);
DICOM_VR SS(0x5353, "Signed Short", psyshort);
DICOM_VR ST(0x5354, "Short Text", psystring);//1024 chars max
DICOM_VR TM(0x544D, "Time", psystring);//16 bytes max
DICOM_VR UI(0x5549, "Unique Identifier", psystring);//64 bytes max
DICOM_VR UL(0x554C, "Unsigned Long", psyuint);
DICOM_VR UN(0x554E, "Unknown", psychar);
DICOM_VR US(0x5553, "Unsigned Short", psyushort);
DICOM_VR UT(0x5554, "Unlimited Text", psystring);
DICOM_VR QQ(0x3F3F, "", psynotype);
DICOM_VR* vr_list[] = {
  &AE, &AS, &AT, &CS, &DA, &DS, &DT, &FD, &FL, &IS, &LO, &LT, &PN,
  &SH, &SL, &SS, &ST, &TM, &UI, &UL, &US, &UT, &OB, &OW, &SQ, &UN,
  &QQ, NULL
};

inline void reversebytes(unsigned char *c, int bytes) {
  // reverse bytes
  // 0,1->1,0; 0,1,2,3->3,2,1,0; 0,1,2,3,4,5,6,7->7,6,5,4,3,2,1,0
  int b1 = 0;
  int b2 = bytes - 1;
  for(; b2 > b1; b2--, b1++) {
    unsigned char tmp = c[b2]; c[b2] = c[b1]; c[b1] = tmp;
  }
}

inline void swapwords(unsigned char *c, int words, int bytesperword) {
  int w1 = 0;
  int w2 = words - 1;
  for(; w2 > w1; w2--, w1++) {
    for(int i=0; i<bytesperword; i++, w1++, w2++) {
      unsigned char tmp = c[w1]; c[w2] = c[w2]; c[w1] = tmp;
    }
  }
}

// remove leading and trailing white spaces
inline void TRIMSTRING(char *c) {
  char *ptr1, *ptr2;
  ptr1 = ptr2 = c;
  while(isspace(*ptr2)) ptr2++; // skip leading spaces
  while(*ptr2 != '\0') *ptr1++ = *ptr2++; // copy remaining
  *ptr1 = '\0';
  // remove trailing spaces
  ptr1--;
  while(ptr1 > c && isspace(*ptr1)) *ptr1-- = '\0';
}

void DICOM_VR::show(ostream *out) {
  *out<<"vr=0x"<<hex<<setfill('0')<<setw(4)<<vr;
  *out<<"="<<(char)((vr>>8) & 0xFF)<<(char)(vr & 0xFF);
  *out<<" vrname=\""<<name<<'\"';
  *out<<" vrtype="<<psytypenames[type];
}

void DICOM_Tag::show(ostream *out, show_mode showMode) {
  if(showMode > NORMAL) {
    *out<<"groupNelement=0x"<<hex<<setfill('0')<<setw(8)<<groupNelement<<dec;
    *out<<" tagimplicitvr="; vr->show(out);
    *out<<" ";
  }
  if(showMode > CONCISE) *out<<"tagname=\"";
  *out<<name<<'\"';
}

DICOM_VR* getDICOM_VR(int vr) {
  for(int i=0; vr_list[i] != NULL; i++) {
    if(vr == vr_list[i]->getVR()) return vr_list[i];
  }
  return NULL;
}

void show_dde_value(DICOM_DataElement *dde, ostream *out, show_mode showMode=NORMAL) {
  if(dde != NULL) {
    if(dde->data != NULL) {
      if(showMode > BRIEF) *out<<"value=";
      if(dde->storageType == psystring) {
	*out<<'\"';
	for(int i=0; i<dde->dataLength && *(dde->data+i) != '\0'; i += 1) *out<<*(dde->data+i);
	*out<<'\"';
      }
      else if(dde->dataLength < 1024) {
	int bytesperword = gettypebytes(dde->storageType);
	*out<<"{";
	for(int i=0; i<dde->dataLength; i += bytesperword) {
	  double d;
	  type2double(dde->data + i, dde->storageType, (char *) &d);
	  if(i > 0) *out<<",";
	  *out<<dec<<d;
	}
	if(dde->storageType == psychar) {
	  *out<<"=\"";
	  for(int i=0; i<dde->dataLength; i += 1) *out<<*(dde->data+i);
	  *out<<'\"';
	}
	*out<<"}";
      }
    }
  }
}

void DICOM_DataElement::show(ostream *out, show_mode showMode) {
  if(showMode > NORMAL) *out<<"tag= ";
  if(showMode > CONCISE) tag->show(out, showMode);
  if(showMode > NORMAL) {
    *out<<" vr="; vr->show(out);
    *out<<" mode="<<vrModeNames[mode];
    *out<<" fileOffset="<<dec<<fileOffset;
    *out<<" dataFileOffset="<<dec<<dataFileOffset;
    *out<<" dataLength="<<dec<<dataLength;
    *out<<" type="<<psytypenames[storageType];
  }
  if(showMode > CONCISE) *out<<" ";
  show_dde_value(this, out, showMode);
}

DICOM_DataElement *getDICOM_DataElement(fstream *fd, vrMode mode,
					psyswaptype swap)
{
  int error=0;
  DICOM_Tag* tag = NULL;
  DICOM_VR* vr = NULL;
  int dataLength = 0;
  long fileOffset = (long) fd->tellg();
  unsigned char b[4];
  fd->read((char *) b, 4);
  if(swap == psyreversewordbytes)
  {
    reversebytes(b,2); reversebytes(b+2,2);
  }

  int group = *((unsigned short *) b);
  int element = *((unsigned short *) (b+2));
  tag = getDICOM_Tag(group, element);
  fd->read((char *) b, 4);
  if(mode != IMPLICIT_VR) {
    int explicitVr = ((b[0] << 8) & 0xFF00) | (b[1] & 0xFF);
    vr = getDICOM_VR(explicitVr);
    if(vr != NULL) {
      mode = EXPLICIT_VR;
      if((vr == &OB) || (vr == &OW) || (vr == &SQ) ||
	 (vr == &UN) || (vr == &UT) ) {
	if((b[2] != 0) || (b[3] != 0)) {
	  cerr<<"explicit vr not followed by two 0 bytes\n";
	  error=1;
	}
	fd->read((char *) b, 4);
	if(swap == psyreversewordbytes) reversebytes(b, 4);
	dataLength = *((int *) b);
	if((dataLength == -1)) ; // found DICOM files with -1 used for start of directory sequence
	else if(dataLength < 0) {
	  cerr<<"invalid negative explicit long data length at tag offset="<<fileOffset<<"\n";
	  error=1;
	}
	else if(dataLength%2) {
	  cerr<<"invalid odd data length="<<dataLength<<" at tag offset=";
	  cerr<<fileOffset<<"\n";
	  if(dataLength == 13) {
	    cerr<<"known common error of length 13 being reset to 10\n";
	    dataLength = 10;
	  }
	}
      }
      else {
	if(swap == psyreversewordbytes) reversebytes(b+2, 2);
	dataLength = *((unsigned short *) (b+2));
	if(dataLength%2) {
	  cerr<<"invalid odd data length="<<dataLength<<" at tag offset=";
	  cerr<<fileOffset<<"\n";
	  if(dataLength == 13) {
	    cerr<<"known common error of length 13 being reset to 10\n";
	    dataLength = 10;
	  }
	}
      }
    }
    else if (mode == EXPLICIT_VR) {
      cerr<<"unknown explicit VR at tag offset="<<fileOffset<<"\n";
      error=1;
    }
    else {
      vr = tag->getImplicitVR();
      mode = IMPLICIT_VR;
    }
  }
  if(! error) {
    if(mode == IMPLICIT_VR) {
      if(swap == psyreversewordbytes) reversebytes(b, 4);
      dataLength = *((int *) b);
      if(dataLength == -1) ; // found DICOM files with -1 used for item sequence
      else if(dataLength < 0) {
	cerr<<"invalid negative implicit data length at tag offset="<<fileOffset<<"\n";
	error=1;
      }
      else if(dataLength%2) {
	cerr<<"invalid implicit mode odd data length="<<dataLength<<" at tag offset=";
	cerr<<fileOffset<<"\n";
	if(dataLength == 13) {
	  cerr<<"known common error of length 13 being reset to 10\n";
	  dataLength = 10;
	}
      }
    }
  }
  if(error) {
    cerr<<"tag= ";
    tag->show(&cerr, VERBOSE);
    cerr<<" vr="; vr->show(&cerr);
    cerr<<" mode="<<vrModeNames[mode];
    cerr<<" fileOffset="<<dec<<fileOffset;
    cerr<<" dataLength="<<dec<<dataLength;
    cerr<<"\n";
    return NULL;
  }
  else {
    DICOM_DataElement* dataElement = new DICOM_DataElement();
    dataElement->tag = tag;
    dataElement->vr = vr;
    dataElement->mode = mode;
    dataElement->storageType = vr->getType();
    if(dataElement->storageType == psydicomdataelement)
      dataElement->storageType=psyuchar;
    dataElement->dataLength = dataLength;
    dataElement->fileOffset = fileOffset;
    dataElement->dataFileOffset = (long) fd->tellg();
    dataElement->link = NULL;
    return dataElement;
  }
}

void writeDICOM_DataElementHeader(DICOM_DataElement *dde, fstream *fd,
				  psyswaptype swap)
{
  DICOM_Tag *tag = dde->tag;
  union { unsigned char b[4]; int i; unsigned short s[2]; } combo;
//  combo.i = tag->getCombinedGroupAndElement();
 combo.s[0] = (tag->getCombinedGroupAndElement() & 0xFFFF0000) >> 16;
 combo.s[1] = (tag->getCombinedGroupAndElement() & 0xFFFF);
  if(swap == psyreversewordbytes) {
    reversebytes(combo.b,2); reversebytes(combo.b+2,2);
  }
  fd->write((char *) combo.b, 4);
  if(dde->mode == EXPLICIT_VR) {
    DICOM_VR *vr = dde->vr;
    int tmp = vr->getVR();
    combo.b[0] = (unsigned char) ((tmp >> 8) & 0xFF);
    combo.b[1] = (unsigned char) (tmp & 0xFF);
    if((vr == &OB) || (vr == &OW) || (vr == &SQ) ||
       (vr == &UN) || (vr == &UT) ) {
      combo.b[2] = 0; combo.b[3] = 0;
      fd->write((char *) combo.b, 4);
      combo.i = dde->dataLength;
      if(swap == psyreversewordbytes) reversebytes(combo.b, 4);
      fd->write((char *) combo.b, 4);
    }
    else {
      combo.s[1] = (unsigned short) dde->dataLength;
      if(combo.s[1] != dde->dataLength) {
	cerr<<"data length too large for storage as unsigned short as required by explicit and vr=";
	vr->show(&cerr);
	cerr<<"\ndata element=";
	dde->show(&cerr);
      }
      if(swap == psyreversewordbytes) reversebytes(combo.b+2, 2);
      fd->write((char *) combo.b, 4);
    }
  }
  else {
    // implicit
    if(dde->vr != tag->getImplicitVR()) {
      cerr<<"implicit vr doesn't match actual vr for data element=";
      dde->show(&cerr);
    }
    combo.i = dde->dataLength;
    if(swap == psyreversewordbytes) reversebytes(combo.b, 4);
    fd->write((char *) combo.b, 4);
  }
}

void writeDICOM_DataElementData(DICOM_DataElement *dde, unsigned char *dataptr,
				fstream *fd, psyswaptype swap)
{
  // write the data
  int bytesperword = gettypebytes(dde->storageType);
  if(dataptr == NULL) {
    cerr<<"no data for data element=";
    dde->show(&cerr);
  }
  else {
    unsigned char *endptr = dataptr + dde->dataLength;

    if((swap == psyreversewordbytes) && (bytesperword > 1)) {
      unsigned char reversedbytes[bytesperword];
      for(; dataptr < endptr; ) {
	for(int i=bytesperword-1; i>=0; i--, dataptr++)
	  reversedbytes[i] = *dataptr;
	fd->write((char *) reversedbytes, bytesperword);
      }
    }
    else for(; dataptr < endptr; dataptr += bytesperword)
      fd->write((char *) dataptr, bytesperword);
  }
}

void readDicom_DataElementData(DICOM_DataElement *dde, fstream *fd, psyswaptype swap) {
  if(dde->dataLength > 0) {
    dde->data = new char[dde->dataLength];
    fd->read(dde->data, dde->dataLength);
    int bytesperword = gettypebytes(dde->storageType);
    if((swap == psyreversewordbytes) && (bytesperword > 1)) {
      unsigned char *dataptr = (unsigned char *) dde->data;
      unsigned char *endptr = dataptr + dde->dataLength;
      for(; dataptr < endptr; dataptr += bytesperword) reversebytes(dataptr,  bytesperword);
    }
  }
  else dde->data = new char[0];
  return;
}

int isdicomfile(string name) {
  int status;
  fstream fd;
  psyopenfile(name, "r", &fd, &status);
  if(status != 0) {
    //    cerr<<"dicomfile2:isdicomfile failed opening file=\""<<name<<"\"\n";
    return 0;
  }
  fd.seekg(128, ios::beg);
  char b[4];
  fd.read(b, 4);
  fd.close();
  if(strncmp(b, "DICM", 4) != 0) {
    //    cerr<<"dicomfile2:isdicomfile DICM not found in file=\""<<name<<"\"\n";
    return 0;
  }
  return 1;
}

psytype getdicomclosesttype(psytype type) {
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
  case psycomplex:
    return psystring;
  case psydate:
    return psydate;
  case psystring:
    return psystring;
  case psydicomdataelement:
    return psydicomdataelement;
  case psyrgb:
    return psyrgb;
  case psyargb:
    return psyargb;
  default:
  case psynotype:
    return psynotype;
  }
}

int getdicomnumberofbits(psytype type) {
  switch (type) {
  case psyuchar:
  case psychar:
  case psyrgb:
  case psyargb:
  case psydate:
  case psystring:
  case psydicomdataelement:
  default:
  case psynotype:
    return 8;
  case psyshortsw:
  case psyshort:
  case psyushort:
    return 16;
  case psyuint:
  case psyint:
  case psyfloat:
    return 32;
  case psydouble:
  case psycomplex:
    return 64;
  }
}

int getdicomsamplesperpixel(psytype type) {
  switch (type) {
  case psyuchar:
  case psychar:
  case psyshortsw:
  case psyshort:
  case psyushort:
  case psyuint:
  case psyint:
  case psyfloat:
  case psydouble:
  case psycomplex:
  case psydate:
  case psystring:
  case psydicomdataelement:
  default:
  case psynotype:
    return 1;
  case psyrgb:
    return 3;
  case psyargb:
    return 4;
  }
}

enum psychartocase {psytolower=-1, psynochange, psytoupper};
/*
  Build a unix file name relative to the dicomdirfile based on refname.
*/
string blddicomreferencefilename(string refname, string dicomdirfile,
				 int casechange=psynochange)
{
  string localfullrefname;

  // check inputs
  size_t namelength=refname.length();
  if(namelength > 0) {
    // put directory portion of dicomdir at beginning of filename
    size_t dirfilelength = dicomdirfile.length();
    if(dirfilelength > 0) {
      localfullrefname = dicomdirfile;
      // erase everything after last '/'
      size_t lastbackslash = localfullrefname.rfind('/');
      if(lastbackslash == string::npos) localfullrefname.erase(); // no back slash
      else if((lastbackslash+1) < localfullrefname.length())
	localfullrefname.erase(lastbackslash + 1);
    }

    // add refname to end of file name
    // while doing needed conversions
    for(string::iterator it=refname.begin(); it < refname.end(); it++) {
      char c = *it;
      if(c == '\\') c = '/';
      else {
	switch(casechange) {
	case psytolower:
	  c = tolower(c);
	  break;
	case psytoupper:
	  c = toupper(c);
	  break;
	default:
	  break;
	}
      }
      localfullrefname += c;
    } // end for (iterator it=...
    // remove spaces from end of string
    size_t lastnotspace = localfullrefname.find_last_not_of(' ');
    if(lastnotspace != string::npos && (lastnotspace+1) < localfullrefname.length())
      localfullrefname.erase(lastnotspace+1);
    /*
    char *in_ptr = refname;
    while(*in_ptr != '\0') {
      if(*in_ptr == '\\') *out_ptr = '/';
      else {
	switch(casechange) {
	case psytolower:
	  *out_ptr = tolower(*in_ptr);
	  break;
	case psytoupper:
	  *out_ptr = toupper(*in_ptr);
	  break;
	default:
	  *out_ptr = *in_ptr;
	}
      }
      out_ptr++;
      in_ptr++;
    }
    *out_ptr = '\0';
    out_ptr--;
    // remove spaces from end of string
    while(*out_ptr == ' ' && (out_ptr > localfullrefname) ) {
      *out_ptr = '\0';
      out_ptr--;
    }
    */

    //    ptr = localfullrefname + strlen(localfullrefname) - 1;
    //    while(*ptr == ' ') *ptr-- = '\0';
  } // end if(namelength > 0)
  return localfullrefname;
}

int typeAsStringLength(psytype intype) {
  switch(intype) {
  case psychar:
  case psyuchar:
    return 5;
  case psyshort:
    return 7;
  case psyushort:
    return 6;
  case psyuint:
  case psyint:
    return 12;
  case psyfloat:
    return 17;
  case psydouble:
    return 21;
  case psystring:
  default:
    cerr<<"typeAsStringLength unknown type="<<psytypenames[intype]<<"\n";
    return -1;
    exit(1);
  }
}

char *type2string(char * in, psytype intype, char *out) {
  char *string = out;
  if(string == NULL) {
    int length;
    if(intype == psystring) length = strlen(in) + 1;
    else length = typeAsStringLength(intype);
    string = new char[typeAsStringLength(intype)];
  }
  switch(intype) {
  case psychar:
  case psyuchar:
    sprintf(string, "0x%.2x", *in);
    break;
  case psyshort:
    sprintf(string, "%d", *((short *) in));
    break;
  case psyushort:
    sprintf(string, "%d", *((unsigned short *) in));
    break;
  case psyint:
    sprintf(string, "%d", *((int *) in));
    break;
  case psyuint:
    sprintf(string, "%d", *((unsigned int *) in));
    break;
  case psyfloat:
    sprintf(string, "%6e", (double) *((float *) in));
    break;
  case psydouble:
    sprintf(string, "%10e", *((double *) in));
    break;
  case psystring:
    string = strcpy(string, in);
    break;
  default:
    cerr<<"type2string - no conversion to psystring from type="<<psytypenames[intype]<<" available\n";
    exit(1);
  }
  return string;
}

DICOM_DataElement *buildDICOM_DataElement(char *data, psytype intype,
					  int inwords,
					  DICOM_Tag *tag,
					  vrMode mode, DICOM_VR *vr=NULL) {
  DICOM_DataElement *dde = new DICOM_DataElement();
  dde->tag = tag;
  if(mode == IMPLICIT_VR) {
    dde->mode = IMPLICIT_VR;
    dde->vr = tag->getImplicitVR();
  }
  else {
    dde->mode = EXPLICIT_VR;
    if(vr == NULL) vr = tag->getImplicitVR();
    dde->vr = vr;
  }
  dde->storageType = dde->vr->getType();
  if(dde->storageType == psydicomdataelement) dde->storageType=psyuchar;
  // set intype for ow 05/08/2012
  else if(dde->vr == &OW && intype != psynotype) dde->storageType=intype;
  int inwordbytes = gettypebytes((intype==psystring) ? psychar : intype);
  dde->data = NULL;
  if(dde->storageType == psystring) {
    if(intype == psystring) dde->dataLength = inwords + 1;
    else {
      dde->dataLength = typeAsStringLength(intype) * inwords;
      dde->dataLength += inwords - 1;
    }
    dde->dataLength += (dde->dataLength)%2; // should be even
  }
  else dde->dataLength = inwords * gettypebytes(dde->storageType);

  if(data != NULL) {
    char *inptr = data;
    dde->data = new char[dde->dataLength + 1];
    char *outptr = dde->data;
    // initialize with nulls
    for(int n=0; n < dde->dataLength+1; n++, outptr++) *outptr = '\0';
    outptr = dde->data;
    if(dde->storageType == psystring) {
      // *(dde->data) = '\0';
      if(intype == psystring) {
	outptr = dde->data;
	strncpy(outptr, inptr, dde->dataLength + 1);
      }
      else {
	for(int i=0; i<inwords; i++, inptr+=inwordbytes) {
	  if(*(dde->data) != '\0') { strcat(outptr, "\\"); outptr++; }
	  type2string(inptr, intype, outptr);
	  outptr += strlen(outptr);
	}
      }
      int tmplength = strlen(dde->data) + 1;
      tmplength += tmplength%2;  // if odd make even
      if(tmplength > dde->dataLength) {
	cerr<<"buildDICOM_DataElement - string length greater then storage array\n";
	cerr<<"buildDICOM_DataElement - this should not happen - exiting\n";
	exit(1);
      }
      dde->dataLength = tmplength;
    }
    else {
      int outwordbytes = gettypebytes(dde->storageType);
      for(int i=0; i<inwords; i++, inptr+=inwordbytes, outptr+=outwordbytes) {
	pixeltypechg(data, intype, dde->data, dde->storageType);
      }
    }  
  }
  return dde;
}

dicomfile::dicomfile(string fname, psyimg *psyimgptr,
		     psytype pixeltype)
  : rawfile(psyimgptr, pixeltype) {
  combinedFileSetPtr=NULL;
  psyopenfile(fname, "w", &imgfd);
  imgfilename=fname;
#ifdef USES_LITTLE_ENDIAN
  //  swaptype = psyreversewordbytes;
#else
  swaptype = psyreversewordbytes;
  //  swaptype = psynoswap;
#endif
  swaptype = psynoswap;
  // build the data elements from the input image
  psydims dims = psyimgptr->getsize();
  psyres res = psyimgptr->getres();
  double wres = psyimgptr->getwordres();
  psytype type = psyimgptr->gettype();
  //  int length = psyimgptr->getlength();
  DICOM_DataElement *dde = buildDICOM_DataElement((char *) &(dims.z), psyint, 1,
						  getDICOM_Tag(NUMBER_OF_FRAMES),
						  EXPLICIT_VR);
  firstDataElement = dde;

  dde->link = buildDICOM_DataElement((char *) &(dims.y), psyint, 1,
				     getDICOM_Tag(ROWS), EXPLICIT_VR);
  dde = dde->link;

  dde->link = buildDICOM_DataElement((char *) &(dims.x), psyint, 1,
				     getDICOM_Tag(COLUMNS), EXPLICIT_VR);
  dde = dde->link;

  double dtemp = res.z * 1e3; // convert meters to mm
  dde->link = buildDICOM_DataElement((char *) &(dtemp), psydouble, 1,
  				     getDICOM_Tag(SLICE_SPACING), EXPLICIT_VR);
  dde = dde->link;

  int tmplength =  2 * typeAsStringLength(psydouble) + 4;
  char strptr[tmplength];
  sprintf(strptr, "%10e\\%10e", res.x*1e3, res.y*1e3); // in mm
  dde->link = buildDICOM_DataElement(strptr, psystring, strlen(strptr),
  				     getDICOM_Tag(PIXEL_SPACING), EXPLICIT_VR);
  dde = dde->link;

  int bitsallocated = getdicomnumberofbits(type);
  dde->link = buildDICOM_DataElement((char *) &bitsallocated, psyint, 1,
  				     getDICOM_Tag(BITS_ALLOCATED),
				     EXPLICIT_VR);
  dde = dde->link;

  dde->link = buildDICOM_DataElement((char *) &bitsallocated, psyint, 1,
  				     getDICOM_Tag(BITS_STORED),
				     EXPLICIT_VR);

  dde = dde->link;

  int itmp = getdicomsamplesperpixel(type);
  dde->link = buildDICOM_DataElement((char *) &itmp, psyint, 1,
  				     getDICOM_Tag(SAMPLES_PER_PIXEL),
				     EXPLICIT_VR);
  dde = dde->link;

  char *photometric;
  if(type == psyrgb) photometric = "RGB";
  else if(type == psyargb) photometric = "ARGB";
  else photometric = "MONOCHROME2";
  dde->link = buildDICOM_DataElement(photometric, psystring, strlen(photometric),
  				     getDICOM_Tag(PHOTOMETRIC_INTERPRETATION),
				     EXPLICIT_VR);
  dde = dde->link;

  if(type == psyrgb || type == psyargb) {
    int planer_conf = 0; // 000 = R1,G1,B1,R2,G2,B2, ... // 001 = R1,R2,R3, ... G1,G2,G2, ... B1,B2,B3
    dde->link = buildDICOM_DataElement((char *) &planer_conf, psyint, 1,
				       getDICOM_Tag(PLANER_CONFIGURATION),
				       EXPLICIT_VR);
    dde = dde->link;

  }

  int pixel_rep = istypesigned(type);
  dde->link = buildDICOM_DataElement((char *) &pixel_rep, psyint, 1,
  				     getDICOM_Tag(PIXEL_REPRESENTATION),
				     EXPLICIT_VR);
  dde = dde->link;


  dde->link = buildDICOM_DataElement((char *) &wres, psydouble, 1,
  				     getDICOM_Tag(RESCALE_SLOPE),
				     EXPLICIT_VR);
  dde = dde->link;

  dde->link = buildDICOM_DataElement(NULL, type, size.x*size.y*size.z*size.i,
    				     getDICOM_Tag(PIXEL_DATA),
  				     IMPLICIT_VR);
  dde = dde->link;

  lastDataElement=dde;
  lastDataElement->link = NULL;

  for(int j=0; j<128; j++) imgfd.write(" ", 1);
  imgfd.write("DICM", 4);
  for(dde=firstDataElement; dde != NULL; dde=dde->link) {
    writeDICOM_DataElementHeader(dde, &imgfd, swaptype);
    if(dde->data != NULL)
      writeDICOM_DataElementData(dde, (unsigned char *) dde->data,
				 &imgfd, swaptype);
  }
  writedata();
}

dicomfile::dicomfile(string fname, const char *mode) : rawfile(fname, mode) {
  combinedFileSetPtr=NULL;
  imgfd.seekg(128, ios::beg);
  char b[4];
  imgfd.read(b, 4);
  if(strncmp(b, "DICM", 4) != 0) {
    cerr <<"dicomfile::dicomfile:  file missing \"DICM\" string\n";
    imgfd.seekg(0);
  }
  firstDataElement = NULL;
  lastDataElement = NULL;
  vrMode vrmode = UNKNOWN_VR_MODE;
#ifdef USES_LITTLE_ENDIAN
  //  swaptype = psyreversewordbytes;
#else
  swaptype = psyreversewordbytes;
  //  swaptype = psynoswap;
#endif
  int moreElements = 1;
  int xdim=1; int ydim=1; int zdim=1; int idim=1;
  double zres=1.0; double yres=1.0; double xres=1.0;
  double ires=1.0; double wres = 1.0;
  DICOM_DataElement *rawData_dde = NULL;

  int samplesPerPixel = 1;
  char *photometric_interpretation = NULL;
  //  int planer_configuration = -1;
  int bits_allocated = -1;
  int bits_stored = -1;
  int high_bit = -1;
  int pixel_representation = -1;

  int seriesNumber = -1;
  int keepSeriesNumber = -1;
  int seriesErrorOutted=0;
  char *seriesInstanceUID = NULL;
  char *keepSeriesInstanceUID = NULL;

  int imageNumber=-1;
  int firstImagePositionFound=0;
  int secondImagePositionFound=0;
  double firstImageXPosition=0;
  double secondImageXPosition=0;
  double firstImageYPosition=0;
  double secondImageYPosition=0;
  double firstImageZPosition=0;
  double secondImageZPosition=0;
  int imageOrientationPatientFound=0;
  double xRowCosine=0;
  double yRowCosine=0;
  double zRowCosine=0;
  double xColumnCosine=0;
  double yColumnCosine=0;
  double zColumnCosine=0;
  int changecase = psynochange;

  string filename;
  while(moreElements) {
    long ddeLocation = (long) imgfd.tellg();
    DICOM_DataElement *dde = getDICOM_DataElement(&imgfd, vrmode, swaptype);
    if(dde == NULL) {
      cerr<<"error reading data element at file offset=";
      cerr<<ddeLocation<<" bytes\n";
      exit(1);
    }
    else {
      // keep a link list of data elements
      dde->link = NULL;
      if(firstDataElement == NULL) firstDataElement = dde;
      else lastDataElement->link = dde;
      lastDataElement = dde;

      int seekbytes = 0;
      switch(dde->tag->getCombinedGroupAndElement()) {
      case NUMBER_OF_FRAMES:
	//	dde->data = new char[dde->dataLength];
	//	imgfd.read(dde->data, dde->dataLength);
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &zdim);
	break;
      case ROWS:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &ydim);
	break;
      case COLUMNS:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &xdim);
	break;
      case SLICE_SPACING:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2double(dde->data, dde->storageType, (char *) &zres);
	zres *= 1e-3; // convert mm to meters
	break;
      case PIXEL_SPACING:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	if(dde->storageType == psystring) {
	  int cnt = sscanf(dde->data, "%lf\\%lf", &xres, &yres);
	  if(cnt < 1) { xres = yres = 1.0; }
	  else if(cnt == 1) yres = xres;
	  xres *= 1e-3; yres *= 1e-3; // convert mm to meters
	}
	break;
      case SAMPLES_PER_PIXEL:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	//	dde->data = new char[dde->dataLength];
	//	imgfd.read(dde->data, dde->dataLength);
	type2int(dde->data, dde->storageType, (char *) &samplesPerPixel);
	/*
	if(samplesPerPixel != 1) {
	  cerr<<"error - ignoring samples per pixel (="<<samplesPerPixel<<
	    ") not equal to 1\n";
	}
	*/
	break;
      case BITS_ALLOCATED:
	// number of bits allocated per pixel >= bits stored
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &bits_allocated);
	break;
      case BITS_STORED:
	// number of bits allocated per pixel <= bits_allocated
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &bits_stored);
	break;
      case HIGH_BIT:
	// high bit of bits stored (< bits allocated) (> bits stored -1)
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &high_bit);
	break;
      case PIXEL_REPRESENTATION:
	// 0x0000 = unsigned integer, 0x0001 = 2's complement 
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &pixel_representation);
	break;
      case RESCALE_SLOPE:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	if(dde->storageType == psystring) sscanf(dde->data, "%lf", &wres);
	break;
      case PIXEL_DATA:
	rawData_dde = dde;
	seekbytes = dde->dataLength;
	break;
      case SERIES_NUMBER:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &seriesNumber);
	break;
      case SERIES_INSTANCE_UID:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	if(dde->storageType == psystring) {
	  seriesInstanceUID = dde->data;
	  if(seriesInstanceUID != NULL) TRIMSTRING(seriesInstanceUID);
	}
	break;
      case REFERENCED_FILE_ID:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	//	cout<<"dicomfile::dicomfile case REFERENCED_FILE_ID data=" <<dde->data <<"\n";
	//	cout<<"imgfilename="<<imgfilename<<"\n";

	filename = blddicomreferencefilename(dde->data, imgfilename, changecase);
	//	cout<<" built filename="<<filename<<"\n";
	if(combinedFileSetPtr == NULL) {
	  keepSeriesNumber = seriesNumber;
	  keepSeriesInstanceUID = seriesInstanceUID;
	  if(! isdicomfile(filename)) {
	    // try lower case
	    changecase = psytolower;
	    filename = blddicomreferencefilename(dde->data, imgfilename, changecase);
	    if(! isdicomfile(filename)) {
	      // try upper case
	      changecase = psytoupper;
	      filename = blddicomreferencefilename(dde->data, imgfilename, changecase);
	      if(! isdicomfile(filename)) {
		filename = blddicomreferencefilename(dde->data, imgfilename, psynochange);
		cerr << "dicomfile::dicomfile - error failed reading series=\""<<seriesNumber<<"\" dicom file=\"" << filename << "\"\n";
		cerr << "dicomfile::dicomfile - attempted both upper and lower case file names\n";
		// exit(1);
		filename.erase();
	      }
	    }
	  }
	  if(! filename.empty()) {
	    cout<<"starting combined file set with "<<filename<<" in series "<<seriesNumber;
	    cout<<" series Instance UID "<<seriesInstanceUID<<'\n';
	    combinedFileSetPtr = new dicomfile(filename, "r");
	  }
	}
	else if( (keepSeriesNumber == seriesNumber) &&
		 ((keepSeriesInstanceUID == NULL) || (strcmp(keepSeriesInstanceUID, seriesInstanceUID) == 0))
		 ) {
	  if(isdicomfile(filename)) {
	    cout<<"calling concatimgs for "<<filename<<" in series "<<seriesNumber;
	    cout<<" series Instance UID "<<seriesInstanceUID<<'\n';
	    dicomfile *addtocombined = new dicomfile(filename, "r");
	    if(combinedFileSetPtr->gettype() != addtocombined->gettype()) {
	      cout<<"warning - referenced file not concatenated because not the same data type - "<<filename<<" in series "<<seriesNumber<<'\n';
	    }
	    else {
	      combinedFileSetPtr = (psyimg *) new concatimgs(combinedFileSetPtr,
							     (psyimg *) addtocombined);
	    }
	  }
	  else {
	    cout<<"not a dicom file - "<<filename<<" in series "<<seriesNumber<<'\n';
	  }
	}
	else if(! seriesErrorOutted) {
	  cerr<<"warning - dicomdir with multiple series - only first (series "<<keepSeriesNumber<<") used\n";
	  seriesErrorOutted = 1;
	}
	break;
      case IMAGE_POSITION:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	if(imageNumber == 1) {
	  if(sscanf(dde->data, "%lf\\%lf\\%lf",
		    &firstImageXPosition, &firstImageYPosition, &firstImageZPosition) == 3) {
	    firstImagePositionFound = 1;
	  }
	}
	else if(imageNumber == 2) {
	  if(sscanf(dde->data, "%lf\\%lf\\%lf",
		    &secondImageXPosition, &secondImageYPosition, &secondImageZPosition) == 3) {
	    secondImagePositionFound = 1;
	  }
	}
	break;
      case IMAGE_ORIENTATION_PATIENT:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	if(! imageOrientationPatientFound ) {  // hopefully same orientation for all slices
	  if(sscanf(dde->data, "%lf\\%lf\\%lf\\%lf\\%lf\\%lf",
		    &xRowCosine, &yRowCosine, &zRowCosine, &xColumnCosine, &yColumnCosine, &zColumnCosine) == 6) {
	    firstImagePositionFound = 1;
	  }
	}
	break;
      case IMAGE_NUMBER:
	readDicom_DataElementData(dde, &imgfd, swaptype);
	type2int(dde->data, dde->storageType, (char *) &imageNumber);
	break;
      case DIRECTORY_RECORD_SEQUENCE:
	cerr<<"found directory sequence\n";
	break;
      case ITEM:
	break;
      case ITEM_DELIMITATION_ITEM:
	break;
      case PHOTOMETRIC_INTERPRETATION:
	// "MONOCHROME1" samples per pixel == 1, min value = white
	// "MONOCHROME2" samples per pixel == 1, min value = black
	// "PALETTE COLOR" samples per pixel == 1, red, green, blue palletes needed
	// "RGB" samples per pixel == 3
	// "ARGB" samples per pixel == 4
	// "YBR_FULL" samples per pixel == 3
	// "YBR_FULL_422" samples per pixel == 3
	// "YBR_PARTIAL_422" samples per pixel == 3
	readDicom_DataElementData(dde, &imgfd, swaptype);
	if(dde->storageType == psystring) {
	  photometric_interpretation = dde->data;
	  if(photometric_interpretation != NULL) TRIMSTRING(photometric_interpretation);
	}
	break;
      case RESCALE_INTERCEPT:
      case RESCALE_TYPE:
      default:
	if(dde->dataLength < 1024) readDicom_DataElementData(dde, &imgfd, swaptype);
	else seekbytes = dde->dataLength;
      }
      if(seekbytes > 0) imgfd.seekg(seekbytes, ios::cur);
    }
    if(imgfd.eof()) {
      moreElements = 0;
      imgfd.clear(); // not really an error
    }
  }

  if(rawData_dde != NULL) {
    // may be OW or OB - how to know?
    // base OW or OB on bitsAllocated or dimensions
    if(bits_allocated <=0) ; // not found
    else if(bits_allocated <= 8) rawData_dde->storageType=psychar; // OB
    else if(bits_allocated <= 16) rawData_dde->storageType=psyshort; // OW

    idim = samplesPerPixel;
    if(photometric_interpretation != NULL) {
      if(strcmp("RGB", photometric_interpretation) == 0) {
	idim = 1;
	rawData_dde->storageType = psyrgb;
	if(samplesPerPixel != 3) {
	  cerr<<"warning - found RGB photometric and ignoring samples per pixel (="<<samplesPerPixel<< ") not equal to 3\n";
	}
      }
      else if(strcmp("ARGB", photometric_interpretation) == 0) {
	idim = 1;
	rawData_dde->storageType = psyargb;
	if(samplesPerPixel != 4) {
	  cerr<<"warning - found ARGB photometric and ignoring samples per pixel (="<<samplesPerPixel<< ") not equal to 4\n";
	}
      }
      else if(strcmp("MONOCHROME1", photometric_interpretation) == 0) {
	idim = 1;
	if(samplesPerPixel != 1) {
	  cerr<<"warning - found MONOCHROME1 photometric and ignoring samples per pixel (="<<samplesPerPixel<< ") not equal to 1\n";
	}
      }
      else if(strcmp("MONOCHROME2", photometric_interpretation) == 0) {
	idim = 1;
	if(samplesPerPixel != 1) {
	  cerr<<"warning - found MONOCHROME2 photometric and ignoring samples per pixel (="<<samplesPerPixel<< ") not equal to 1\n";
	}
      }
      else {
	cerr<<"warning - ignoring non-implemented photometric interpretation=\""<<photometric_interpretation<< "\"\n";
      }
    }

    initpsyimg(xdim, ydim, zdim, idim, rawData_dde->storageType, 0, 0, 0, 0,
	       rawData_dde->dataFileOffset, xres, yres, zres, ires, wres);

    // jtlee 04/02/13 try to determine transverse/sagittal/coronal
    if(imageOrientationPatientFound) {
	psyorient localorient=psynoorient;
	if( (fabs(zRowCosine) < 0.1) && (fabs(zColumnCosine) < 0.1) ) localorient=psytransverse;
	else if( (fabs(xRowCosine) < 0.1) && (fabs(xColumnCosine) < 0.1) ) localorient=psysagittal;
	else if( (fabs(yRowCosine) < 0.1) && (fabs(yColumnCosine) < 0.1) ) localorient=psycoronal;
	if(localorient != psynoorient) setorient(localorient);
    }

    if(idim != 1) {
      cerr<<"warning - samples per pixel (="<<samplesPerPixel<< ") not equal to 1\n";
      cerr<<"separating samples across i dimension\n";
      cerr<<"new dimensions=("<<xdim<<","<<ydim<<","<<zdim<<","<<idim<<")\n";
      int xinc, yinc, zinc, iinc;
      int xend, yend, zend, iend;
      getinc(&xinc, &yinc, &zinc, &iinc);
      getend(&xend, &yend, &zend, &iend);
      setinc(xinc*idim, yinc*idim, zinc*idim, 1);
      setend(xend*idim, yend*idim, zend*idim, iend);
    }
  }
  else if(combinedFileSetPtr != NULL) {
    combinedFileSetPtr->getsize(&xdim, &ydim, &zdim, &idim);
    combinedFileSetPtr->getres(&xres, &yres, &zres, &ires);
    // jtlee 04/03/13 changed to reflect the fact that zres is actually between slice res
    //   for saggital or coronal this may reflect a change in x or y image position
    //   also note orientation transverse/sagittal/coronal if not set already
    psyorient localorient=psytransverse;
    if(firstImagePositionFound && secondImagePositionFound) {
      double localres = fabs(secondImageZPosition - firstImageZPosition) * 1e-3;
      double restmp = fabs(secondImageYPosition - firstImageYPosition) * 1e-3;
      if(restmp > localres) {
	localres = restmp;
	localorient = psysagittal;
      }
      restmp = fabs(secondImageXPosition - firstImageXPosition) * 1e-3;
      if(restmp > localres) {
	localres = restmp;
	localorient = psycoronal;
      }
      if(zres == 1) zres = localres;
      if(combinedFileSetPtr->getorient() != psynoorient) localorient=combinedFileSetPtr->getorient();
    }
    wres = combinedFileSetPtr->getwordres();
    initpsyimg(xdim, ydim, zdim, idim, combinedFileSetPtr->gettype(), 0, 0, 0, 0,
	       0, xres, yres, zres, ires, wres);
    setorient(localorient);
  }
  else {
    cerr <<"\ndicomfile::dicomfile:  file missing pixel data\n";
//    exit(1);
  }
}

void dicomfile::copyblock(char *outbuff, int xorig, int yorig, int zorig,
			  int iorig, int xend, int yend, int zend, int iend,
			  int xinc, int yinc, int zinc, int iinc,
			  psytype pixeltype) {
  if(combinedFileSetPtr != NULL)
    combinedFileSetPtr->copyblock(outbuff, xorig, yorig, zorig, iorig,
				  xend, yend, zend, iend,
				  xinc, yinc, zinc, iinc, pixeltype);
  else rawfile::copyblock(outbuff, xorig, yorig, zorig, iorig,
			  xend, yend, zend, iend,
			  xinc, yinc, zinc, iinc, pixeltype);
}

void dicomfile::initgetpixel(psytype pixeltype, int x, int y, int z, int i) {
  if(combinedFileSetPtr != NULL) combinedFileSetPtr->initgetpixel(pixeltype, x, y, z, i);
  else rawfile::initgetpixel(pixeltype, x, y, z, i);
}

void dicomfile::getpixel(char *pixel, psytype pixeltype,
			 int x, int y, int z, int i) {
  if(combinedFileSetPtr != NULL) combinedFileSetPtr->getpixel(pixel, pixeltype, x, y, z, i);
  else rawfile::getpixel(pixel, pixeltype, x, y, z, i);
}

void dicomfile::getnextpixel(char *pixel) {
  if(combinedFileSetPtr != NULL) combinedFileSetPtr->getnextpixel(pixel);
  else rawfile::getnextpixel(pixel);
}

void dicomfile::showhdr(ostream *out, show_mode showMode) {
  int file_set=0;

  for(DICOM_DataElement *dde = firstDataElement; dde != NULL; dde = dde->link) {
    dde->show(out, showMode);
    *out<<'\n';
    if(dde->tag->getCombinedGroupAndElement() == FILE_SET_ID) file_set=1; 
  }

  if(file_set) {
    DICOM_DataElement *series_number_dde=NULL;
    DICOM_DataElement *series_instance_UID_dde=NULL;
    DICOM_DataElement *file_id_dde=NULL;
    DICOM_DataElement *directory_record_type_dde=NULL;
    DICOM_DataElement *image_number_dde=NULL;
    DICOM_DataElement *image_position_dde=NULL;
    DICOM_DataElement *temporal_position_identifier_dde=NULL;
    double offset_next_directory_record = -1;

    for(DICOM_DataElement *dde = firstDataElement; dde != NULL; dde = dde->link) {
      switch(dde->tag->getCombinedGroupAndElement()) {
      case ITEM:
	// jtlee 03/29/2013
	//  offset_next_directory_record != 0 test added
	//   on latest V00001 and V00001 studies offset_next_directory_record=0 for last image of series
	//   and only clear end is an ITEM tag followed by new OFFSET_NEXT_DIRECTORY_RECORD tag or end of tags.
	// jtlee 10/22/2012
	// offset_next_directory_record test added because Philips Imaging fMRI had multiple ITEMS under
	// each directory record with image position, temporal position etc. under different ITEMS
	if((file_id_dde != NULL && image_number_dde != NULL) && (offset_next_directory_record != 0) && (dde->fileOffset >= offset_next_directory_record)) {
	  *out<<"FS_INFO:";
	  if(series_number_dde != NULL)
	    { *out<<" SERIES "; show_dde_value(series_number_dde, out); }
	  if(image_number_dde != NULL)
	    { *out<<" IMAGE "; show_dde_value(image_number_dde, out); }
	  *out<<" FILE "; show_dde_value(file_id_dde, out);
	  if(series_instance_UID_dde != NULL)
	    { *out<<" SERIES_INSTANCE_UID "; show_dde_value(series_instance_UID_dde, out); }
	  if(image_position_dde != NULL)
	    { *out<<" POSITION "; show_dde_value(image_position_dde, out); }
	  if(temporal_position_identifier_dde != NULL)
	    { *out<<" TEMPORAL_POSITION_IDENTIFIER "; show_dde_value(temporal_position_identifier_dde, out); }
	  if(directory_record_type_dde != NULL)
	    { *out<<" DIRECTORY_RECORD_TYPE "; show_dde_value(directory_record_type_dde, out); }
	  *out<<" \n";
	  // series_number_dde=NULL;
	  file_id_dde=NULL;
	  image_number_dde=NULL;
	}
	break;
      case FILE_SET_ID:
	*out<<"FS_INFO: FILE_SET_ID: ";
	show_dde_value(dde, out);
	*out<<'\n';
	break;
      case OFFSET_NEXT_DIRECTORY_RECORD:
	// jtlee 03/29/2013
	//   on latest V00001 and V00001 studies offset_next_directory_record=0 for last image of series
	//   and only clear end is an ITEM tag followed by new OFFSET_NEXT_DIRECTORY_RECORD tag or end of file.
	//   so new test above under case ITEM and rely on this or falling out of loop for these bad offset values
	if(file_id_dde != NULL) {
	  *out<<"FS_INFO:";
	  if(series_number_dde != NULL)
	    { *out<<" SERIES "; show_dde_value(series_number_dde, out); }
	  if(image_number_dde != NULL)
	    { *out<<" IMAGE "; show_dde_value(image_number_dde, out); }
	  *out<<" FILE "; show_dde_value(file_id_dde, out);
	  if(series_instance_UID_dde != NULL)
	    { *out<<" SERIES_INSTANCE_UID "; show_dde_value(series_instance_UID_dde, out); }
	  if(image_position_dde != NULL)
	    { *out<<" POSITION "; show_dde_value(image_position_dde, out); }
	  if(temporal_position_identifier_dde != NULL)
	    { *out<<" TEMPORAL_POSITION_IDENTIFIER "; show_dde_value(temporal_position_identifier_dde, out); }
	  if(directory_record_type_dde != NULL)
	    { *out<<" DIRECTORY_RECORD_TYPE "; show_dde_value(directory_record_type_dde, out); }
	  *out<<" \n";
	  // series_number_dde=NULL;
	  file_id_dde=NULL;
	  image_number_dde=NULL;
	}
	type2double(dde->data, dde->storageType, (char *) &offset_next_directory_record);
	break;
      case DIRECTORY_RECORD_TYPE:
	directory_record_type_dde = dde;
	break;
      case PATIENT_NAME:
	*out<<"FS_INFO: PATIENT_NAME: ";
	show_dde_value(dde, out);
	*out<<'\n';
	break;
      case PATIENT_ID:
	*out<<"FS_INFO: PATIENT_ID: ";
	show_dde_value(dde, out);
	*out<<'\n';
	break;
      case SERIES_NUMBER:
	series_number_dde = dde;
	break;
      case SERIES_INSTANCE_UID:
	series_instance_UID_dde = dde;
	break;
      case SERIES_DESCRIPTION:
	*out<<"FS_INFO: SERIES_DESCRIPTION: ";
	show_dde_value(dde, out);
	*out<<'\n';
	break;
      case IMAGE_NUMBER:
	image_number_dde = dde;
	break;
      case IMAGE_POSITION:
	image_position_dde = dde;
	break;
      case TEMPORAL_POSITION_IDENTIFIER:
	temporal_position_identifier_dde = dde;
	break;
      case REFERENCED_FILE_ID:
	file_id_dde = dde;
	break;
      default:
	break;
      }
    }
    if(file_id_dde != NULL) {
      *out<<"FS_INFO:";
      if(series_number_dde != NULL)
	{ *out<<" SERIES "; show_dde_value(series_number_dde, out); }
      if(image_number_dde != NULL)
	{ *out<<" IMAGE "; show_dde_value(image_number_dde, out); }
      *out<<" FILE "; show_dde_value(file_id_dde, out);
      if(series_instance_UID_dde != NULL)
	{ *out<<" SERIES_INSTANCE_UID "; show_dde_value(series_instance_UID_dde, out); }
      if(image_position_dde != NULL)
	{ *out<<" POSITION "; show_dde_value(image_position_dde, out); }
      if(temporal_position_identifier_dde != NULL)
	{ *out<<" TEMPORAL_POSITION_IDENTIFIER"; show_dde_value(temporal_position_identifier_dde, out); }
      if(directory_record_type_dde != NULL)
	{ *out<<" DIRECTORY_RECORD_TYPE "; show_dde_value(directory_record_type_dde, out); }
      *out<<" \n";
    }
  } // end if(file_set)
}

