#include "psyhdr.h"

threeDtransform::threeDtransform(matrix4X4 in_matrix,  matrix4X4 *in_inverseMatrix) {
  quatern_set = 0;
  inverse_set = 0;
  setMatrix(in_matrix);
  if(in_inverseMatrix != NULL) {
    setInverseMatrix(*in_inverseMatrix);
  }
}

threeDtransform::threeDtransform(xyzidouble in_srow1, xyzidouble in_srow2, xyzidouble in_srow3) {
  quatern_set = 0;
  inverse_set = 0;
  t.rv.v1 = in_srow1; t.rv.v2 = in_srow2;  t.rv.v3 = in_srow3;
  t.rv.v4.x = t.rv.v4.y = t.rv.v4.z = 0l; t.rv.v4.i = 1.0l;
}

threeDtransform::threeDtransform(cnuquatern in_quatern) {
  quatern_set = 1;
  inverse_set = 0;
  setquatern(in_quatern, 1);
}

void threeDtransform::setMatrix(matrix4X4 in_matrix) { t = in_matrix; }

void threeDtransform::setInverseMatrix(matrix4X4 in_inverseMatrix) {
  inv = in_inverseMatrix;
  inverse_set = 1;
}

void threeDtransform::setquatern(cnuquatern in_quatern, int calcMatrix) {
  quatern=in_quatern;
  quatern_set = 1;
  if(calcMatrix) {
    double a, b, c, d;
    b = quatern.b; c = quatern.c; d = quatern.d;
    a = 1.0l - (b*b + c*c + d*d);
    if( a < 1.e-7l ){                   // special case  as in http://nifthi.nimh.nig.gov/pub/dist/src/niftilib/nifti1_io.c example
      double nf = 1.0l / sqrt(b*b+c*c+d*d); // normalize (b,c,d) vector
      b *= nf ; c *= nf ; d *= nf ;       
      a = 0.0l ;                        // a = 0 ==> 180 degree rotation
    } else{
      a = sqrt(a) ;                     // angle = 2*arccos(a)
    }
    double zf = quatern.zfactor;
    if(quatern.qfac < 0.0) {
      quatern.qfac = -1.0l;
      zf = -zf;
    }
    else quatern.qfac = 1.0l;

    t.m.m11 = (a*a+b*b-c*c-d*d) * quatern.xfactor;
    t.m.m12 = (2*b*c-2*a*d) * quatern.yfactor;
    t.m.m13 = (2*b*d+2*a*c) * zf;
    t.m.m14 = quatern.xoff;

    t.m.m21 = (2*b*c+2*a*d) * quatern.xfactor;
    t.m.m22 = (a*a+c*c-b*b-d*d) * quatern.yfactor;
    t.m.m23 = (2*c*d-2*a*b) * zf;
    t.m.m24 = quatern.yoff;

    t.m.m31 = (2*b*d-2*a*c) * quatern.xfactor;
    t.m.m32 = (2*c*d+2*a*b) * quatern.yfactor;
    t.m.m33 = (a*a+d*d-c*c-b*b) * zf;
    t.m.m34 = quatern.zoff;
    // fill last row
    t.m.m41 = t.m.m42 = t.m.m43 = 0l; t.m.m44 = 1.0l;
  }
}

void threeDtransform::buildInverseMatrix() {
  // try simple inverse calculations -- transpose -- works for rotation matrices
  // calc transpose products
  // diagonals - must be non-zero
  double p11 = t.m.m11 * t.m.m11 + t.m.m12 * t.m.m12  + t.m.m13 * t.m.m13;
  if(p11 > 1e-10) {
    double p22 = t.m.m21 * t.m.m21 + t.m.m22 * t.m.m22  + t.m.m23 * t.m.m23;
    if(p22 > 1e-10) {
      double p33 = t.m.m31 * t.m.m31 + t.m.m32 * t.m.m32  + t.m.m33 * t.m.m33;
      if(p33 > 1e-10) {
	// off diagonals - must be zero
	// lower diagonals equivalent to upper
	double p12 = t.m.m11 * t.m.m21 + t.m.m12 * t.m.m22  + t.m.m13 * t.m.m23; // same as p21
	if(p12 < 1e-20) {
	  double p13 = t.m.m11 * t.m.m31 + t.m.m12 * t.m.m32  + t.m.m13 * t.m.m33; // same as p31
	  if(p13 < 1e-20) {
	    double p23 = t.m.m21 * t.m.m31 + t.m.m22 * t.m.m32  + t.m.m23 * t.m.m33; // same as p32
	    if(p23 < 1e-20) {
	      // transpose
	      // post multiply transpose matrix with diagonal inverses
	      double col1f = 1.0l/p11; double col2f = 1.0l/p22; double col3f = 1.0l/p33;
	      inv.m.m11 = t.m.m11 * col1f; inv.m.m12 = t.m.m21 * col2f; inv.m.m13 = t.m.m31 * col3f;
	      inv.m.m21 = t.m.m12 * col1f; inv.m.m22 = t.m.m22 * col2f; inv.m.m23 = t.m.m32 * col3f;
	      inv.m.m31 = t.m.m13 * col1f; inv.m.m32 = t.m.m23 * col2f; inv.m.m33 = t.m.m33 * col3f;
	      // create column 4 translation inverses
	      inv.m.m14 = -(inv.m.m11 * t.m.m14 + inv.m.m12 * t.m.m24 + inv.m.m13 * t.m.m34);
	      inv.m.m24 = -(inv.m.m21 * t.m.m14 + inv.m.m22 * t.m.m24 + inv.m.m23 * t.m.m34);
	      inv.m.m34 = -(inv.m.m31 * t.m.m14 + inv.m.m32 * t.m.m24 + inv.m.m33 * t.m.m34);
	      // fill last row
	      inv.m.m41 = inv.m.m42 = inv.m.m43 = 0l; inv.m.m44 = 1.0l;
	      inverse_set = 1;
	    }
	  }
	}
      }
    }
  }
}

matrix4X4 threeDtransform::getMatrix() { return t; }

matrix4X4 *threeDtransform::getInverseMatrix() {
  if(! inverse_set) buildInverseMatrix();
  if(inverse_set) return new matrix4X4(&inv);
  else return NULL;
}

int threeDtransform::isQuaternSet() {
  return quatern_set;
}

cnuquatern *threeDtransform::getQuatern() {
  if(quatern_set) return new cnuquatern(quatern);
  else return NULL;
}

xyzdouble threeDtransform::to_space(xyzdouble in) {
  return t * in;
}

xyzidouble threeDtransform::getRow1() { return t.rv.v1; }
xyzidouble threeDtransform::getRow2() { return t.rv.v2; }
xyzidouble threeDtransform::getRow3() { return t.rv.v3; }

threeDtransform* threeDtransform::getInverseTransform() {
  if(! inverse_set) buildInverseMatrix();
  if(inverse_set) return new threeDtransform(t, &inv);
  else return NULL;
}

void threeDtransform::write(ostream *out) {
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      *out<<" \t"<< setw(10) << setprecision(8) <<t.aa[i][j];
    }
    *out<<'\n';
  }
  if(inverse_set) {
    *out<<"inverse=\n";
    for(int i=0; i<4; i++){
      for(int j=0; j<4; j++){
	*out<<" \t"<< setw(10) << setprecision(8) <<inv.aa[i][j];
      }
      *out<<'\n';
    }
  }
  else *out <<"inverse not set\n";
  if(quatern_set) {
    *out << "quatern.b="<<quatern.b<<" quatern.c="<<quatern.c<<" quatern.d="<<quatern.d<<'\n';
    *out << "quatern.qfac="<<quatern.qfac<<'\n';
    *out << "quatern.xoff="<<quatern.xoff<<" quatern.yoff="<<quatern.yoff<<" quatern.zoff="<<quatern.zoff<<'\n';
    *out << "quatern.xfactor="<<quatern.xfactor<<" quatern.yfactor="<<quatern.yfactor<<" quatern.zfactor="<<quatern.zfactor<<'\n';
  }
  else *out <<"quatern not set\n";
}
