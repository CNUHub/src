#include "psyhdr.h"
#include "talairach.h"
#include "psyiofunctions.h"
// Can't find where this is declared - hope it's right
int strncasecmp(const char*, const char*, size_t);
const double DEG_TO_RAD = (M_PI/180.0);
const double RAD_TO_DEG = (180.0/M_PI);

double CalcAngle(double x, double y){
  double angle;
  if(fabs(x)<1e-16) {
    if(y > 1e-16)angle=90.0;
    else if(y < -1e-16)angle= -90.0;
    else angle=0;
  }
  else angle=RAD_TO_DEG*atan2(y, x);
  return(angle);
}

to_talairach::to_talairach(psyimg *psyimgptr, psytype outtype,
			   int resize)
     : psyimglnkpxl(psyimgptr, outtype)
{
  init(resize);
}

void to_talairach::init(int resize)
{
  int xdim, ydim, zdim;//, idim;
  int xorig, yorig, zorig, iorig;
  int xend, yend, zend, iend;
  double xres, yres, zres, ires;
  xyzdouble diff;

  setmappingmode(standard_mappingmode);
  if(resize) {
// make output image 10% bigger then needed to store Talairach image
    xdim=irint((1.3*TALAIRACH_WIDTH)/res.x);
    ydim=irint((1.3*TALAIRACH_LENGTH)/res.y);
    zdim=irint((1.1*TALAIRACH_HEIGHT)/res.z);
    setsizeendinc(xdim, ydim, zdim, size.i);
  }
// locate Talairach origin x in center of image
  Offset.x = (orig.x + end.x) * .5 * res.x;
// locate Talairach origin y centered between ac pc
// with whole head length centered on image
//  Offset.y = (orig.y + end.y) * .5 * res.x -
//    (TALAIRACH_ACPC_CENTER - (TALAIRACH_LENGTH/2.0));
// locate Talairach origin y at standard ac location
  Offset.y = (orig.y + end.y) * .5 * res.x -
    (TALAIRACH_FRONT_AC_LENGTH - (TALAIRACH_LENGTH/2.0));
// locate Talairach z origin ac-pc distance below center of image
  Offset.z = (orig.z + end.z) * .5 * res.z +
    ((TALAIRACH_HEIGHT/2.0)-TALAIRACH_ACPC_HEIGHT);

// AtlasLengths of standard brain
  AtlasLengths.x=TALAIRACH_HEIGHT;
  AtlasLengths.y=TALAIRACH_LENGTH;
  AtlasLengths.z=TALAIRACH_HEIGHT - TALAIRACH_ACPC_HEIGHT;

// now build default Tomo values

// retrieve Tomo origin, end, and resolution
  inputpsyimg->getorig(&xorig, &yorig, &zorig, &iorig);
  inputpsyimg->getend(&xend, &yend, &zend, &iend);
  inputpsyimg->getres(&xres, &yres, &zres, &ires);

// Default lengths of Tomo set to same as Atlas
  Lengths = AtlasLengths;

// front of brain
// locate x at center of image
  front.x=(xend+xorig)*xres*0.5;
// locate y at standard talairach distance from middle of image
  front.y=(yend+yorig)*yres*0.5 + (Lengths.y/2.0);
// locate z at standard ACPC talairach distance below middle of image
  front.z=(zend+zorig)*zres*0.5 +
    (TALAIRACH_HEIGHT/2.0)-TALAIRACH_ACPC_HEIGHT;

// back of brain
  back.x = front.x;
  back.y = front.y - Lengths.y;
  back.z = front.z;

// set default AC & PC location based on standard ac-pc fractions
  diff=back-front;
  AC=front + (diff * (TALAIRACH_FRONT_AC_LENGTH/TALAIRACH_LENGTH));
  PC=AC + (diff * (TALAIRACH_ACPC_LENGTH/TALAIRACH_LENGTH));

// Rotations default to 0
  RelRot.x = RelRot.y = RelRot.z = 0;

//  TomoOrig = (AC + PC) * .5;
// new TomoOrig set to ac location
  TomoOrig = AC;

  Scale.x = Lengths.x / AtlasLengths.x;
  Scale.y = Lengths.y / AtlasLengths.y;
  Scale.z = Lengths.z / AtlasLengths.z;

  SinRelRot.x = sin(RelRot.x * DEG_TO_RAD);
  SinRelRot.y = sin(RelRot.y * DEG_TO_RAD);
  SinRelRot.z = sin(RelRot.z * DEG_TO_RAD);

  CosRelRot.x = cos(RelRot.x * DEG_TO_RAD);
  CosRelRot.y = cos(RelRot.y * DEG_TO_RAD);
  CosRelRot.z = cos(RelRot.z * DEG_TO_RAD);

}

void to_talairach::showto_talairach()
{
  showpsyimg();
  cout<<"to_talairach added values:\n";
  cout<<"AC=("<<AC.x<<','<<AC.y<<','<<AC.z<<")\n";
  cout<<"PC=("<<PC.x<<','<<PC.y<<','<<PC.z<<")\n";
  cout<<"TomoOrig=("<<TomoOrig.x<<','<<TomoOrig.y<<','<<TomoOrig.z<<")\n";
  cout<<"Lengths=("<<Lengths.x<<','<<Lengths.y<<','<<Lengths.z<<")\n";
  cout<<"AtlasLengths=("<<AtlasLengths.x<<',';
  cout<<AtlasLengths.y<<','<<AtlasLengths.z<<")\n";
  cout<<"Scale=("<<Scale.x<<','<<Scale.y<<','<<Scale.z<<")\n";
  cout<<"Offset=("<<Offset.x<<','<<Offset.y<<','<<Offset.z<<")\n";
  cout<<"RelRot=("<<RelRot.x<<','<<RelRot.y<<','<<RelRot.z<<")\n";
  cout<<"SinRelRot=("<<SinRelRot.x<<','<<SinRelRot.y<<','<<SinRelRot.z<<")\n";
  cout<<"CosRelRot=("<<CosRelRot.x<<','<<CosRelRot.y<<','<<CosRelRot.z<<")\n";
}

// Purpose: Output mapping parameters to a file
// Input: The file name
// Output: RelRot, Scales and TomoOrig written to a file
void to_talairach::outputmapping(char *file)
{
  if(file == NULL)return;
  fstream fd(file, ios::out);
  if(fd.fail() || fd.bad()) {
    cerr<<"to_talairach::outputmapping - unable to open file "<<file<<'\n';
    exit(1);
  }
  fd <<"# this file was program generated\n";
  fd <<"# person running program: "<<getenv("LOGNAME")<<'\n';
  fd <<"# date: "<< currentdate() << '\n';
  fd <<"# time: "<< currenttime() << '\n';
  /*  fd.form disappeared on me
  fd.form("TomoOrig(mm)=(%.12lg,%.12lg,%.12lg)\n",
	  TomoOrig.x*1000, TomoOrig.y*1000, TomoOrig.z*1000);
  fd.form("Scale=(%.12lg,%.12lg,%.12lg)\n",
	  Scale.x, Scale.y, Scale.z);
  fd.form("RelRot(deg)=(%.12lg,%.12lg,%.12lg)\n",
	  RelRot.x, RelRot.y, RelRot.z);
  */
  fd << "TomoOrig(mm)=("<<TomoOrig.x*1000<<','<<TomoOrig.y*1000<<',';
  fd <<TomoOrig.z*1000<<")\n";
  fd <<"Scale=("<<Scale.x<<','<<Scale.y<<','<<Scale.z<<")\n";
  fd <<"RelRot(deg)=("<<RelRot.x<<','<<RelRot.y<<','<<RelRot.z<<")\n";
}

// Purpose: Get mapping parameters from a file.
// Input:   The file name.
// Output:  RelRot, CosRelRot, SinRelRot, Scales, Lengths, and TomoOrig

inline void error_chk_get_xyzdouble(char *buff, xyzdouble *xyz) {
  if(sscanf(buff, "(%lf,%lf,%lf)",
	    &xyz->x, &xyz->y, &xyz->z) != 3){
    cerr<<"to_talairach::mappinginput - ";
    cerr<<"error reading triplet data=\n\""<<buff<<"\"\n";
    exit(1);
  }
}
inline void error_chk_get_double(char *buff, double *x) {
  if(sscanf(buff, "%lf", x) != 1){
    cerr<<"to_talairach::mappinginput - ";
    cerr<<"error reading single value data=\n\""<<buff<<"\"\n";
    exit(1);
  }
}

void to_talairach::mappinginput(char *file,
				mapfiletype mftype)
{
  xyzdouble localres;
  double ires;
  xyzdouble diff;
  char linebuff[1024], c, *cptr;
  //  int mode=0;
  int orig_flag=0, scale_flag=0, rotation_flag=0;
  int ac_flag=0, pc_flag=0;
  int front_flag=0, back_flag=0;
  int width_flag=0, height_flag=0;
  int yrotation_flag=0;
  double mag;

  if(file == NULL)mappinginput();
  else {
// retrieve resolutions for input conversions
    getres(&localres.x, &localres.y, &localres.z, &ires);
    fstream fd(file, ios::in);
    if(fd.fail() || fd.bad()) {
      cerr<<"to_talairach::mappinginput - unable to open file "<<file<<'\n';
      exit(1);
    }
    switch(mftype) {
    case standard_mapfile:
      while(! fd.eof() && fd.good()) {
	//remove left over carriage return
	if(fd.peek() == '\n')fd.get(c);
	fd.get(linebuff, 1024-1);
	if(fd.good()) {
	// parse line
	  if(linebuff[0] == '#') ; //ignore comment lines
	  else {
	    // find =
	    if((cptr=strchr(linebuff, '='))==NULL) {
	      cerr<<"to_taliarach::mappinginput - no \"=\" sign in input line\n";
	      cerr<<"\""<<linebuff<<"\"\n";
	      exit(1);
	    }
	    *cptr='\0'; cptr++;
	    if(strcmp(linebuff, "TomoOrig(mm)") == 0 && !orig_flag) {
	      error_chk_get_xyzdouble(cptr, &TomoOrig);
	      TomoOrig = TomoOrig * .001; //convert to meters
	      orig_flag=1;
	    }
	    else if(strcmp(linebuff, "TomoOrig(pixels)")==0 && !orig_flag) {
	      error_chk_get_xyzdouble(cptr, &TomoOrig);
	      TomoOrig = TomoOrig * localres; //convert to meters
		orig_flag=1;
	    }
	    else if(strcmp(linebuff, "Scale") == 0 && !scale_flag) {
	      error_chk_get_xyzdouble(cptr, &Scale);
	      scale_flag=1;
	    }
	    else if(strcmp(linebuff, "RelRot(deg)") == 0
		    && !rotation_flag && !yrotation_flag) {
	      error_chk_get_xyzdouble(cptr, &RelRot);
	      rotation_flag=1;
	    }
	    else if(strcmp(linebuff, "AC(mm)")==0 && !ac_flag) {
	      error_chk_get_xyzdouble(cptr, &AC);
	      AC = AC * .001; //convert to meters
	      ac_flag=1;
	    }
	    else if(strcmp(linebuff, "AC(pixels)")==0 && !ac_flag) {
	      error_chk_get_xyzdouble(cptr, &AC);
	      AC = AC * localres; //convert to meters
	      ac_flag=1;
	    }
	    else if(strcmp(linebuff, "PC(mm)")==0 && !pc_flag) {
	      error_chk_get_xyzdouble(cptr, &PC);
	      PC = PC * .001; //convert to meters
	      pc_flag=1;
	    }
	    else if(strcmp(linebuff, "PC(pixels)")==0 && !pc_flag) {
	      error_chk_get_xyzdouble(cptr, &PC);
	      PC = PC * localres; //convert to meters
	      pc_flag=1;
	    }
	    else if(strcmp(linebuff, "Front(mm)")==0 && !front_flag) {
	      error_chk_get_xyzdouble(cptr, &front);
	      front = front * .001; //convert to meters
	      front_flag=1;
	    }
	    else if(strcmp(linebuff, "Front(pixels)")==0 && !front_flag) {
	      error_chk_get_xyzdouble(cptr, &front);
	      front = front * localres; //convert to meters
	      front_flag=1;
	    }
	    else if(strcmp(linebuff, "Back(mm)")==0 && !back_flag) {
	      error_chk_get_xyzdouble(cptr, &back);
	      back = back * .001; //convert to meters
	      back_flag=1;
	    }
	    else if(strcmp(linebuff, "Back(pixels)")==0 && !back_flag) {
	      error_chk_get_xyzdouble(cptr, &back);
	      back = back * localres; //convert to meters
	      back_flag=1;
	    }
	    else if(strcmp(linebuff, "Width(mm)")==0 && !width_flag) {
	      error_chk_get_double(cptr, &Lengths.x);
	      Lengths.x *= .001; //convert to meters
	      width_flag=1;
	    }
	    else if(strcmp(linebuff, "Width(pixels)")==0 && !width_flag) {
	      error_chk_get_double(cptr, &Lengths.x);
	      Lengths.x *= localres.x; //convert to meters
	      width_flag=1;
	    }
	    else if(strcmp(linebuff, "Height(mm)")==0 && !height_flag) {
	      error_chk_get_double(cptr, &Lengths.z);
	      Lengths.z *= .001; //convert to meters
	      height_flag=1;
	    }
	    else if(strcmp(linebuff, "Height(pixels)")==0 && !height_flag) {
	      error_chk_get_double(cptr, &Lengths.z);
	      Lengths.z *= localres.z; //convert to meters
	      height_flag=1;
	    }
	    else if(strcmp(linebuff, "YRotation(deg)")==0 &&
		    !yrotation_flag && !rotation_flag) {
	      error_chk_get_double(cptr, &RelRot.y);
	      yrotation_flag=1;
	    }
	    else {
	      cerr<<"to_talairach::mappinginput error unrecognized";
	      cerr<<"or repeated value name=\"";
	      cerr<<linebuff<<"\"\n";
	      exit(1);
	    }// end if ... else if ... else ...
	  }// end else linebuff[0] != '#'
	}// end if(fd.good())
      }// end while
      break;
    case satoshi_sta_lib:
      
      if(fd.good()) {
	cout<<"mappinginput satoshi_sta_lib parameters:\n";
	fd>>TomoOrig.x>>TomoOrig.y>>TomoOrig.z;
	if(!fd.good()) {
	  cerr<<"to_talairach::mappinginput - error reading TomoOrig\n";
	  exit(1);
	}
	cout<<"Original Pixel TomoOrig=("<<TomoOrig.x<<','<<TomoOrig.y<<','<<TomoOrig.z<<")\n";
// convert to Satoshi pixel location in meters
        TomoOrig.x = TomoOrig.x * localres.x;
        TomoOrig.y = TomoOrig.y * localres.y;
        TomoOrig.z = TomoOrig.z * .00225;
	cout<<"TomoOrig=("<<TomoOrig.x<<','<<TomoOrig.y<<','<<TomoOrig.z<<")\n";
	orig_flag=1;
	fd>>RelRot.x>>RelRot.y>>RelRot.z;
	if(!fd.good()) {
	  cerr<<"to_talairach::mappinginput - error reading RelRot\n";
	  exit(1);
	}
	rotation_flag=1;
        fd>>Scale.y>>Scale.x>>Scale.z;
	Scale.x = (Scale.x + Scale.z)/2.0;
	fd>>Scale.z;
	if(!fd.good()) {
	  cerr<<"to_talairach::mappinginput - error reading Scale\n";
	  exit(1);
	}
	scale_flag=1;
// origin is relative to the center of the image
// need to find ac origin on acpc plane
        TomoOrig.y = TomoOrig.y +
	  ((TALAIRACH_LENGTH/2.0) - TALAIRACH_FRONT_AC_LENGTH)/Scale.y;
/*
	TomoOrig.z = TomoOrig.z -
	  ((TALAIRACH_HEIGHT/2) - TALAIRACH_ACPC_HEIGHT)/Scale.z;
*/
	cout<<"Corrected TomoOrig=("<<TomoOrig.x<<','<<TomoOrig.y<<','<<TomoOrig.z<<")\n";
	cout<<"RelRot=("<<RelRot.x<<','<<RelRot.y<<','<<RelRot.z<<")\n";
	cout<<"Scale=("<<Scale.x<<','<<Scale.y<<','<<Scale.z<<")\n";
      }
      break;
    }// end switch(mftype)
// check input mode and do appropriate calculations
    if(orig_flag && scale_flag && rotation_flag) {
      if(ac_flag || pc_flag || front_flag || back_flag ||
	 width_flag || height_flag) {
	cerr<<"to_talairach::mappinginput - ";
	cerr<<"warning file specifies unused information\n";
      }
// calculate trig values based on file input
      SinRelRot.x = sin(RelRot.x * DEG_TO_RAD);
      CosRelRot.x = cos(RelRot.x * DEG_TO_RAD);
      SinRelRot.y = sin(RelRot.y * DEG_TO_RAD);
      CosRelRot.y = cos(RelRot.y * DEG_TO_RAD);
      SinRelRot.z = sin(RelRot.z * DEG_TO_RAD);
      CosRelRot.z = cos(RelRot.z * DEG_TO_RAD);
// calculate lengths although they're not used for anything
      Lengths = Scale * AtlasLengths;
// set ac pc to zero to show not calculated
//      AC=AC * 0.0;
// no set ac to origin need to come up with better calculation for pc
      AC=TomoOrig;
      PC=AC;
    }
    else if((ac_flag && pc_flag) && (front_flag && back_flag)) {
      cerr<<"to_talairach::mappinginput - ";
      cerr<<"file specifies both acpc and front & back points\n";
      exit(1);
    }
    else if(((ac_flag && pc_flag)||(front_flag && back_flag))
	    && width_flag && height_flag) {
// calculate front to back difference based on best info given
      if(front_flag && back_flag)diff = back - front;
      else diff = (PC - AC) * (TALAIRACH_LENGTH/TALAIRACH_ACPC_LENGTH);
      mag = magnitude(diff);
      if(mag < 1e-10) {
       	cerr<<"to_talairach::mappinginput - ";
	cerr<<"specified front and back or AC and PC to close together\n";
        exit(1);
      }
// set y length from front to back magnitude
      Lengths.y = mag;
      if(front_flag && back_flag) {
// locate ac and pc based on front and back
	AC = front + diff * (TALAIRACH_FRONT_AC_LENGTH/TALAIRACH_LENGTH);
	PC = AC + diff * (TALAIRACH_ACPC_LENGTH/TALAIRACH_LENGTH);
      }
      else {
// locate front and back based on ac and pc
        front = AC - diff * (TALAIRACH_FRONT_AC_LENGTH/TALAIRACH_LENGTH);
        back = diff + front;
      }
// calculate origin
//      TomoOrig = (AC + PC) * .5;
// new origin set to AC location
      TomoOrig = AC;
// set Scales to 1 for angle calculations
      Scale.x = Scale.y = Scale.z=1;
// calculate rotation
// calc z angle to straighten front to back line
// first find front in TomoCoordinates note - indice2tomo inverts z
      diff = front / localres;
      indice2tomo(diff.x, diff.y, diff.z, &diff.x, &diff.y, &diff.z);
// then since front should lie on y axis rotate -90 degrees(x=y,y=-x)
// and calculate angle from x axis
      RelRot.z = CalcAngle(diff.y, -diff.x);
      SinRelRot.z = sin(RelRot.z * DEG_TO_RAD);
      CosRelRot.z = cos(RelRot.z * DEG_TO_RAD);
// make sure y trig values are set
      SinRelRot.y = sin(RelRot.y * DEG_TO_RAD);
      CosRelRot.y = cos(RelRot.y * DEG_TO_RAD);
// calc x angle to straighten front to back line
// first need to calculate front location after z & y rotation
      SinRelRot.x = 0;
      CosRelRot.x = 1;
      diff=TomoToAtlas(diff);
// to find x rotation rotate x to z with y going to x and z to y
      RelRot.x = CalcAngle(diff.y, diff.z);
      SinRelRot.x = sin(RelRot.x * DEG_TO_RAD);
      CosRelRot.x = cos(RelRot.x * DEG_TO_RAD);
// calculate scale factors
      Scale = Lengths / AtlasLengths;
    }
    else {
      cerr<<"to_talairach::mappinginput - missing information from file\n";
      exit(1);
    }

  }// end else
}

// Purpose: Get mapping parameters from the user.
// Output:  rotations, Atlas constants, lengths, AC/PC locations.
       
void to_talairach::mappinginput()
{
  double xres, yres, zres, ires;
  xyzdouble diff1, diff2;
  //  int xdim, ydim, zdim, idim;
  xyzdouble center;
  double frontbackdist, scalelength;

// initialize input classes
  unitinput inputmeters("meters");
  inputmeters.setdefaultvalue(0);
  inputmeters.setdefaultunits("mm");
  unitinput inputdegrees("degrees");
  inputdegrees.setdefaultvalue(0);
  inputdegrees.setdefaultunits("deg");

// retrieve resolutions for input conversions
  getres(&xres, &yres, &zres, &ires);

// get extents used to set default length and ac pc locations
  cout << "\n";
  cout << "Enter front of brain x, y & z[location [units]]\n";
  cout << "(Note - two carriage returns accepts default):\n";

  inputmeters.setdefaultvalue(front.x);
  inputmeters.setpixelunits(xres);
  front.x=inputmeters.get("   X front of brain location");

  inputmeters.setdefaultvalue(front.y);
  inputmeters.setpixelunits(yres);
  front.y=inputmeters.get("   Y front of brain location");

  inputmeters.setdefaultvalue(front.z);
  inputmeters.setpixelunits(zres);
  front.z=inputmeters.get("   Z front of brain location");
  cout << "Enter back of brain x, y & z[location [units]]:\n";

  back.x=front.x;
  inputmeters.setdefaultvalue(back.x);
  inputmeters.setpixelunits(xres);
  back.x=inputmeters.get("   X back of brain location");

  back.y=front.y - TALAIRACH_LENGTH;
  inputmeters.setdefaultvalue(back.y);
  inputmeters.setpixelunits(yres);
  back.y=inputmeters.get("   Y back of brain location");

  back.z=front.z;
  inputmeters.setdefaultvalue(back.z);
  inputmeters.setpixelunits(zres);
  back.z=inputmeters.get("   Z back of brain location");

// reset default y length
  diff1=back-front;
  frontbackdist=magnitude(diff1);
  Lengths.y = frontbackdist;
// Lengths along each axis
  cout << "\n";
  cout << "Enter axis lengths relative to AC-PC line[length [units]]:\n";

  inputmeters.setdefaultvalue(Lengths.x);
  inputmeters.setpixelunits(xres);
  Lengths.x=inputmeters.get("   X axis length");

  inputmeters.setdefaultvalue(Lengths.y);
  inputmeters.setpixelunits(yres);
  Lengths.y=inputmeters.get("   Y axis length(AC-PC length)");

  inputmeters.setdefaultvalue(Lengths.z);
  inputmeters.setpixelunits(zres);
  Lengths.z=
    inputmeters.get("   Z axis length(AC-PC to top of head)");

// keeping the same centroid and angle reset front and back to have lengths.y
  center = (front + back) * .5;
  scalelength=(Lengths.y/frontbackdist);
  front = center - (diff1 * (.5 * scalelength));
  back = front + (diff1 * scalelength);

// set default AC location based on standard ac-pc fractions
// between front and back extents
  diff1=back-front;
  AC=front + (diff1 * (TALAIRACH_FRONT_AC_LENGTH/TALAIRACH_LENGTH));
// Enter the AC and PC locations in the min tomo coor. system
  cout << "\n";
  cout << "Enter the AC X, Y and Z coordinates[location [units]]\n";
  cout << "(Note - rotation and scaling performed around AC-PC midpoint):\n";
  inputmeters.setdefaultvalue(AC.x);
  inputmeters.setpixelunits(xres); AC.x=inputmeters.get("    X");
  inputmeters.setdefaultvalue(AC.y);
  inputmeters.setpixelunits(yres); AC.y=inputmeters.get("    Y");
  inputmeters.setdefaultvalue(AC.z);
  inputmeters.setpixelunits(zres); AC.z=inputmeters.get("    Z");

// set default PC location based on AC and standard ac-pc fractions
  PC=AC + (diff1 * (TALAIRACH_ACPC_LENGTH/TALAIRACH_LENGTH));

  cout << "Enter the PC X, Y and Z coordinates[location [units]]:\n";
  inputmeters.setdefaultvalue(PC.x);
  inputmeters.setpixelunits(xres); PC.x=inputmeters.get("    X");
  inputmeters.setdefaultvalue(PC.y);
  inputmeters.setpixelunits(yres); PC.y=inputmeters.get("    Y");
  inputmeters.setdefaultvalue(PC.z);
  inputmeters.setpixelunits(zres); PC.z=inputmeters.get("    Z");
//  TomoOrig = (AC + PC) * .5;
// new TomoOrig set to AC location
  TomoOrig = AC;

// Relative rotation about each axis around origin
// calc default z angle to straighten front to origin line
// first find front in TomoCoordinates note - indice2tomo inverts z
  indice2tomo(front.x/res.x, front.y/res.y, front.z/res.z,
	      &diff1.x, &diff1.y, &diff1.z);
// note - diff lies along y therefore y-axis rotated -90 to x-axis
// (ie. x=y, y=-x) when finding z rotation
  RelRot.z = CalcAngle(diff1.y, -diff1.x);

  cout << "\n";
  cout << "Enter relative Z, Y and X rotations[angle [units]]:\n";
  cout << "(Note-converting image to taliarach Z rotation performed first)\n";
  inputdegrees.setdefaultvalue(RelRot.z);
  RelRot.z=inputdegrees.get("    Rotation about the Z axis");
  SinRelRot.z = sin(RelRot.z * DEG_TO_RAD);
  CosRelRot.z = cos(RelRot.z * DEG_TO_RAD);

// default y rotation is zero we haven't enough info to calculate it
  RelRot.y = 0.0;

// get y rotation
  inputdegrees.setdefaultvalue(RelRot.y);
  RelRot.y=inputdegrees.get("    Rotation about the Y axis");

  SinRelRot.y = sin(RelRot.y * DEG_TO_RAD);
  CosRelRot.y = cos(RelRot.y * DEG_TO_RAD);

// set trig values of x and scale to allow using TomoToAtlas
// and keep transform calculations in one place
  SinRelRot.x = 0;
  CosRelRot.x = 1;
  Scale.x=Scale.y=Scale.z=1;

// calc default x angle to straighten ac-pc line
// first correct for z & y rotation
  diff2=TomoToAtlas(diff1);
// to find x rotation rotate x to z with y going to x and z to y
  RelRot.x = CalcAngle(diff2.y, diff2.z);

  inputdegrees.setdefaultvalue(RelRot.x);
  RelRot.x=inputdegrees.get("    Rotation about the X axis");
  SinRelRot.x = sin(RelRot.x * DEG_TO_RAD);
  CosRelRot.x = cos(RelRot.x * DEG_TO_RAD);
// calc scale factors
  Scale.x = Lengths.x / AtlasLengths.x;
  Scale.y = Lengths.y / AtlasLengths.y;
  Scale.z = Lengths.z / AtlasLengths.z;


  return;
}


// Purpose: To map tomographic coordinates to Talairach Atlas
//          coordinates.
//
//          These equations were derived from the equations in Fox,
//          Perlmutter and Raichle's paper "A Stereotactic
//          Method of Anatomical Localization for Positron Emission
//          Tomography" in Journal of Computer Assisted Tomography;
//          v.9:1; 1985; p.p. 141-153.
//          
// Input:   The tomographic coordinates and various transformation 
//          parameters: scalings and rotations.
// Output:  The Talairach Atlas coordinates.

xyzdouble to_talairach::TomoToAtlas(xyzdouble TomoCoor)
{
   xyzdouble term1, term2;
// invert rotation around z axis
   term1.z = TomoCoor.z;
   term1.x = CosRelRot.z*TomoCoor.x + SinRelRot.z*TomoCoor.y;
   term1.y = -SinRelRot.z*TomoCoor.x + CosRelRot.z*TomoCoor.y;
// invert rotation around y axis
   term2.y = term1.y;
   term2.z = CosRelRot.y*term1.z + SinRelRot.y*term1.x;
   term2.x = -SinRelRot.y*term1.z + CosRelRot.y*term1.x;
// invert rotation around x axis
   term1.x = term2.x;
   term1.y = CosRelRot.x*term2.y + SinRelRot.x*term2.z;
   term1.z = -SinRelRot.x*term2.y + CosRelRot.x*term2.z;
// invert scale
   term2 = term1/Scale;

   return term2;
}

// Purpose: To map Talairach Atlas coordinates to tomographic
//          coordinates.
//
//          These equations were taken from Fox, Perlmutter and
//          Raichle's paper "A Stereotactic Method of Anatomical
//          Localization for Positron Emission Tomography" in Journal
//          of Computer Assisted Tomography; v.9:1; 1985; p.p. 141-153.
//          
// Input:   The Talairach Atlas coordinates and various transformation 
//          parameters: scalings and rotations.
// Output:  The tomographic coordinates.

xyzdouble to_talairach::AtlasToTomo(xyzdouble AtlasCoor)
{
   xyzdouble term1, term2;
//cout<<"cosines=("<<CosRelRot.x<<','<<CosRelRot.y<<','<<CosRelRot.z<<")\n";
//cout<<"sinines=("<<SinRelRot.x<<','<<SinRelRot.y<<','<<SinRelRot.z<<")\n";
//cout<<"AtlasCoor=("<<AtlasCoor.x<<','<<AtlasCoor.y<<','<<AtlasCoor.z<<")\n";
//cout<<"term2=("<<term2.x<<','<<term2.y<<','<<term2.z<<")\n";

// scale
   term1 = Scale * AtlasCoor;
// rotate around x axis 
   term2.x = term1.x;
   term2.y = CosRelRot.x*term1.y - SinRelRot.x*term1.z; 
   term2.z = SinRelRot.x*term1.y + CosRelRot.x*term1.z;
// rotate around y axis
   term1.y = term2.y;
   term1.z = CosRelRot.y*term2.z - SinRelRot.y*term2.x; 
   term1.x = SinRelRot.y*term2.z + CosRelRot.y*term2.x;
// rotate around z axis
   term2.z = term1.z;
   term2.x = CosRelRot.z*term1.x - SinRelRot.z*term1.y; 
   term2.y = SinRelRot.z*term1.x + CosRelRot.z*term1.y;

   return term2;
}

void to_talairach::indice2atlas(double xi, double yi, double zi,
				double *xa, double *ya, double *za)
{
  *xa = (res.x * xi) - Offset.x;
  *ya = (res.y * yi) - Offset.y;
  *za = Offset.z - (res.z * zi); // reversing z sign
}

void to_talairach::indice2tomo(double xi, double yi, double zi,
			       double *xt, double *yt, double *zt)
{
  *xt = (res.x * xi) - TomoOrig.x;
  *yt = (res.y * yi) - TomoOrig.y;
  *zt = TomoOrig.z - (res.z * zi); // reversing z sign
}

void to_talairach::tomo2indice(double xt, double yt, double zt,
			       double *xi, double *yi, double *zi)
{
  *xi = (xt + TomoOrig.x)/res.x;
  *yi = (yt + TomoOrig.y)/res.y;
  *zi = (TomoOrig.z - zt)/res.z; // reversing z sign
}

// Purpose: To map a Talairach image to a tomographic image.
// Input:   The tomographic image name, the mapping parameters 
//          acquired by MappingInput() and a pointer to the 
//          Talairach image file.
// Output:  The minimum and maximum value in the Talairach image file.

void to_talairach::getnextpixel(char *pixel)
{
  xyzdouble AtlasCoor;     // Stereotactic atlas coordinates.      [m]
  xyzdouble TomoCoor;      // Tomographic coordinates.             [m] 
  xyzint RawIndex;         // x,y,z indices into Raw Data          [-]

  // Compute Talairach Atlas coordinates from Talairach indices
  indice2atlas((double)getpixelloc.x, (double)getpixelloc.y,
	       (double)getpixelloc.z,
	       &AtlasCoor.x, &AtlasCoor.y, &AtlasCoor.z);

  // Convert Talairach Atlas coordinates to tomographic
  // coordinates.

  TomoCoor = AtlasToTomo(AtlasCoor);

  // Convert tomographic coordinates to min tomographic
  // coordinates.

  tomo2indice(TomoCoor.x, TomoCoor.y, TomoCoor.z,
	      &TomoCoor.x, &TomoCoor.y, &TomoCoor.z);

  // Find the indices corresponding to the min TomoCoor
  // round .5 down otherwize odd output dimensions produce replicate pixels

  RawIndex.x = (int)ceil(TomoCoor.x - .5);
  RawIndex.y = (int)ceil(TomoCoor.y - .5);
  RawIndex.z = (int)ceil(TomoCoor.z - .5);
  switch(getmappingmode()) {
  default:
  case standard_mappingmode:
    // Check to see if the indexed voxel lies in the tomographic
    // image. If not set the Talairach value to 0; if yes set
    // the Talairach value to the correct input value.
    if(!inputpsyimg->inside(RawIndex.x, RawIndex.y,
			    RawIndex.z, getpixelloc.i)) {
      for(int i=gettypebytes(getpixeltype)-1; i>=0; i--)pixel[i]=0;
    }
    else {
      inputpsyimg->getpixel(pixel, getpixeltype,
			    RawIndex.x, RawIndex.y,
			    RawIndex.z, getpixelloc.i);
    }
    break;
  case outside_nearest:
    xyzint nearloc;
    nearloc.x=clip(RawIndex.x, orig.x, end.x);
    nearloc.y=clip(RawIndex.y, orig.y, end.y);
    nearloc.z=clip(RawIndex.z, orig.z, end.z);
    inputpsyimg->getpixel(pixel, getpixeltype,
			  nearloc.x, nearloc.y,
			  nearloc.z, getpixelloc.i);
    break;
  }
  incgetpixel();
}

void talairach_location::init(psyimg *psyimgptr, xyzint *reverse_flag)
{
  double dtemp;
  int itemp;
  xyzint orig, end;
  xyzdouble res;
  
  psyimgptr->getres(&res.x, &res.y, &res.z, &dtemp);
  psyimgptr->getorig(&orig.x, &orig.y, &orig.z, &itemp);
  psyimgptr->getend(&end.x, &end.y, &end.z, &itemp);
  init(orig, end, res, reverse_flag);
}

void talairach_location::init(xyzint orig, xyzint end, xyzdouble res,
			      xyzint *rflag)
{
  xyzdouble image_center_pixels;
  double dtemp;
  xyzdouble xyzdtemp;

  // note - had to rename input reverse_flag rflag because compiler
    // complained about not aggregate with talairach::reverse_flag.x
  if(rflag != NULL) {
    reverse_flag = *rflag;
  }
  else {
    reverse_flag.x = 0;
    reverse_flag.y = 0;
    reverse_flag.z = 0;
  }

  // locate center of image in pixels
  image_center_pixels= (xyzint2double(orig) + xyzint2double(end)) * .5;

  // set res
  talairach_location::res = res;

  // locate standard location front and back of brain on AC-PC line
  front = image_center_pixels * res;
  // x remains at center of image

  // y located at standard talairach distance from middle of image
  front.y += (TALAIRACH_LENGTH/2.0);

  // locate z at standard ACPC talairach distance below middle of image
  if(reverse_flag.z)
    front.z -= (TALAIRACH_HEIGHT/2.0)-TALAIRACH_ACPC_HEIGHT;
  else front.z += (TALAIRACH_HEIGHT/2.0)-TALAIRACH_ACPC_HEIGHT;

  // locate back
  back.x=front.x;
  back.y=front.y - TALAIRACH_LENGTH;
  back.z=front.z;

  if(reverse_flag.y) {
    dtemp=front.y;
    front.y=back.y;
    back.y=dtemp;
  }

  // locate ac pc
  xyzdtemp=back-front;
  ac=front + (xyzdtemp * (TALAIRACH_FRONT_AC_LENGTH/TALAIRACH_LENGTH));
  pc=ac + (xyzdtemp * (TALAIRACH_ACPC_LENGTH/TALAIRACH_LENGTH));

  // locate origin
//  origin = (ac + pc) * .5;
// new origin located at ac location
  origin = ac;
}

void talairach_location::pixel2talairach(double xp, double yp, double zp,
					 double *xt, double *yt, double *zt)
{
// normally increasing x is positive towards left of brain
  *xt = xp * res.x - origin.x;
  if(reverse_flag.x) *xt = - *xt;
// normally increasing y is positive towards front of brain
  *yt = yp * res.y - origin.y;
  if(reverse_flag.y) *yt = - *yt;
// normally decreasing z location is positive towards top of brain
  *zt = origin.z - zp * res.z;
  if(reverse_flag.z)*zt = - *zt;
}
