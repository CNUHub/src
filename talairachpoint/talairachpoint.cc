#include "psyhdr.h"
#include "talairach.h"
#include <stdio.h>
#include <iomanip>

int main(int argc, char *argv[])
{
  char *talairach_image_file=NULL;
  char *nontalairach_image_file=NULL;
  char *mapfile=NULL;
  char *stalibfile=NULL;
  char *point_file=NULL;
  xyzdouble point, location;
  psyimg *inimageptr=NULL;
  xyzdouble factor, translate;
  point.x=point.y=point.z=0;
  factor.x=factor.y=factor.z=1;
  translate.x=translate.y=translate.z=0;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<5) {
      cout<<"Usage: "<<argv[0];
      cout<<"  [-tf talairach_image_file | -nf nontalairach_image_file]\n";
      cout<<"  [-mf mapfile | -sta p#_sta.lib] [-p x,y,z | -pf point_file]\n";
      cout<<"  [-xf x_factor] [-yf y_factor] [-zf z_factor]\n";
      cout<<"  [-xt x_translation] [-yt y_translation] [-zt z_translation]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp(argv[i], "-tf")==0)&&((i+1)<argc))
      talairach_image_file=argv[++i];
    else if((strcmp(argv[i], "-nf")==0)&&((i+1)<argc))
      nontalairach_image_file=argv[++i];
    else if((strcmp(argv[i], "-mf")==0)&&((i+1)<argc))mapfile=argv[++i];
    else if((strcmp(argv[i], "-sta")==0)&&((i+1)<argc))stalibfile=argv[++i];
    else if((strcmp(argv[i], "-p")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf,%lf,%lf", &point.x, &point.y, &point.z) != 3) {
	cerr<<argv[0]<<": invalid point parameter\n";
	exit(1);
      }
    }
    else if((strcmp(argv[i], "-pf")==0)&&((i+1)<argc))point_file=argv[++i];
    else if((strcmp(argv[i], "-xf")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf", &factor.x) != 1) {
	cerr<<argv[0]<<": invalid x factor\n";
	exit(1);
      }
    }
    else if((strcmp(argv[i], "-yf")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf", &factor.y) != 1) {
	cerr<<argv[0]<<": invalid y factor\n";
	exit(1);
      }
    }
    else if((strcmp(argv[i], "-zf")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf", &factor.z) != 1) {
	cerr<<argv[0]<<": invalid z factor\n";
	exit(1);
      }
    }
    else if((strcmp(argv[i], "-xt")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf", &translate.x) != 1) {
	cerr<<argv[0]<<": invalid x translation\n";
	exit(1);
      }
    }
    else if((strcmp(argv[i], "-yt")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf", &translate.y) != 1) {
	cerr<<argv[0]<<": invalid y translation\n";
	exit(1);
      }
    }
    else if((strcmp(argv[i], "-zt")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i], "%lf", &translate.z) != 1) {
	cerr<<argv[0]<<": invalid z translation\n";
	exit(1);
      }
    }
    else {
      cerr << argv[0] << ": invalid parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input file
  if((talairach_image_file != NULL) && (nontalairach_image_file != NULL)) {
    cerr << argv[0] << ": error both talairach and nontalairach image file specified\n";
    exit(1);
  }
  else if(talairach_image_file != NULL)
    inimageptr = psynewinfile(talairach_image_file);
  else if(nontalairach_image_file != NULL)
    inimageptr = psynewinfile(nontalairach_image_file);
  else {
    cerr<<argv[0]<<" no image file specified\n";
    exit(1);
  }

  to_talairach talairpt(inimageptr);

// get talairach parameters for nontalairach_image_file
  if(nontalairach_image_file) {
    if(mapfile && stalibfile) {
      cerr << argv[0] << ": error both mapfile and p#_sta.lib specified\n";
      exit(1);
    }
    if(stalibfile) talairpt.mappinginput(stalibfile, satoshi_sta_lib);
    else talairpt.mappinginput(mapfile, standard_mapfile);
  }
  else if(mapfile || stalibfile) {
    cerr << argv[0] << ": warning mapfile(or p#_sta.lib) ignored\n";
  }

  if(point_file == NULL || point.x != 0 || point.y != 0 || point.z != 0) {
// convert the single input point
    point = (factor*point) + translate;
// indice2tomo and indice2atlas are equivalent with MappingInput not
    talairpt.indice2tomo(point.x, point.y, point.z,
			&location.x, &location.y, &location.z);
// TomoToAtlas should do nothing if MappingInput not called
    location=talairpt.TomoToAtlas(location);

    // convert to mm
    location = location * 1000.0;

    // output point
    //    cout.form("location=(%10.7lg,%10.7lg,%10.7lg)mm\n",
    //	       location.x, location.y, location.z);
	      
    cout<<"location=(";
    cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.x;
    cout<<',';
    cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.y;
    cout<<',';
    cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.z;
    cout<<")mm\n";
  }

  if(point_file != NULL) {
    char c, instring[1024], *sptr;
    fstream point_fs(point_file, ios::in);
    if(point_fs.bad()) {
      cerr<<argv[0]<<": unable to open point_file="<<point_file<<'\n';
      exit(1);
    }

    instring[0]='\0'; // in case empty file - forces sscanf to fail
    int string_location;
    char c1, c2;
    while(1) {
      point_fs.get(instring, 1023, '\n');
      if(!point_fs.good())break;
      sptr=instring;
      if(sptr[0] == '('){cout<<sptr[0]; sptr++; }
      if(sscanf(sptr, "%lf%c%lf%c%lf%n", &point.x, &c1,
		&point.y, &c2, &point.z,
		&string_location) == 5) {
	point = (factor*point) + translate;
// indice2tomo and indice2atlas are equivalent with MappingInput not
        talairpt.indice2tomo(point.x, point.y, point.z,
			&location.x, &location.y, &location.z);
// TomoToAtlas should do nothing if MappingInput not called
        location=talairpt.TomoToAtlas(location);
// convert to mm
	location = location * 1000.0;
// output point
//	cout.form("%10.7lg%c%10.7lg%c%10.7lg",
//		  location.x, c1, location.y, c2, location.z);
	cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.x;
	cout<<c1;
	cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.y;
	cout<<c2;
	cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.z;
// skip point strings
        sptr += string_location;
      }
// echo unprocessed portion of line
      cout<<sptr;
// clean up and output carriage returns
      while(point_fs.peek() == '\n'){point_fs.get(c); cout<<c;}
      if(point_fs.eof() || !point_fs.good())break;
    }// end while
  }// end if(point_file != NULL)

  if(inimageptr!=NULL)delete inimageptr;
}
