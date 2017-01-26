#include "psyhdr.h"
#include "psyiofunctions.h"
#include "psytools.h"
#include <math.h>

enum mathoperation { sumoperation=0,
		     minoperation, maxoperation,
		     avgoperation, medianoperation
};

class multiimgmath : public psypgbuff {
 protected:
  int num_images;
  int num_images_is_odd;
  int median_index;
  psyimg **image_ptrs;
  mathoperation operation;
  void fillpage(char *buff, int z, int i);
 public:
  multiimgmath(psyimg *image_ptrs[], int num_images, mathoperation operation);
  ~multiimgmath();
};

multiimgmath::multiimgmath(psyimg *image_ptrs[], int num_images,
			   mathoperation operation)
{
  initpgbuff(image_ptrs[0], 1, psydouble);
  setwordres(1.0);
  multiimgmath::num_images=num_images;
  num_images_is_odd = num_images % 2;
  median_index = (num_images - 1)/2;
  multiimgmath::image_ptrs = new psyimg *[num_images];
  for(int i=0; i<num_images; i++)
    multiimgmath::image_ptrs[i] = image_ptrs[i];
  multiimgmath::operation = operation;
}

multiimgmath::~multiimgmath()
{
  if(image_ptrs != NULL)delete[] image_ptrs;
}

void multiimgmath::fillpage(char *buff, int z, int i)
{
  int x, y, n;
  char *xoptr, *yoptr;
  double x_value, dtemp;
  char_or_largest_pixel pixel;
  psytype imgtype[num_images];
  double wordres[num_images];

  for(n=0; n<num_images; n++) {
    imgtype[n]=image_ptrs[n]->gettype();
    wordres[n]=image_ptrs[n]->getwordres();
    image_ptrs[n]->initgetpixel(imgtype[n], orig.x, orig.y, z, i);
  }

  y=orig.y; yoptr=buff;
  // decide on operation outside of loop for speed
  switch(operation) {
  case sumoperation:
  case avgoperation:
    for(; y<=end.y; y++, yoptr += inc.y) {
      for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
	dtemp = 0;
	for(n=0; n<num_images; n++) {
	  image_ptrs[n]->getnextpixel(pixel.c);
	  type2double(pixel.c, imgtype[n], (char *)&x_value);
	  x_value *= wordres[n];
	  dtemp += x_value;
	}
	if(operation == avgoperation) dtemp /= num_images;
	pixeltypechg((char *)&dtemp, psydouble, xoptr, type);
      } // end for(x=...)
    } // end for(y=...)
    break;
  case minoperation:
    for(; y<=end.y; y++, yoptr += inc.y) {
      for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
	// initialize dtemp based on first image
	image_ptrs[0]->getnextpixel(pixel.c);
	type2double(pixel.c, imgtype[0], (char *)&dtemp);
	dtemp *= wordres[0];
	// now work with remaining images
	for(n=1; n<num_images; n++) {
	  image_ptrs[n]->getnextpixel(pixel.c);
	  type2double(pixel.c, imgtype[n], (char *)&x_value);
	  x_value *= wordres[n];
	  dtemp = (x_value < dtemp) ? x_value : dtemp;
	}
	pixeltypechg((char *)&dtemp, psydouble, xoptr, type);
      } // end for(x=...)
    } // end for(y=...)
    break;
  case maxoperation:
    for(; y<=end.y; y++, yoptr += inc.y) {
      for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
	// initialize dtemp based on first image
	image_ptrs[0]->getnextpixel(pixel.c);
	type2double(pixel.c, imgtype[0], (char *)&dtemp);
	dtemp *= wordres[0];
	// now work with remaining images
	for(n=1; n<num_images; n++) {
	  image_ptrs[n]->getnextpixel(pixel.c);
	  type2double(pixel.c, imgtype[n], (char *)&x_value);
	  x_value *= wordres[n];
	  dtemp = (x_value > dtemp) ? x_value : dtemp;
	}
	pixeltypechg((char *)&dtemp, psydouble, xoptr, type);
      } // end for(x=...)
    } // end for(y=...)
    break;
  case medianoperation:
    {
      double sortarray[num_images];
      for(; y<=end.y; y++, yoptr += inc.y) {
	for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
	  for(n=0; n<num_images; n++) {
	    image_ptrs[n]->getnextpixel(pixel.c);
	    type2double(pixel.c, imgtype[n], (char *)&x_value);
	    x_value *= wordres[n];
	    sortarray[n] = x_value * wordres[n];
	  }
	  shellsort(num_images, sortarray);
	  if(num_images_is_odd) dtemp = sortarray[median_index];
	  else dtemp = 0.5*(sortarray[median_index]+sortarray[median_index+1]);
	  pixeltypechg((char *)&dtemp, psydouble, xoptr, type);
	} // end for(x=...)
      } // end for(y=...)
    }
    break;
  }

  for(n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
  }
}

int main(int argc, char *argv[])
{
  char *outfile=NULL;
  char *listfile=NULL;
  psytype outtype=psynotype; // will default to largest input type
  psytype biggestintype=psynotype;
  psyfileclass outfileclass=psynoclass; // defaults to first input class
  psypgbuff *newpsypgbuff=NULL;
  int numinputfiles=0;
  int across_dim= -1;
  mathoperation operation=sumoperation;
// allocate a name list large enough to point to all input file names //
  string commandlinelist[argc];
  string *namelist=commandlinelist;

// parse input parameters
  int i;
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" -list listfile | infile1 [infile2 [infile3 [...]]]\n";
      cout<<" [-sum | -avg | -median | -min | -max]\n";
      cout<<" [-across_idim | -across_xdim | -across_ydim | -across_zdim] -o outfile\n";
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(strcmp("-min", argv[i])==0)operation = minoperation;
    else if(strcmp("-max", argv[i])==0)operation = maxoperation;
    else if(strcmp("-sum", argv[i])==0)operation = sumoperation;
    else if(strcmp("-avg", argv[i])==0)operation = avgoperation;
    else if(strcmp("-median", argv[i])==0)operation = medianoperation;
    else if((strcmp("-list", argv[i])==0) && ((i+1)<argc))listfile=argv[++i];
    else if(strcmp("-across_xdim", argv[i])==0)across_dim=0;
    else if(strcmp("-across_ydim", argv[i])==0)across_dim=1;
    else if(strcmp("-across_zdim", argv[i])==0)across_dim=2;
    else if(strcmp("-across_idim", argv[i])==0)across_dim=3;
    else if((strcmp("-o", argv[i])==0) && ((i+1)<argc))outfile=argv[++i];
    else if(strncmp("-", argv[i], 1) == 0) {
      cerr<<argv[0]<<": error invalid parameter="<<argv[i]<<'\n';
      exit(1);
    }
    else {
      namelist[numinputfiles]=argv[i];
      numinputfiles++;
    }
  }

// verify input files
  if(numinputfiles < 1 && listfile==NULL) {
    cerr<<argv[0]<<" - no input files specified\n";
    exit(1);
  }
  if(numinputfiles != 0 && listfile != NULL) {
    cerr<<argv[0]<<" - input files not allowed with listfile specified\n";
    exit(1);
  }

// verify output file name set
  if(outfile == NULL) {
    cerr<<argv[0]<<" - no output file specified\n";
    exit(1);
  }

  if(listfile != NULL) {
// read input list
    namelist=read_list(listfile, &numinputfiles);
  }

// verify at least one input file
  if(numinputfiles < 1) {
    cerr<<argv[0]<<" - no input files specified\n";
    exit(1);
  }
  if(numinputfiles != 1 && (across_dim > -1)) {
    cerr<<argv[0]<<" - only 1 input file allowed with across dim options\n";
    exit(1);
  }

// allocate a psyimg list
  psyimg **imageptrs;

  if(across_dim > -1) {
    psyfileclass infileclass;
    psyimg *imgptr=psynewinfile(namelist[0], &infileclass, &biggestintype);
    if(across_dim == 0) imgptr=new psypgbuff(imgptr,1);
    if(outfileclass == psynoclass)outfileclass=infileclass;
    psydims in_beg=imgptr->getorig();
    psydims in_end=imgptr->getend();
    psydims in_size=imgptr->getsize();
    // use psypgbuff to avoid message: 
    //   psyimg::psyimglnk::ecatfile:ecatfile::copyblock - invalid extents requested
    //   copies only full planes(use psypgbuff)
    if(across_dim < 2) imgptr=new psypgbuff(imgptr,in_size.z);

    int begin[4];
    begin[0] = in_beg.x; begin[1]=in_beg.y; begin[2]=in_beg.z; begin[3]=in_beg.i;
    int end[4];
    end[0] = in_end.x; end[1]=in_end.y; end[2]=in_end.z; end[3]=in_end.i;
    int size[4];
    size[0] = in_size.x; size[1]=in_size.y; size[2]=in_size.z; size[3]=in_size.i;

    numinputfiles = size[across_dim];
    imageptrs = new psyimg*[numinputfiles];
    int acrossdimbeg = begin[across_dim];
    int acrossdimend = end[across_dim];
    for(int j=acrossdimbeg; j<=acrossdimend; j++) {
// select block of image
      begin[across_dim] = end[across_dim] = j;
      psyimg *newimgptr = new psyimgblk(imgptr, begin[0], begin[1], begin[2], begin[3],
					end[0], end[1], end[2], end[3], 1);
// buffer each input j to improve accumulation speed
      newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
// keep a pointer to this buffer
      imageptrs[j]=(psyimg *)newpsypgbuff;
    }
  }
  else {
    imageptrs = new psyimg*[numinputfiles];
    for(i=0; i<numinputfiles; i++) {
// open links to each input file
      psyfileclass infileclass;
      psytype intype;
      psyimg *newimgptr=psynewinfile(namelist[i], &infileclass, &intype);
// buffer each input file to improve accumulation speed
      newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
// keep a pointer to this buffer
      imageptrs[i]=(psyimg *)newpsypgbuff;
// note largest type
      if(biggestintype == psynotype)biggestintype=intype;
      else if(gettypebytes(biggestintype) < gettypebytes(intype))
	biggestintype=intype;
      if(outfileclass == psynoclass)outfileclass=infileclass;
    }
  }

  psyimg *psyimgptr = NULL;
// now calculate
  multiimgmath *mathimg = NULL;
  mathimg = new multiimgmath(imageptrs, numinputfiles, operation);
  psyimgptr = (psyimg *)mathimg;
// set description for output image
  switch(operation) {
  case sumoperation:
    psyimgptr->setdescription("Voxel sums accumulated across images");
    break;
  case avgoperation:
    psyimgptr->setdescription("Voxel average accumulated across images");
    break;
  case medianoperation:
    psyimgptr->setdescription("Voxel median found across images");
    break;
  case minoperation:
    psyimgptr->setdescription("Minimum voxels found across images");
    break;
  case maxoperation:
    psyimgptr->setdescription("Maximum voxels found across images");
    break;
  }
// set date and time to current date and time
  psyimgptr->settime();
  psyimgptr->setdate();

  if(outfile != NULL) {
// output result to file
    if(outtype==psynotype)outtype=biggestintype;
    psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr,
				       outfileclass, outtype);
// log
    logfile log(outfile, argc, argv);

// print out images stats
    double min, max, mean;
    outpsyimgptr->getstats(&min, &max, &mean);
    cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outpsyimgptr;
  }

// clean up
  for(i=0; i<numinputfiles; i++){
    delete ((psypgbuff *)imageptrs[i])->inputpsyimg;
    delete imageptrs[i];
  }
  if(namelist != commandlinelist) delete[] namelist;
  if(mathimg != NULL) delete mathimg;
}
