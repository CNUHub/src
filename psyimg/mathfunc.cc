#include "psyhdr.h"

// 3 value array functions

xyzdouble operator + (xyzdouble l, xyzdouble r)
{
  xyzdouble result;
  result.x = l.x + r.x;
  result.y = l.y + r.y;
  result.z = l.z + r.z;
  return(result);
}

xyzdouble operator + (xyzdouble l, double r)
{
  xyzdouble result;
  result.x = l.x + r;
  result.y = l.y + r;
  result.z = l.z + r;
  return(result);
}

xyzdouble operator - (xyzdouble l, xyzdouble r)
{
  xyzdouble result;
  result.x = l.x - r.x;
  result.y = l.y - r.y;
  result.z = l.z - r.z;
  return(result);
}

xyzdouble operator * (xyzdouble l, xyzdouble r)
{
  xyzdouble result;
  result.x = l.x * r.x;
  result.y = l.y * r.y;
  result.z = l.z * r.z;
  return(result);
}

xyzdouble operator / (xyzdouble l, xyzdouble r)
{
  xyzdouble result;
  result.x = l.x / r.x;
  result.y = l.y / r.y;
  result.z = l.z / r.z;
  return(result);
}

xyzdouble operator * (xyzdouble l, double r)
{
  xyzdouble result;
  result.x = l.x * r;
  result.y = l.y * r;
  result.z = l.z * r;
  return(result);
}

double magnitude(xyzdouble p)
{
  return(sqrt(p.x*p.x + p.y*p.y + p.z*p.z));
}

xyzdouble xyzint2double(xyzint in)
{
  xyzdouble out;
  out.x=in.x; out.y=in.y; out.z=in.z;
  return(out);
}

xyzint xyzdouble2int(xyzdouble in) // truncates no rounding
{
  xyzint out;
  out.x=(int)in.x; out.y=(int)in.y; out.z=(int)in.z;
  return(out);
}

xyzint lesserxyzdouble2int(xyzdouble in) // truncates towards lesser numbers
{
  xyzint out;
  out.x = (int)in.x; if(in.x < ((double)out.x)) (out.x)--;
  out.y = (int)in.y; if(in.y < ((double)out.y)) (out.y)--;
  out.z = (int)in.z; if(in.z < ((double)out.z)) (out.z)--;
  return(out);
}

xyzint greaterxyzdouble2int(xyzdouble in) // truncates towards greater numbers
{
  xyzint out;
  out.x = (int)in.x; if(in.x > ((double)out.x)) (out.x)++;
  out.y = (int)in.y; if(in.y > ((double)out.y)) (out.y)++;
  out.z = (int)in.z; if(in.z > ((double)out.z)) (out.z)++;
  return(out);
}

xyzint roundxyzdouble2int(xyzdouble in)
{
  xyzint out;
  out.x=irint(in.x); out.y=irint(in.y); out.z=irint(in.z);
  return(out);
}

// 4 value array functions

xyzidouble operator + (xyzidouble l, xyzidouble r)
{
  xyzidouble result;
  result.x = l.x + r.x;
  result.y = l.y + r.y;
  result.z = l.z + r.z;
  result.i = l.i + r.i;
  return(result);
}

xyzidouble operator + (xyzidouble l, double r)
{
  xyzidouble result;
  result.x = l.x + r;
  result.y = l.y + r;
  result.z = l.z + r;
  result.i = l.i + r;
  return(result);
}

xyzidouble operator - (xyzidouble l, xyzidouble r)
{
  xyzidouble result;
  result.x = l.x - r.x;
  result.y = l.y - r.y;
  result.z = l.z - r.z;
  result.i = l.i - r.i;
  return(result);
}

xyzidouble operator * (xyzidouble l, xyzidouble r)
{
  xyzidouble result;
  result.x = l.x * r.x;
  result.y = l.y * r.y;
  result.z = l.z * r.z;
  result.i = l.i * r.i;
  return(result);
}

xyzidouble operator / (xyzidouble l, xyzidouble r)
{
  xyzidouble result;
  result.x = l.x / r.x;
  result.y = l.y / r.y;
  result.z = l.z / r.z;
  result.i = l.i / r.i;
  return(result);
}

xyzidouble operator * (xyzidouble l, double r)
{
  xyzidouble result;
  result.x = l.x * r;
  result.y = l.y * r;
  result.z = l.z * r;
  result.i = l.i * r;
  return(result);
}

double magnitude(xyzidouble p)
{
  return(sqrt(p.x*p.x + p.y*p.y + p.z*p.z + p.i*p.i));
}

xyzidouble xyziint2double(xyziint in)
{
  xyzidouble out;
  out.x=in.x; out.y=in.y; out.z=in.z; out.i=in.i;
  return(out);
}

xyziint xyzidouble2int(xyzidouble in) // truncates no rounding
{
  xyziint out;
  out.x=(int)in.x; out.y=(int)in.y; out.z=(int)in.z; out.i=(int)in.i;
  return(out);
}

xyziint lesserxyzidouble2int(xyzidouble in) // truncates towards lesser numbers
{
  xyziint out;
  out.x = (int)in.x; if(in.x < ((double)out.x)) (out.x)--;
  out.y = (int)in.y; if(in.y < ((double)out.y)) (out.y)--;
  out.z = (int)in.z; if(in.z < ((double)out.z)) (out.z)--;
  out.i = (int)in.i; if(in.i < ((double)out.i)) (out.i)--;
  return(out);
}

xyziint greaterxyzidouble2int(xyzidouble in) // truncates towards greater numbers
{
  xyziint out;
  out.x = (int)in.x; if(in.x > ((double)out.x)) (out.x)++;
  out.y = (int)in.y; if(in.y > ((double)out.y)) (out.y)++;
  out.z = (int)in.z; if(in.z > ((double)out.z)) (out.z)++;
  out.i = (int)in.i; if(in.i > ((double)out.i)) (out.i)++;
  return(out);
}

xyziint roundxyzidouble2int(xyzidouble in)
{
  xyziint out;
  out.x=irint(in.x); out.y=irint(in.y); out.z=irint(in.z); out.i=irint(in.i);
  return(out);
}

xyziint operator + (xyziint l, xyziint r)
{
  xyziint result;
  result.x = l.x + r.x;
  result.y = l.y + r.y;
  result.z = l.z + r.z;
  result.i = l.i + r.i;
  return(result);
}

xyziint operator + (xyziint l, int r)
{
  xyziint result;
  result.x = l.x + r;
  result.y = l.y + r;
  result.z = l.z + r;
  result.i = l.i + r;
  return(result);
}

xyziint operator - (xyziint l, xyziint r)
{
  xyziint result;
  result.x = l.x - r.x;
  result.y = l.y - r.y;
  result.z = l.z - r.z;
  result.i = l.i - r.i;
  return(result);
}

xyzint operator + (xyzint l, xyzint r)
{
  xyzint result;
  result.x = l.x + r.x;
  result.y = l.y + r.y;
  result.z = l.z + r.z;
  return(result);
}

xyzint operator + (xyzint l, int r)
{
  xyzint result;
  result.x = l.x + r;
  result.y = l.y + r;
  result.z = l.z + r;
  return(result);
}

xyzint operator - (xyzint l, xyzint r)
{
  xyzint result;
  result.x = l.x - r.x;
  result.y = l.y - r.y;
  result.z = l.z - r.z;
  return(result);
}

matrix4X4::matrix4X4(init_matrix_type mtype) { reset(mtype); }

matrix4X4::matrix4X4(matrix4X4 *duplicate_mat) { reset(duplicate_mat); }

void matrix4X4::reset(init_matrix_type mtype) {
  for(int i=0; i<16; i++)a[i]=0;
  if(mtype == Identity)aa[0][0]=aa[1][1]=aa[2][2]=aa[3][3]=1;
}

void matrix4X4::reset(matrix4X4 *duplicate_mat) {
  if(duplicate_mat != NULL)for(int i=0; i<16; i++)a[i]=duplicate_mat->a[i];
  else reset(Zero);
}

void dispmatrix4X4(const char *name, matrix4X4 *m)
{
  cout<<name<<" =\n";
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      cout<<" \t"<< setw(10) << setprecision(8) <<m->aa[i][j];
    }
    cout<<'\n';
  }
}

matrix4X4 operator * (double l, matrix4X4 r)
{
  matrix4X4 result;
  for(int i=0; i<16; i++)result.a[i]= l * r.a[i];
  return result;
}

matrix4X4 operator + (matrix4X4 l, matrix4X4 r)
{
  matrix4X4 result;
  for(int i=0; i<16; i++)result.a[i]= l.a[i] + r.a[i];
  return result;
}

matrix4X4 operator - (matrix4X4 l, matrix4X4 r)
{
  matrix4X4 result;
  for(int i=0; i<16; i++)result.a[i]= l.a[i] - r.a[i];
  return result;
}

xyzdouble operator * (matrix4X4 l, xyzdouble r)
{
  xyzdouble result;

  result.x = (l.m.m11 * r.x) + (l.m.m12 * r.y) + (l.m.m13 * r.z) + l.m.m14;
  result.y = (l.m.m21 * r.x) + (l.m.m22 * r.y) + (l.m.m23 * r.z) + l.m.m24;
  result.z = (l.m.m31 * r.x) + (l.m.m32 * r.y) + (l.m.m33 * r.z) + l.m.m34;

  return result;
}
xyzidouble operator * (matrix4X4 l, xyzidouble r)
{
  xyzidouble result;

  result.x = (l.m.m11 * r.x) + (l.m.m12 * r.y) +
    (l.m.m13 * r.z) + (l.m.m14);
  result.y = (l.m.m21 * r.x) + (l.m.m22 * r.y) +
    (l.m.m23 * r.z) + (l.m.m24);
  result.z = (l.m.m31 * r.x) + (l.m.m32 * r.y) +
    (l.m.m33 * r.z) + (l.m.m34);
  result.i = r.i;

  return result;
}

matrix4X4 operator * (matrix4X4 l, matrix4X4 r)
{
  matrix4X4 result;

  result.m.m11 = (l.m.m11 * r.m.m11) + (l.m.m12 * r.m.m21) +
    (l.m.m13 * r.m.m31) + (l.m.m14 * r.m.m41);
  result.m.m12 = (l.m.m11 * r.m.m12) + (l.m.m12 * r.m.m22) +
    (l.m.m13 * r.m.m32) + (l.m.m14 * r.m.m42);
  result.m.m13 = (l.m.m11 * r.m.m13) + (l.m.m12 * r.m.m23) +
    (l.m.m13 * r.m.m33) + (l.m.m14 * r.m.m43);
  result.m.m14 = (l.m.m11 * r.m.m14) + (l.m.m12 * r.m.m24) +
    (l.m.m13 * r.m.m34) + (l.m.m14 * r.m.m44);

  result.m.m21 = (l.m.m21 * r.m.m11) + (l.m.m22 * r.m.m21) +
    (l.m.m23 * r.m.m31) + (l.m.m24 * r.m.m41);
  result.m.m22 = (l.m.m21 * r.m.m12) + (l.m.m22 * r.m.m22) +
    (l.m.m23 * r.m.m32) + (l.m.m24 * r.m.m42);
  result.m.m23 = (l.m.m21 * r.m.m13) + (l.m.m22 * r.m.m23) +
    (l.m.m23 * r.m.m33) + (l.m.m24 * r.m.m43);
  result.m.m24 = (l.m.m21 * r.m.m14) + (l.m.m22 * r.m.m24) +
    (l.m.m23 * r.m.m34) + (l.m.m24 * r.m.m44);

  result.m.m31 = (l.m.m31 * r.m.m11) + (l.m.m32 * r.m.m21) +
    (l.m.m33 * r.m.m31) + (l.m.m34 * r.m.m41);
  result.m.m32 = (l.m.m31 * r.m.m12) + (l.m.m32 * r.m.m22) +
    (l.m.m33 * r.m.m32) + (l.m.m34 * r.m.m42);
  result.m.m33 = (l.m.m31 * r.m.m13) + (l.m.m32 * r.m.m23) +
    (l.m.m33 * r.m.m33) + (l.m.m34 * r.m.m43);
  result.m.m34 = (l.m.m31 * r.m.m14) + (l.m.m32 * r.m.m24) +
    (l.m.m33 * r.m.m34) + (l.m.m34 * r.m.m44);

  result.m.m41 = (l.m.m41 * r.m.m11) + (l.m.m42 * r.m.m21) +
    (l.m.m43 * r.m.m31) + (l.m.m44 * r.m.m41);
  result.m.m42 = (l.m.m41 * r.m.m12) + (l.m.m42 * r.m.m22) +
    (l.m.m43 * r.m.m32) + (l.m.m44 * r.m.m42);
  result.m.m43 = (l.m.m41 * r.m.m13) + (l.m.m42 * r.m.m23) +
    (l.m.m43 * r.m.m33) + (l.m.m44 * r.m.m43);
  result.m.m44 = (l.m.m41 * r.m.m14) + (l.m.m42 * r.m.m24) +
    (l.m.m43 * r.m.m34) + (l.m.m44 * r.m.m44);

  return result;
}
