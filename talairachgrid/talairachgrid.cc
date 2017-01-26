#include <math.h>
#include "psyhdr.h"
#include "talairach.h"

class gridimage : public psypgbuff {
  xyzdouble ac, pc;
  double front, back, left, right, top, bottom;
  psyorient displaygridtype;
  int nsagittal;
  int *sagittal_planes;
  int ncoronal;
  int *coronal_planes;
  int ntransverse;
  int *transverse_planes;
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  gridimage(psyimg *psyimgptr, psyorient gridtype,
	    xyzint *reverse_flag=NULL);
  virtual ~gridimage();
};

gridimage::gridimage(psyimg *psyimgptr, psyorient gridtype,
		     xyzint *reverse_flag)
     : psypgbuff(psyimgptr, 1)
{
  int j;
  double dtemp;
  if(gettype() != psyuchar) {
    cerr<<"gridimage::gridimage - sorry only works with unsigned char data\n";
    exit(1);
  }
  displaygridtype=gridtype;
// define extents of brain
  left=(orig.x + end.x - (TALAIRACH_WIDTH/res.x))/2.0;
  right=left + (TALAIRACH_WIDTH/res.x);
  front=(orig.y + end.y + (TALAIRACH_LENGTH/res.y))/2.0;
  back=front - (TALAIRACH_LENGTH/res.y);
  top=(orig.z + end.z - (TALAIRACH_HEIGHT/res.z))/2.0;
  bottom=top + (TALAIRACH_HEIGHT/res.z);
  if(reverse_flag != NULL) {
    if(reverse_flag->x) { dtemp=left; left=right; right=dtemp; }
    if(reverse_flag->y) { dtemp=front; front=back; back=dtemp; }
    if(reverse_flag->z) { dtemp=top; top=bottom; bottom=dtemp; }
  }
  cout<<"left,right="<<left<<','<<right<<'\n';
  cout<<"front,back="<<front<<','<<back<<'\n';
  cout<<"top,bottom="<<top<<','<<bottom<<'\n';
// locate ac and pc line
  ac.x = pc.x = left + ((right - left)/2.0);
  ac.y = front + ((back - front)*(TALAIRACH_FRONT_AC_LENGTH/TALAIRACH_LENGTH));
  pc.y = ac.y + ((back - front)*(TALAIRACH_ACPC_LENGTH/TALAIRACH_LENGTH));
  ac.z = pc.z = bottom + ((top - bottom)*(TALAIRACH_ACPC_HEIGHT/TALAIRACH_HEIGHT));

  nsagittal=9;
  sagittal_planes=new int[nsagittal];
  for(j=0; j<nsagittal; j++) {
    sagittal_planes[j]= irint(left + (j*(right-left)/8.0));
  }
  ncoronal=10;
  coronal_planes=new int[ncoronal];
  for(j=0; j<5; j++) {
    coronal_planes[j]=irint(back + (j*(pc.y-back)/4.0));
  }
  for(j=5; j<ncoronal; j++) {
    coronal_planes[j]= irint(ac.y + ((j-5)*(front-ac.y)/4.0));
  }
  ntransverse=1;
  transverse_planes=new int[ntransverse];
  transverse_planes[0]=irint(ac.z);
/*
  ntransverse=13;
  transverse_planes=new int[ntransverse];
  for(j=0; j<9; j++) {
    transverse_planes[j]= irint(top + ( j*(ac.z - top)/8.0));
  }
  for(j=9; j<13; j++) {
    transverse_planes[j]= irint(ac.z + ( (j-8)*(bottom-ac.z)/4.0));
  }
*/
}

gridimage::~gridimage()
{
  if(nsagittal > 0)delete[] sagittal_planes;
  if(ncoronal > 0)delete[] coronal_planes;
  if(ntransverse > 0)delete[] transverse_planes;
}

void gridimage::fillpage(char *buff, int z, int i)
{
  int j, x, y, ystart, ylast, xstart, xlast;
  int initialoffset;
  unsigned char *ptr;
  initialoffset=offset(orig.x, orig.y, orig.z, orig.i);
// let psypgbuff do initial fill
  psypgbuff::fillpage(buff, z, i);
// don't draw above top of head or below bottom of head
//*****************************************************
  if(z < top || z > bottom)return;
//*****************************************************
// draw sagittal lines
  ystart = irint((front<back)?front:back);
  ystart = (ystart < orig.y) ? orig.y : ystart;
  ylast = irint((back>front)?back:front);
  ylast = (ylast > end.y) ? end.y : ylast;
  if(displaygridtype == psytransverse || displaygridtype == psycoronal) {
    ptr=(unsigned char *)buff;
    for(y=ystart+(z%2); y<=ylast; y+=2) {
      for(j=0; j<nsagittal; j++) {
	if(sagittal_planes[j] >= orig.y && sagittal_planes[j] <= end.y) {
	  ptr[offset(sagittal_planes[j], y, orig.z, orig.i)-initialoffset]=255;
	}
      }
    }
  }
// draw coronal lines
  xstart = irint((left<right)?left:right);
  xstart = (xstart<orig.x)? orig.x : xstart;
  xlast = irint((right>left)?right:left);
  xlast = (xlast>end.x)? end.x : xlast;
  if(displaygridtype == psytransverse || displaygridtype == psysagittal) {
    xstart += z%2;
    for(j=0; j<ncoronal; j++) {
      if(coronal_planes[j] >= orig.y && coronal_planes[j] <= end.y){
	ptr=(unsigned char *)buff + 
	  offset(orig.x, coronal_planes[j], orig.z, orig.i)-initialoffset;
	for(x=xstart; x<=xlast; x+=2)ptr[x]=255;
      }
    }
    xstart -= z%2;
  }
// draw transverse lines
  if(displaygridtype == psycoronal || displaygridtype == psysagittal) {
    ptr=(unsigned char *)buff;
    for(j=0; j<ntransverse; j++) {
      if(transverse_planes[j] == z) {
	for(y=ystart; y<ylast; y++)
	  for(x=xstart+y%2; x<xlast; x+=2)
            ptr[offset(x,y,orig.z,orig.i)-initialoffset]=255;
	break;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean;
  char *desc;
  psytype outtype=psyuchar;
  psyfileclass outfileclass=psynoclass; //defaults to input class
  psyorient gridtype=psytransverse;
  xyzint reverse_flag;
  reverse_flag.x=0; reverse_flag.y=0; reverse_flag.z=0;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile\n";
      cout <<"       [-t[ransverse]] [-s[agital]] [-c[oronal]]\n";
      cout <<"       [-yr[everse]] [-zr[everse]]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strncmp(argv[i], "-t", 2)==0)gridtype=psytransverse;
    else if(strncmp(argv[i], "-s", 2)==0)gridtype=psysagittal;
    else if(strncmp(argv[i], "-c", 2)==0)gridtype=psycoronal;
    else if(strncmp(argv[i], "-xr", 3)==0)reverse_flag.x=1;
    else if(strncmp(argv[i], "-yr", 3)==0)reverse_flag.y=1;
    else if(strncmp(argv[i], "-zr", 3)==0)reverse_flag.z=1;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": invalid parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input files
  psyfileclass infileclass;
  psyimg *inimag=psynewinfile(infile, &infileclass);
  if(outfileclass==psynoclass)outfileclass=infileclass;
// buffer input files
  psypgbuff inputbuffered(inimag, 1);
// convert input image to unsigned char with max value of 254
// scale image by largest absolute value
  inputbuffered.getstats(&min, &max, &mean);
  min=fabs(min); max=fabs(max);
  max = (max > min)? max : min;
  if(fabs(max) < 1e-16) max = 254;
  double scale_factor = 254.0/max;
  cout<<"scale_factor="<<scale_factor<<'\n';
  scaleimg inputscaled((psyimg *)&inputbuffered, outtype, scale_factor, 0.0);
//  inputscaled.set_scale_factor_for_max_res();
// apply grid
  gridimage imagegridded((psyimg *)&inputscaled, gridtype, &reverse_flag);
// build new description
  desc = new char[strlen(infile) + 17];
  strcpy(desc, "talairachgrid: ");
  strcat(desc, infile);
  imagegridded.setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  imagegridded.setdate();
  imagegridded.settime();
// output result to analyze file
  psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg*)&imagegridded,
				     outfileclass, outtype);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
  if(outpsyimgptr != NULL)delete outpsyimgptr;
  if(inimag != NULL)delete inimag;
}
