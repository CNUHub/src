#include "psyhdr.h"
#include <limits>

#define NONEG(i) (((i)<0)? 0:(i))
#define COMBINEDTYPE(in,out) (((in)<<8)+(out))
// to char
inline void char2char(char *in, char *out)
{ *(signed char *)out=*(signed char *)in; }
inline void uchar2char(char *in, char *out)
{ *(signed char *)out= *(unsigned char *)in; }
inline void short2char(char *in, char *out)
{ *(signed char *)out= *(short *)in; }
inline void ushort2char(char *in, char *out)
{ *(signed char *)out= *(unsigned short *)in; }
inline void int2char(char *in, char *out)
{ *(signed char *)out= *(int *)in; }
inline void float2char(char *in, char *out)
{ *(signed char *)out= irint(*(float *)in); }
inline void complex2char(char *in, char *out)
{ *(signed char *)out= irint(*(float *)in); }
inline void double2char(char *in, char *out)
{ *(signed char *)out= irint(*(double *)in); }
// to unsigned char
inline void char2uchar(char *in, char *out)
{ *(unsigned char *)out=NONEG(*(signed char *)in); }
inline void uchar2uchar(char *in, char *out)
{ *(unsigned char *)out= *(unsigned char *)in; }
inline void short2uchar(char *in, char *out)
{ *(unsigned char *)out= NONEG(*(short *)in); }
inline void ushort2uchar(char *in, char *out)
{ *(unsigned char *)out= *(unsigned short *)in; }
inline void int2uchar(char *in, char *out)
{ *(unsigned char *)out= NONEG(*(int *)in); }
inline void float2uchar(char *in, char *out)
{ int i=irint(*(float *)in);
  *(unsigned char *)out= NONEG(i); }
inline void complex2uchar(char *in, char *out)
{ int i=irint(*(float *)in);
  *(unsigned char *)out= NONEG(i); }
inline void double2uchar(char *in, char *out)
{ int i=irint(*(double *)in);
  *(unsigned char *)out= NONEG(i); }
// to short
inline void char2short(char *in, char *out)
{ *(short *)out= *(signed char *)in; }
inline void uchar2short(char *in, char *out)
{ *(short *)out= *(unsigned char *)in; }
inline void short2short(char *in, char *out)
{ *(short *)out= *(short *)in; }
inline void ushort2short(char *in, char *out)
{ *(short *)out= *(unsigned short *)in; }
inline void shortsw2short(char *in, char *out)
// tmp used to allow in place conversion
{ char tmp; tmp=in[0]; out[0]=in[1]; out[1]=tmp;}
inline void int2short(char *in, char *out)
{ *(short *)out= *(int *)in; }
inline void float2short(char *in, char *out)
{ *(short *)out= irint(*(float *)in); }
inline void complex2short(char *in, char *out)
{ *(short *)out= irint(*(float *)in); }
inline void double2short(char *in, char *out)
{ *(short *)out= irint(*(double *)in); }
// to unsigned short
inline void char2ushort(char *in, char *out)
{ *(unsigned short *)out=NONEG(*(signed char *)in); }
inline void uchar2ushort(char *in, char *out)
{ *(unsigned short *)out= *(unsigned char *)in; }
inline void short2ushort(char *in, char *out)
{ *(unsigned short *)out= NONEG(*(short *)in); }
inline void ushort2ushort(char *in, char *out)
{ *(unsigned short *)out= *(unsigned short *)in; }
inline void int2ushort(char *in, char *out)
{ *(unsigned short *)out= NONEG(*(int *)in); }
inline void float2ushort(char *in, char *out)
{ int i=irint(*(float *)in);
  *(unsigned short *)out= NONEG(i); }
inline void complex2ushort(char *in, char *out)
{ int i=irint(*(float *)in);
  *(unsigned short *)out= NONEG(i); }
inline void double2ushort(char *in, char *out)
{ int i=irint(*(double *)in);
  *(unsigned short *)out= NONEG(i); }
// to int
inline void char2int(char *in, char *out)
{ *(int *)out=*(signed char *)in; }
inline void uchar2int(char *in, char *out)
{ *(int *)out= *(unsigned char *)in; }
inline void short2int(char *in, char *out)
{ *(int *)out= *(short *)in; }
inline void ushort2int(char *in, char *out)
{ *(int *)out= *(unsigned short *)in; }
inline void int2int(char *in, char *out)
{ *(int *)out= *(int *)in; }
inline void float2int(char *in, char *out)
{ *(int *)out= irint(*(float *)in); }
inline void complex2int(char *in, char *out)
{ *(int *)out= irint(*(float *)in); }
inline void double2int(char *in, char *out)
{ *(int *)out= irint(*(double *)in); }
inline void string2int(char *in, char *out)
{ *(int *)out= atoi(in); }
inline void rgb2int(char *in, char *out)
{*(unsigned char *)(out++)=255; *(out++)=*(in++), *(out++)=*(in++), *(out++)=*(in++);}
inline void argb2int(char *in, char *out)
{*(int *)out=*(int *)in;}
// to float
inline void char2float(char *in, char *out)
{ *(float *)out=*(signed char *)in; }
inline void uchar2float(char *in, char *out)
{ *(float *)out= *(unsigned char *)in; }
inline void short2float(char *in, char *out)
{ *(float *)out= *(short *)in; }
inline void ushort2float(char *in, char *out)
{ *(float *)out= *(unsigned short *)in; }
inline void int2float(char *in, char *out)
{ *(float *)out= *(int *)in; }
inline void float2float(char *in, char *out)
{ *(float *)out= *(float *)in; }
inline void complex2float(char *in, char *out)
{ *(float *)out= *(float *)in; }
inline void double2float(char *in, char *out)
{ *(float *)out= *(double *)in; }
inline void string2float(char *in, char *out)
{ *(float *)out= atof(in); }
// to complex
inline void char2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0]= *(signed char *)in; ofptr[1]=0.0; }
inline void uchar2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0]= *(unsigned char *)in; ofptr[1]=0.0; }
inline void short2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0]= *(short *)in; ofptr[1]=0.0; }
inline void ushort2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0]= *(unsigned short *)in; ofptr[1]=0.0; }
inline void int2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0]= *(int *)in; ofptr[1]=0.0; }
inline void float2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0]= *(float *)in; ofptr[1]=0.0; }
inline void complex2complex(char *in, char *out)
{ float *ofptr=(float *)out; float *ifptr=(float *)in;
  ofptr[0]=ifptr[0]; ofptr[1]=ifptr[1]; }
inline void double2complex(char *in, char *out)
{ float *ofptr=(float *)out;
  ofptr[0] = *(double *)in; ofptr[1]=0.0; }
// to double
inline void char2double(char *in, char *out)
{ *(double *)out=*(signed char *)in; }
inline void uchar2double(char *in, char *out)
{ *(double *)out= *(unsigned char *)in; }
inline void short2double(char *in, char *out)
{ *(double *)out= *(short *)in; }
inline void ushort2double(char *in, char *out)
{ *(double *)out= *(unsigned short *)in; }
inline void int2double(char *in, char *out)
{ *(double *)out= *(int *)in; }
inline void uint2double(char *in, char *out)
{ *(double *)out= *(unsigned int *)in; }
inline void float2double(char *in, char *out)
{ *(double *)out= *(float *)in; }
inline void complex2double(char *in, char *out)
{ *(double *)out= *(float *)in; }
inline void double2double(char *in, char *out)
{ *(double *)out= *(double *)in; }
inline void string2double(char *in, char *out)
{ *(double *)out= atof(in); }

int gettypebytes(psytype type)
{
  switch(type) {
  case psystring:
  case psychar:
    return(sizeof(char));
  case psyuchar:
    return(sizeof(unsigned char));
  case psyshort:
    return(sizeof(short));
  case psyushort:
    return(sizeof(unsigned short));
  case psyshortsw:
    return(sizeof(short));
  case psyint:
    return(sizeof(int));
  case psyuint:
    return(sizeof(unsigned int));
  case psyfloat:
    return(sizeof(float));
  case psycomplex:
    return(2*sizeof(float));
  case psydouble:
    return(sizeof(double));
  case psyrgb:
    return(3);
  case psyargb:
    return(4);
  default:
    break;
  }
  cerr<<"gettypebytes - unknown pixel type = "<<type<<"("<<psytypenames[type]<<")"<<'\n';
  exit(1);
}

int istypesigned(psytype type)
{
  switch(type) {
  case psystring:
  case psychar:
  case psyshort:
  case psyint:
  case psyshortsw:
  case psyfloat:
  case psydouble:
  case psycomplex:
    return(1);
  case psyuchar:
  case psyushort:
  case psyuint:
  case psyrgb:
  case psyargb:
    return(0);
  default:
    break;
  }
  cerr<<"istypesigned - unknown pixel type = "<<type<<"("<<psytypenames[type]<<")"<<'\n';
  exit(1);
}

double min_value_for_type(psytype type) {
  switch(type) {
  case psychar:
    return std::numeric_limits<char>::min();
  case psyuchar:
  case psyushort:
  case psyuint:
    return 0;
  case psyshort:
    return std::numeric_limits<short>::min();
  case psyint:
    return std::numeric_limits<int>::min();
  case psyfloat:
    return -std::numeric_limits<float>::max();
  case psydouble:
    return -std::numeric_limits<double>::max();
  default:
    cerr<<"min_value_for_type - minimum unknown for type=" <<type<<"("<<psytypenames[type]<<")\n";
    exit(1);
  }
}

double max_value_for_type(psytype type) {
  switch(type) {
  case psychar:
    return std::numeric_limits<char>::max();
  case psyuchar:
    return std::numeric_limits<unsigned char>::max();
  case psyushort:
    return std::numeric_limits<unsigned short>::max();
  case psyuint:
    return std::numeric_limits<unsigned int>::max();
  case psyshort:
    return std::numeric_limits<short>::max();
  case psyint:
    return std::numeric_limits<int>::max();
  case psyfloat:
    return std::numeric_limits<float>::max();
  case psydouble:
    return std::numeric_limits<double>::max();
  default:
    cerr<<"max_value_for_type - maximum unknown for type=" <<type<<"("<<psytypenames[type]<<")\n";
    exit(1);
  }
}

void type2char(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2char(in, out); break;
  case psyuchar:
    uchar2char(in, out); break;
  case psyshort:
    short2char(in, out); break;
  case psyushort:
    ushort2char(in, out); break;
  case psyint:
    int2char(in, out); break;
  case psyfloat:
    float2char(in, out); break;
  case psycomplex:
    complex2char(in, out); break;
  case psydouble:
    double2char(in, out); break;
  default:
    cerr<<"type2char - no conversion from in type=" <<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2uchar(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2uchar(in, out); break;
  case psyuchar:
    uchar2uchar(in, out); break;
  case psyshort:
    short2uchar(in, out); break;
  case psyushort:
    ushort2uchar(in, out); break;
  case psyint:
    int2uchar(in, out); break;
  case psyfloat:
    float2uchar(in, out); break;
  case psycomplex:
    complex2uchar(in, out); break;
  case psydouble:
    double2uchar(in, out); break;
  default:
    cerr<<"type2uchar - no conversion from in type=" <<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2short(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2short(in, out); break;
  case psyuchar:
    uchar2short(in, out); break;
  case psyshort:
    short2short(in, out); break;
  case psyushort:
    ushort2short(in, out); break;
  case psyshortsw:
    shortsw2short(in, out); break;
  case psyint:
    int2short(in, out); break;
  case psyfloat:
    float2short(in, out); break;
  case psycomplex:
    complex2short(in, out); break;
  case psydouble:
    double2short(in, out); break;
  default:
    cerr<<"type2short - no conversion from in type=" <<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2ushort(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2ushort(in, out); break;
  case psyuchar:
    uchar2ushort(in, out); break;
  case psyshort:
    short2ushort(in, out); break;
  case psyushort:
    ushort2ushort(in, out); break;
  case psyint:
    int2ushort(in, out); break;
  case psyfloat:
    float2ushort(in, out); break;
  case psycomplex:
    complex2ushort(in, out); break;
  case psydouble:
    double2ushort(in, out); break;
  default:
    cerr<<"type2ushort - no conversion from in type=" <<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2int(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2int(in, out); break;
  case psyuchar:
    uchar2int(in, out); break;
  case psyshort:
    short2int(in, out); break;
  case psyushort:
    ushort2int(in, out); break;
  case psyint:
    int2int(in, out); break;
  case psyfloat:
    float2int(in, out); break;
  case psycomplex:
    complex2int(in, out); break;
  case psydouble:
    double2int(in, out); break;
  case psystring:
    string2int(in, out); break;
  case psyrgb:
    rgb2int(in, out); break;
  case psyargb:
    argb2int(in, out); break;
  default:
    cerr<<"type2int - no conversion from in type=" <<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2float(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2float(in, out); break;
  case psyuchar:
    uchar2float(in, out); break;
  case psyshort:
    short2float(in, out); break;
  case psyushort:
    ushort2float(in, out); break;
  case psyint:
    int2float(in, out); break;
  case psyfloat:
    float2float(in, out); break;
  case psycomplex:
    complex2float(in, out); break;
  case psydouble:
    double2float(in, out); break;
  case psystring:
    string2float(in, out); break;
  default:
    cerr<<"type2float - no conversion from in type=" <<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2complex(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2complex(in, out); break;
  case psyuchar:
    uchar2complex(in, out); break;
  case psyshort:
    short2complex(in, out); break;
  case psyushort:
    ushort2complex(in, out); break;
  case psyint:
    int2complex(in, out); break;
  case psyfloat:
    float2complex(in, out); break;
  case psycomplex:
    complex2complex(in, out); break;
  case psydouble:
    double2complex(in, out); break;
  default:
    cerr<<"type2complex - no conversion from in type="<<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void type2double(char *in, psytype intype, char *out)
{
  switch(intype) {
  case psychar:
    char2double(in, out); break;
  case psyuchar:
    uchar2double(in, out); break;
  case psyshort:
    short2double(in, out); break;
  case psyushort:
    ushort2double(in, out); break;
  case psyint:
    int2double(in, out); break;
  case psyuint:
    uint2double(in, out); break;
  case psyfloat:
    float2double(in, out); break;
  case psycomplex:
    complex2double(in, out); break;
  case psydouble:
    double2double(in, out); break;
  case psystring:
    string2double(in, out); break;
  default:
    cerr<<"type2double - no conversion from in type="<<intype<<"("<<psytypenames[intype]<<")"<<" available\n";
    exit(1);
  }
}

void pixeltypechg(char *in, psytype in_type,
	     char *out, psytype out_type)
{
  switch (COMBINEDTYPE(in_type,out_type)) {
// to char
  case COMBINEDTYPE(psychar,psychar):
    char2char(in, out); break;
  case COMBINEDTYPE(psyuchar,psychar):
    uchar2char(in, out); break;
  case COMBINEDTYPE(psyshort,psychar):
    short2char(in, out); break;
  case COMBINEDTYPE(psyushort,psychar):
    ushort2char(in, out); break;
  case COMBINEDTYPE(psyint,psychar):
    int2char(in, out); break;
  case COMBINEDTYPE(psyfloat,psychar):
    float2char(in, out); break;
  case COMBINEDTYPE(psycomplex,psychar):
    complex2char(in, out); break;
  case COMBINEDTYPE(psydouble,psychar):
    double2char(in, out); break;
// to uchar
  case COMBINEDTYPE(psychar,psyuchar):
    char2uchar(in, out); break;
  case COMBINEDTYPE(psyuchar,psyuchar):
    uchar2uchar(in, out); break;
  case COMBINEDTYPE(psyshort,psyuchar):
    short2uchar(in, out); break;
  case COMBINEDTYPE(psyushort,psyuchar):
    ushort2uchar(in, out); break;
  case COMBINEDTYPE(psyint,psyuchar):
    int2uchar(in, out); break;
  case COMBINEDTYPE(psyfloat,psyuchar):
    float2uchar(in, out); break;
  case COMBINEDTYPE(psycomplex,psyuchar):
    complex2uchar(in, out); break;
  case COMBINEDTYPE(psydouble,psyuchar):
    double2uchar(in, out); break;
// to short
  case COMBINEDTYPE(psychar,psyshort):
    char2short(in, out); break;
  case COMBINEDTYPE(psyuchar,psyshort):
    uchar2short(in, out); break;
  case COMBINEDTYPE(psyshort,psyshort):
    short2short(in, out); break;
  case COMBINEDTYPE(psyushort,psyshort):
    ushort2short(in, out); break;
  case COMBINEDTYPE(psyshortsw,psyshort):
    shortsw2short(in, out); break;
  case COMBINEDTYPE(psyint,psyshort):
    int2short(in, out); break;
  case COMBINEDTYPE(psyfloat,psyshort):
    float2short(in, out); break;
  case COMBINEDTYPE(psycomplex,psyshort):
    complex2short(in, out); break;
  case COMBINEDTYPE(psydouble,psyshort):
    double2short(in, out); break;
// to unsigned short
  case COMBINEDTYPE(psychar,psyushort):
    char2ushort(in, out); break;
  case COMBINEDTYPE(psyuchar,psyushort):
    uchar2ushort(in, out); break;
  case COMBINEDTYPE(psyshort,psyushort):
    short2ushort(in, out); break;
  case COMBINEDTYPE(psyushort,psyushort):
    ushort2ushort(in, out); break;
  case COMBINEDTYPE(psyint,psyushort):
    int2ushort(in, out); break;
  case COMBINEDTYPE(psyfloat,psyushort):
    float2ushort(in, out); break;
  case COMBINEDTYPE(psycomplex,psyushort):
    complex2ushort(in, out); break;
  case COMBINEDTYPE(psydouble,psyushort):
    double2ushort(in, out); break;
// to int
  case COMBINEDTYPE(psychar,psyint):
    char2int(in, out); break;
  case COMBINEDTYPE(psyuchar,psyint):
    uchar2int(in, out); break;
  case COMBINEDTYPE(psyshort,psyint):
    short2int(in, out); break;
  case COMBINEDTYPE(psyushort,psyint):
    ushort2int(in, out); break;
  case COMBINEDTYPE(psyint,psyint):
    int2int(in, out); break;
  case COMBINEDTYPE(psyfloat,psyint):
    float2int(in, out); break;
  case COMBINEDTYPE(psycomplex,psyint):
    complex2int(in, out); break;
  case COMBINEDTYPE(psydouble,psyint):
    double2int(in, out); break;
  case COMBINEDTYPE(psyrgb,psyint):
    rgb2int(in, out); break;
  case COMBINEDTYPE(psyargb,psyint):
    argb2int(in, out); break;
// to float
  case COMBINEDTYPE(psychar,psyfloat):
    char2float(in, out); break;
  case COMBINEDTYPE(psyuchar,psyfloat):
    uchar2float(in, out); break;
  case COMBINEDTYPE(psyshort,psyfloat):
    short2float(in, out); break;
  case COMBINEDTYPE(psyushort,psyfloat):
    ushort2float(in, out); break;
  case COMBINEDTYPE(psyint,psyfloat):
    int2float(in, out); break;
  case COMBINEDTYPE(psyfloat,psyfloat):
    float2float(in, out); break;
  case COMBINEDTYPE(psycomplex,psyfloat):
    complex2float(in, out); break;
  case COMBINEDTYPE(psydouble,psyfloat):
    double2float(in, out); break;
// to complex
  case COMBINEDTYPE(psychar,psycomplex):
    char2complex(in, out); break;
  case COMBINEDTYPE(psyuchar,psycomplex):
    uchar2complex(in, out); break;
  case COMBINEDTYPE(psyshort,psycomplex):
    short2complex(in, out); break;
  case COMBINEDTYPE(psyushort,psycomplex):
    ushort2complex(in, out); break;
  case COMBINEDTYPE(psyint,psycomplex):
    int2complex(in, out); break;
  case COMBINEDTYPE(psyfloat,psycomplex):
    float2complex(in, out); break;
  case COMBINEDTYPE(psycomplex,psycomplex):
    complex2complex(in, out); break;
  case COMBINEDTYPE(psydouble,psycomplex):
    double2complex(in, out); break;
// to double
  case COMBINEDTYPE(psychar,psydouble):
    char2double(in, out); break;
  case COMBINEDTYPE(psyuchar,psydouble):
    uchar2double(in, out); break;
  case COMBINEDTYPE(psyshort,psydouble):
    short2double(in, out); break;
  case COMBINEDTYPE(psyushort,psydouble):
    ushort2double(in, out); break;
  case COMBINEDTYPE(psyint,psydouble):
    int2double(in, out); break;
  case COMBINEDTYPE(psyuint,psydouble):
    uint2double(in, out); break;
  case COMBINEDTYPE(psyfloat,psydouble):
    float2double(in, out); break;
  case COMBINEDTYPE(psycomplex,psydouble):
    complex2double(in, out); break;
  case COMBINEDTYPE(psydouble,psydouble):
    double2double(in, out); break;
  default:
    cerr<<"pixeltypechg - no conversion from in type="<<psytypenames[in_type];
    cerr<<" to out type="<<out_type<<"("<<psytypenames[out_type]<<")"<<" available\n";
    exit(1);
  };
}

void convertimg::convertpixel(char *in, psytype intype,
			      char *out, psytype outtype)
{
  pixeltypechg(in, intype, out, outtype);
}

void convertimg::initconvertimg(psyimg *psyimgptr, psytype pixeltype)
{
  initproc1img(psyimgptr, pixeltype);
}
