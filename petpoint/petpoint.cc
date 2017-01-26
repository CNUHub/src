#include "psyhdr.h"
#include "talairach.h"
#include <stdio.h>
#include <iomanip>

int main(int argc, char *argv[])
{
  char *talairach_image_file=NULL;
  char *nontalairach_image_file=NULL;
  char *mapfile=NULL;
  char *point_file=NULL;
  xyzdouble point, location;
  psyimg *inimageptr;
  xyzdouble factor, translate;
  point.x=point.y=point.z=0;
  factor.x=factor.y=factor.z=1;
  translate.x=translate.y=translate.z=0;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<5) {
      cout<<"Usage: "<<argv[0];
      cout<<"  [-tf talairach_image_file | -nf nontalairach_image_file]\n";
      cout<<"  [-mf mapfile]] [-p x,y,z | -pf point_file]\n";
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
  if(talairach_image_file != NULL)
    inimageptr = psynewinfile(talairach_image_file);
  else if(nontalairach_image_file != NULL)
    inimageptr = psynewinfile(nontalairach_image_file);
  else {
    cerr<<argv[0]<<" no image file specified\n";
    exit(1);
  }

  to_talairach talairpt(inimageptr);

// get talairach parameters for nontalairach_image_file
  if(nontalairach_image_file)talairpt.mappinginput(mapfile);

  if(point_file == NULL || point.x != 0 || point.y != 0 || point.z != 0) {
// convert the single input point
// convert to m
    point = point * .001;
    point=talairpt.AtlasToTomo(point);
    talairpt.tomo2indice(point.x, point.y, point.z,
			&location.x, &location.y, &location.z);

    location = (factor*location) + translate;
    // output point
    //    cout.form("location=(%10.7lg,%10.7lg,%10.7lg)pixels\n",
    //       location.x, location.y, location.z);
    cout<<"location=(";
    cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.x;
    cout<<',';
    cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.y;
    cout<<',';
    cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.z;
    cout<<")pixels\n";
  }

  if(point_file != NULL) {
    char c, instring[1024], *sptr;
    fstream point_fs(point_file, ios::in);
    if(point_fs.bad()) {
      cerr<<argv[0]<<": unable to open point_file="<<point_file<<'\n';
      exit(1);
    }

    instring[0]='\0'; // in case empty file - forces sscanf to fail
    while(1) {
      point_fs.get(instring, 1023, '\n');
      if(!point_fs.good())break;
      if(sscanf(instring, "%lf %lf %lf", &point.x, &point.y, &point.z) == 3) {
	point = point * .001;
	point=talairpt.AtlasToTomo(point);
	talairpt.tomo2indice(point.x, point.y, point.z,
			     &location.x, &location.y, &location.z);
	location = (factor*location) + translate;
// output point
	//cout.form("%10.7lg %10.7lg %10.7lg",
	//	  location.x, location.y, location.z);
	cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.x;
	cout<<' ';
	cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.y;
	cout<<' ';
	cout<<setiosflags(ios::scientific)<<setprecision(7)<<setw(10)<<location.z;
// skip point strings
	strtok(instring, " ");
	strtok(NULL, " ");
	strtok(NULL, " ");
	// echo remainder of line
	while((sptr=strtok(NULL, " ")) != NULL){
	  cout<<" "<<sptr;
	}
      }
      else cout<<instring;
// clean up and output carriage returns
      while(point_fs.peek() == '\n'){point_fs.get(c); cout<<c;}
      if(point_fs.eof() || !point_fs.good())break;
    }// end while
  }// end if(point_file != NULL)

  delete inimageptr;
}
