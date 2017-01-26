#include "psyhdr.h"
#include <stdio.h>

class psywritable : public psyfullbuff {
 public:
  psywritable();
  psywritable(psyimg *psyimgptr);
  void initpsywritable(psyimg *psyimgptr);
  void output_tree(ostream *out) {psyfullbuff::output_tree(out);*out<<"::psywritable";};
  void chknfillbuff();
  void setpixel(char *pixel, psytype pixeltype,
		int x, int y, int z, int i);
};

psywritable::psywritable(psyimg *psyimgptr) : psyfullbuff()
{
  initpsywritable(psyimgptr);
}

void psywritable::initpsywritable(psyimg *psyimgptr)
{
  initpsyimglnk(psyimgptr);
}

void psywritable::setpixel(char *pixel, psytype pixeltype,
			   int x, int y, int z, int i) {
  chknfillbuff();
  pixeltypechg(pixel, pixeltype,
	       buffimage.getbuff() + buffimage.offset(x, y, z, i),
	       buffimage.gettype());
}

void psywritable::chknfillbuff() {
  if(buffimage.getbuff() == NULL) {
// initialize buffer to full histogram size
    psydims orig = getorig();
    psyres res = getres();
    buffimage.initbuff(size.x, size.y, size.z, size.i,
		       gettype(), orig.x, orig.y, orig.z, orig.i,
		       res.x, res.y, res.z, res.i, getwordres());
// initialize values to that of input image
    psydims end = getend();
    psydims inc = getinc();
    inputpsyimg->copyblock(buffimage.getbuff(),
			   orig.x, orig.y, orig.z, orig.i,
			   end.x, end.y, end.z, end.i,
			   inc.x, inc.y, inc.z, inc.i, gettype());
  }
}

class psyimghalfspace : public psyimgshape {
  xyzdouble point, normal;
 public:
  psyimghalfspace();
  psyimghalfspace(psyimg *psyimgptr, xyzdouble point, xyzdouble normal,
	    psytype pixeltype=psynotype);
  void init(psyimg *psyimgptr, xyzdouble point, xyzdouble normal,
	    psytype pixeltype=psynotype);
  void output_tree(ostream *out) {psyimgshape::output_tree(out);*out<<"::psyimgalt";};
  int inshape(psydims location);
};

psyimghalfspace::psyimghalfspace()
{
  point.x = point.y = point.z = 0.0;
  normal.x = normal.y = 0.0;
  normal.z = 1.0;
}

psyimghalfspace::psyimghalfspace(psyimg *psyimgptr, xyzdouble point, xyzdouble normal,
				 psytype pixeltype) {
  init(psyimgptr, point, normal, pixeltype);
}

void psyimghalfspace::init(psyimg *psyimgptr, xyzdouble point, xyzdouble normal,
			   psytype pixeltype) {
  psyimgshape::init(psyimgptr, pixeltype);
  psyimghalfspace::point = point;
  psyimghalfspace::normal = normal;
}

int psyimghalfspace::inshape(psydims location) {
  xyzdouble diff;
  diff.x=location.x-point.x; diff.y=location.y-point.y; diff.z=location.z-point.z;
  return( ((diff.x*normal.x) + (diff.y*normal.y) + (diff.z*normal.z)) >= 0);
}

class psyimgalt : public psyimgshape {
  psydims alternate;
 public:
  psyimgalt();
  psyimgalt(psyimg *psyimgptr, psydims alternate,
	    psytype pixeltype=psynotype);
  void init(psyimg *psyimgptr, psydims alternate,
	    psytype pixeltype=psynotype);
  void output_tree(ostream *out) {psyimgshape::output_tree(out);*out<<"::psyimgalt";};
  int inshape(psydims location);
};

psyimgalt::psyimgalt()
{
  alternate.x=alternate.y=alternate.z=alternate.i=1;
}

psyimgalt::psyimgalt(psyimg *psyimgptr, psydims alternate,
		     psytype pixeltype)
{
  init(psyimgptr, alternate, pixeltype);
}

void psyimgalt::init(psyimg *psyimgptr, psydims alternate,
		     psytype pixeltype)
{
  psyimgshape::init(psyimgptr, pixeltype);
  if(alternate.x <= 0)alternate.x=2*size.x;
  if(alternate.x <= 0)alternate.x=1;
  if(alternate.y <= 0)alternate.y=2*size.y;
  if(alternate.y <= 0)alternate.y=1;
  if(alternate.z <= 0)alternate.z=2*size.z;
  if(alternate.z <= 0)alternate.z=1;
  if(alternate.i <= 0)alternate.i=2*size.i;
  if(alternate.i <= 0)alternate.i=1;
  psyimgalt::alternate = alternate;
}

int psyimgalt::inshape(psydims location) {
  return(((int)((location.x+1) / alternate.x) +
	  (int)((location.y+1) / alternate.y) +
	  (int)((location.z+1) / alternate.z) +
	  (int)((location.i+1) / alternate.i)) % 2);
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  char *desc;
  psytype outtype=psynotype;
  psyfileclass outfileclass=psynoclass;
  psydims dims;
  xyzdouble center;
  double radius;
  xyzdouble length;
  int axis;
  psydims box_orig, box_end;
  char *ptsfile=NULL;
  psydims alternate;
  int box_set=0;
  int box_centroid=0;
  int sphere_set=0;
  int ellipsoid_set=0;
  int cylinder_set=0;
  int alternate_set=0;
  int halfspace_set=0;
  double foreground_value=1;
  double background_value=0;
  int background_infile_flag=0;
  int ibegin = -1;
  int iend = -1;

// default image dimensions
  dims.x=dims.y=128;
  dims.z=31;
  dims.i=1;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" outfile\n";
      cout <<"      [ -if infile | -size xdim,ydim,zdim,idim ]\n";
      cout <<"      [ -b xorig,yorig,zorig,xend,yend,zend |\n";
      cout <<"        -bc xcenter,ycenter,zcenter,xlength,ylength,zlength |\n";
      cout <<"        -c xcenter,ycenter,zcenter,radius |\n";
      cout <<"        -cyl icenter,jcenter,axis,radius |\n";
      cout <<"        -e xcenter,ycenter,zcenter,xlength,ylength,zlength |\n";
      cout <<"        -half xpoint,ypoint,zpoint,xnormal,ynormal,znormal |\n";
      cout <<"        -pts pointlistfile | -alt ax,ay,az,ai ]\n";
      cout <<"      [ -irange ibegin,iend ]\n";
      cout <<"      [-fg foreground_value]\n";
      cout <<"      [-bg background_value | -bif]\n";
      cout <<"      ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"      ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp(argv[i],"-if")==0)&&((i+1)<argc))infile=argv[++i];
    else if((strcmp(argv[i],"-size")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%d,%d,%d,%d",
		&dims.x, &dims.y, &dims.z, &dims.i) != 4) {
	cerr << argv[0] << ": error parsing size: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-b")==0)&&((i+1)<argc)) {
      box_set=1; box_centroid=0;
      if(sscanf(argv[++i],"%d,%d,%d,%d,%d,%d",
		&box_orig.x, &box_orig.y, &box_orig.z,
		&box_end.x, &box_end.y, &box_end.z) != 6) {
	cerr << argv[0] << ": error parsing box origin and end: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-bc")==0)&&((i+1)<argc)) {
      box_set=1; box_centroid=1;
      if(sscanf(argv[++i],"%lf,%lf,%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z,
		&length.x, &length.y, &length.z) != 6) {
	cerr << argv[0] << ": error parsing box center and lengths: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-c")==0)&&((i+1)<argc)) {
      sphere_set=1;
      if(sscanf(argv[++i],"%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z, &radius) != 4) {
	cerr << argv[0] << ": error parsing sphere center and radius: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-cyl")==0)&&((i+1)<argc)) {
      cylinder_set=1;
      if(sscanf(argv[++i],"%lf,%lf,%d,%lf",
		&center.x, &center.y, &axis, &radius) != 4) {
	cerr << argv[0] << ": error parsing cylinder center, axis and radius: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-e")==0)&&((i+1)<argc)) {
      ellipsoid_set=1;
      if(sscanf(argv[++i],"%lf,%lf,%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z,
		&length.x, &length.y, &length.z) != 6) {
	cerr << argv[0] << ": error parsing ellipsoid center and lengths: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-half")==0)&&((i+1)<argc)) {
      halfspace_set=1;
      if(sscanf(argv[++i],"%lf,%lf,%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z,
		&length.x, &length.y, &length.z) != 6) {
	cerr << argv[0] << ": error parsing halfspace point and normal: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-pts")==0)&&((i+1)<argc))ptsfile=argv[++i];
    else if((strcmp(argv[i],"-alt")==0)&&((i+1)<argc)) {
      alternate_set=1;
      if(sscanf(argv[++i],"%d,%d,%d,%d",
		&alternate.x, &alternate.y, &alternate.z,
		&alternate.i) != 4) {
	cerr << argv[0] << ": error parsing alternate lengths: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-irange")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%d,%d", &ibegin, &iend) != 2) {
	cerr << argv[0] << ": error parsing i origin and end: ";
	cerr << argv[i-1] << " " << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-fg")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf", &foreground_value) != 1) {
	cerr << argv[0] << ": error parsing foreground_value: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-bg")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf", &background_value) != 1) {
	cerr << argv[0] << ": error parsing background_value: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if(strcmp(argv[i],"-bif")==0)background_infile_flag=1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }

// open input file to use dimensions
  psyimg *inimage=NULL;
  psyimg *base_image=NULL;
  if(infile != NULL) {
    psytype intype;
    psyfileclass infileclass;
    inimage=psynewinfile(infile, &infileclass, &intype);
    if(outfileclass==psynoclass)outfileclass=infileclass;
    if(outtype == psynotype) {
      if(background_infile_flag)outtype=intype;
      else outtype=psyuchar;
    }
    base_image=new psypgbuff(inimage, 1);
  }
  else {
    if(outtype == psynotype)outtype=psyuchar;
    if(outfileclass==psynoclass)outfileclass=analyzefileclass;
    base_image=new psyimgconstant(background_value,
				  dims.x, dims.y, dims.z,
				  dims.i, outtype);
  }

  psyimg *imgptr = NULL;
  psyimgshape *shape_image=NULL;
  psywritable *writable_image=NULL;
  psyimgconstant *background_constant_image=NULL;
  psydims orig = base_image->getorig();
  psydims end = base_image->getend();
  ibegin = clip(ibegin, orig.i, end.i);
  if(iend < 0) iend = end.i;
  else iend = clip(iend, orig.i, end.i);
  if(box_set) {
    if(sphere_set || ellipsoid_set){
      cerr << argv[0] << ": mutually exclusive options entered\n";
      exit(1);
    }
// calculate box region
    if(box_centroid) {
      xyzint bo=xyzdouble2int((center - (length * 0.5)) + 0.5);
      xyzint be= bo + xyzdouble2int(length);
      box_orig.x = bo.x; box_orig.y = bo.y; box_orig.z = bo.z; box_orig.i = 0;
      box_end.x = be.x; box_end.y = be.y; box_end.z = be.z; box_end.i = 0;
// the following started failing for unknown assigment problems casting xyziint as xyzint
// the assignment left box_orig and box_end completely uninitialized
//      (xyzint)box_orig = xyzdouble2int((center - (length * 0.5)) + 0.5);
//      (xyzint)box_end =  xyzdouble2int((center + (length * 0.5)) + 0.5);
    }
// keep box on image
    box_orig.x = clip(box_orig.x, orig.x, end.x);
    box_orig.y = clip(box_orig.y, orig.y, end.y);
    box_orig.z = clip(box_orig.z, orig.z, end.z);
    box_orig.i = ibegin;
    box_end.x = clip(box_end.x, orig.x, end.x);
    box_end.y = clip(box_end.y, orig.y, end.y);
    box_end.z = clip(box_end.z, orig.z, end.z);
    box_end.i = iend;
// create box template
    shape_image=new psyimgbox(base_image, box_orig, box_end);
// set background and foreground values
    if(background_infile_flag)shape_image->set_in_image_as_background();
    else shape_image->set_background_value(background_value);
    shape_image->set_foreground_value(foreground_value);
    imgptr = shape_image;
  }
  else if(sphere_set) {
    if(ellipsoid_set){
      cerr << argv[0] << ": mutually exclusive options entered\n";
      exit(1);
    }
    shape_image=new psyimgsphere(base_image, radius,
				 center.x, center.y, center.z);
    ((psyimgsphere *) shape_image)->setirange(ibegin, iend);
// set background and foreground values
    if(background_infile_flag)shape_image->set_in_image_as_background();
    else shape_image->set_background_value(background_value);
    shape_image->set_foreground_value(foreground_value);
    imgptr = shape_image;
  }
  else if(cylinder_set) {
    shape_image=new psyimgcylinder(base_image, radius, center.x, center.y, axis);
    ((psyimgcylinder *) shape_image)->setirange(ibegin,iend);
// set background and foreground values
    if(background_infile_flag)shape_image->set_in_image_as_background();
    else shape_image->set_background_value(background_value);
    shape_image->set_foreground_value(foreground_value);
    imgptr = shape_image;
  }
  else if(ellipsoid_set) {
    shape_image=new psyimgellipsoid(base_image, center, length);
    ((psyimgellipsoid *) shape_image)->setirange(ibegin,iend);
// set background and foreground values
    if(background_infile_flag)shape_image->set_in_image_as_background();
    else shape_image->set_background_value(background_value);
    shape_image->set_foreground_value(foreground_value);
    imgptr = shape_image;
  }
  else if(halfspace_set) {
    shape_image=new psyimghalfspace(base_image, center, length);
    //    ((psyimghalfspace *) shape_image)->setirange(ibegin,iend);
// set background and foreground values
    if(background_infile_flag)shape_image->set_in_image_as_background();
    else shape_image->set_background_value(background_value);
    shape_image->set_foreground_value(foreground_value);
    imgptr = shape_image;
  }
  else if(ptsfile != NULL) {
    if(background_infile_flag) writable_image = new psywritable(base_image);
    else {
      background_constant_image =
	new psyimgconstant(base_image, background_value, outtype);
      writable_image = new psywritable(background_constant_image);
    }
    char c, instring[1024], *sptr;
    fstream point_fs(ptsfile, ios::in);
    if(point_fs.bad()) {
      cerr<<argv[0]<<": unable to open points file="<<ptsfile<<'\n';
      exit(1);
    }

    instring[0]='\0'; // in case empty file - forces sscanf to fail
    int string_location;
    char c1, c2;
    while(1) {
      point_fs.get(instring, 1023, '\n');
      if(!point_fs.good())break;
      sptr=instring;
      if(sptr[0] == '(') sptr++;
      xyzint point;
      if(sscanf(sptr, "%ld%c%ld%c%ld%n", &point.x, &c1,
		&point.y, &c2, &point.z,
		&string_location) == 5) {
	int i = 0;
	writable_image->setpixel((char *) &foreground_value,
				 psydouble,
				 point.x, point.y, point.z, 0);
      }
// clean up to carriage returns
      while(point_fs.peek() == '\n'){point_fs.get(c);}
      if(point_fs.eof() || !point_fs.good())break;
    }// end while
    imgptr = writable_image;
  }
  else if(alternate_set) {
    shape_image=new psyimgalt(base_image, alternate);
// set background and foreground values
    if(background_infile_flag)shape_image->set_in_image_as_background();
    else shape_image->set_background_value(background_value);
    shape_image->set_foreground_value(foreground_value);
    imgptr = shape_image;
  }
  else {
    cerr << argv[0] << ": no shape specified\n";
    exit(1);
  }

// build new description
  desc = new char[strlen("bldshape") +1];
  strcpy(desc, "bldshape");
  imgptr->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  imgptr->setdate();
  imgptr->settime();
// output result to outfileclass file
  psyimg *outpsyimgptr=psynewoutfile(outfile, imgptr,
				     outfileclass, outtype);
// log
  logfile log(outfile, argc, argv);
  if(infile!=NULL)log.loginfilelog(infile);

// clean up
  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(shape_image != NULL)delete shape_image;
  if(writable_image != NULL)delete writable_image;
  if(background_constant_image != NULL)delete background_constant_image;
  if(base_image != NULL)delete base_image;
  if(inimage != NULL)delete inimage;
}
