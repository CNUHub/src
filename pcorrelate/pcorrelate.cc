#include "psyhdr.h"
#include "psyiofunctions.h"
#include <math.h>
#include "psytools.h"

class pcorrelate : public psypgbuff {
 protected:
  int num_images;
  psyimg **image_ptrs;
  double *y_diff_array;
  double sum_sqr_dev_y;
  double std_dev_y;
  void fillpage(char *buff, int z, int i);
 public:
  pcorrelate(psyimg *image_ptrs[], double y_array[], int num_images,
	     double mean_y=0);
  ~pcorrelate();
};

pcorrelate::pcorrelate(psyimg *image_ptrs[], double y_array[], int num_images,
		       double mean_y)
{
  initpgbuff(image_ptrs[0], 1, psydouble);
  // jtlee 02/20/2015 -- playing with scaling images independently showed wordres of first image created a scale factor for whole output image
  // jtlee 02/20/2015 -- basic mistake when initializing psyimglnk class with one of multiple images
  setwordres(1.0); // jtlee 02/20/2015
  pcorrelate::num_images=num_images;
  pcorrelate::image_ptrs = new psyimg *[num_images];
  y_diff_array = new double[num_images];
  sum_sqr_dev_y=0;
  for(int i=0; i<num_images; i++) {
    pcorrelate::image_ptrs[i] = image_ptrs[i];
    y_diff_array[i] = y_array[i] - mean_y;
    sum_sqr_dev_y += y_diff_array[i] * y_diff_array[i];
  }
  std_dev_y = sqrt(sum_sqr_dev_y);
}

pcorrelate::~pcorrelate()
{
  if(image_ptrs != NULL)delete[] image_ptrs;
  if(y_diff_array != NULL)delete[] y_diff_array;
}

void pcorrelate::fillpage(char *buff, int z, int i)
{
  int x, y, n;
  char *xoptr, *yoptr;
  double x_mean, x_value;
  double sumsqrx, sumsqrxy;
  double x_diff, dtemp;
  char_or_largest_pixel pixel;
  psytype imgtype[num_images], mean_image_type;
  double x_array[num_images];

  for(n=0; n<num_images; n++) {
    imgtype[n]=image_ptrs[n]->gettype();
// jtlee 02/20/2015 image_ptrs[n]->initgetpixel(imgtype[n], orig.x, orig.y, z, i);
// jtlee 02/20/2015 noticed all inputs run through psypgbuff which can perform conversion to double
// jtlee 02/20/2015 should be more efficient and possibly accurate with scaled images
    image_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
  }
  yoptr=buff;
  for(y=orig.y, yoptr=buff; y<=end.y; y++, yoptr += inc.y) {
    for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
// accumulate stats in two passes
      x_mean=0;
      sumsqrx=0; sumsqrxy=0;
      for(n=0; n<num_images; n++) {
// jtlee 02/20/2015 	image_ptrs[n]->getnextpixel(pixel.c);
// jtlee 02/20/2015	type2double(pixel.c, imgtype[n], (char *)&x_value);
 	image_ptrs[n]->getnextpixel((char *)&x_value);
	x_array[n]=x_value;
	x_mean += x_value;
      }
      x_mean /= num_images;
      for(n=0; n<num_images; n++) {
	x_diff=(x_array[n] - x_mean);
        sumsqrx += x_diff * x_diff;
	sumsqrxy += x_diff * y_diff_array[n];
      }
      dtemp = sqrt(sumsqrx) * std_dev_y;
      if(fabs(dtemp) < 1e-16)dtemp=0;
      else dtemp = sumsqrxy / dtemp;
      pixeltypechg((char *)&dtemp, psydouble, xoptr, type);
    } // end for(x=...)
  } // end for(y=...)
  for(n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
  }
}


// start pcorrelate2

class pcorrelate2 : public psypgbuff {
 protected:
  int num_images;
  psyimg **image_ptrs;
  psyimg **template_ptrs;
  double *y_array;
  double thresh;
  int mincount;

  void fillpage(char *buff, int z, int i);
 public:
  pcorrelate2(psyimg *image_ptrs[], double y_array[],
	      psyimg *template_ptrs[], int num_images,
	      double thresh, int mincount);
  ~pcorrelate2();
};

pcorrelate2::pcorrelate2(psyimg *image_ptrs[], double y_array[],
			 psyimg *template_ptrs[], int num_images,
			 double thresh, int mincount)

{
  initpgbuff(image_ptrs[0], 1, psydouble);
  // jtlee 02/20/2015 -- playing with scaling images independently showed wordres of first image created a scale factor for whole output image
  // jtlee 02/20/2015 -- basic mistake when initializing psyimglnk class with one of multiple images
  setwordres(1.0); // jtlee 02/20/2015
  pcorrelate2::num_images=num_images;
  pcorrelate2::image_ptrs = new psyimg *[num_images];
  pcorrelate2::template_ptrs = new psyimg *[num_images];
  pcorrelate2::y_array = new double[num_images];
  pcorrelate2::thresh = thresh;
  pcorrelate2::mincount = mincount;

  for(int i=0; i<num_images; i++) {
    pcorrelate2::image_ptrs[i] = image_ptrs[i];
    pcorrelate2::template_ptrs[i] = template_ptrs[i];
    pcorrelate2::y_array[i] = y_array[i];
  }
}

pcorrelate2::~pcorrelate2()
{
  if(image_ptrs != NULL)delete[] image_ptrs;
  if(template_ptrs != NULL)delete[] template_ptrs;
  if(y_array != NULL)delete[] y_array;
}

void pcorrelate2::fillpage(char *buff, int z, int i)
{
  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
    template_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
  }

  char *yoptr=buff;
  for(int y=orig.y; y<=end.y; y++, yoptr += inc.y) {

    char *xoptr = yoptr;
    for(int x=orig.x; x<=end.x; x++, xoptr += inc.x) {

// accumulate stats in two passes
      int count = 0;

      double sum_x = 0;
      double sum_sqr_dev_x = 0;
      double sum_y = 0;
      double sum_sqr_dev_y = 0;
      double sum_xy_dev = 0;

      for(int n=0; n<num_images; n++) {
	double x_value;
	image_ptrs[n]->getnextpixel((char *)&x_value);
	double template_value;
	template_ptrs[n]->getnextpixel((char *)&template_value);
	if(template_value >= thresh) {
	  sum_x += x_value;
	  sum_sqr_dev_x += x_value * x_value;
	  sum_y += y_array[n];
	  sum_sqr_dev_y += y_array[n] * y_array[n];
	  sum_xy_dev += x_value * y_array[n];
	  count++;
	}
      }
      double pcorr = 0;
      if(count >= mincount) {
	double mean_x = sum_x / count;
	sum_sqr_dev_x -= sum_x * mean_x;
	sum_sqr_dev_y -= sum_y * sum_y / count;
	sum_xy_dev -= sum_y * mean_x;
	double denominator = sqrt(sum_sqr_dev_x) * sqrt(sum_sqr_dev_y);
	if(denominator > 1e-16) pcorr = sum_xy_dev/denominator;
      }
      pixeltypechg((char *)&pcorr, psydouble, xoptr, type);
    } // end for(x=...)
  } // end for(y=...)
  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
    template_ptrs[n]->freegetpixel();
  }
}
// end pcorrelate2

class panova : public pcorrelate {
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  panova(psyimg *image_ptrs[], double y_array[], int num_images,
	     double mean_y=0);
};

panova::panova(psyimg *image_ptrs[], double y_array[], int num_images,
	     double mean_y) : pcorrelate(image_ptrs, y_array,
					   num_images, mean_y) {
}

void panova::fillpage(char *buff, int z, int i)
{
  int x, y, n;
  char *xoptr, *yoptr;
  double x_mean, x_value;
  double sumsqrx, sumsqrxy;
  double x_diff, dtemp;
  char_or_largest_pixel pixel;
  psytype imgtype[num_images], mean_image_type;
  double x_array[num_images];

  for(n=0; n<num_images; n++) {
    imgtype[n]=image_ptrs[n]->gettype();
    image_ptrs[n]->initgetpixel(imgtype[n], orig.x, orig.y, z, i);
  }
  yoptr=buff;
  for(y=orig.y, yoptr=buff; y<=end.y; y++, yoptr += inc.y) {
    for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
// accumulate stats in two passes
      x_mean=0;
      sumsqrx=0; sumsqrxy=0;
      for(n=0; n<num_images; n++) {
	image_ptrs[n]->getnextpixel(pixel.c);
	type2double(pixel.c, imgtype[n], (char *)&x_value);
	x_array[n]=x_value;
	x_mean += x_value;
      }
      x_mean /= num_images;
      for(n=0; n<num_images; n++) {
	x_diff=(x_array[n] - x_mean);
        sumsqrx += x_diff * x_diff;
	sumsqrxy += x_diff * y_diff_array[n];
      }
      if( (num_images < 3) || (sumsqrx < 1e-16) ) dtemp = 0;
      else {
	double regressionSumOfSqrs = sumsqrxy / sumsqrx;
	double residualSumOfSqrs = sum_sqr_dev_y - regressionSumOfSqrs;
        if(residualSumOfSqrs < 1e-16) dtemp = 0;
	else {
	  double residualDF = num_images - 2;
	  dtemp = (regressionSumOfSqrs / residualSumOfSqrs) * residualDF;
	}
      }
      pixeltypechg((char *)&dtemp, psydouble, xoptr, type);
    } // end for(x=...)
  } // end for(y=...)
  for(n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
  }
}

class spearman : public psypgbuff {
 protected:
  int num_images;
  psyimg **image_ptrs;
  double *y_rank_array;
  double *y_tmp_array;
  double *x_tmp_array;
  double y_tie_sum_feature;
  void fillpage(char *buff, int z, int i);
 public:
  spearman(psyimg *image_ptrs[], double y_array[], int num_images);
  ~spearman();
};

spearman::spearman(psyimg *image_ptrs[], double y_array[], int num_images)
{
  initpgbuff(image_ptrs[0], 1, psydouble);
  // jtlee 02/20/2015 -- playing with scaling images independently showed wordres of first image created a scale factor for whole output image
  // jtlee 02/20/2015 -- basic mistake when initializing psyimglnk class with one of multiple images
  setwordres(1.0); // jtlee 02/20/2015
  spearman::num_images=num_images;
  spearman::image_ptrs = new psyimg *[num_images];
  y_rank_array = new double[num_images];
  y_tmp_array = new double[num_images];
  x_tmp_array = new double[num_images];
  double index_array[num_images];
  int i = 0;
  for(; i<num_images; i++) {
    spearman::image_ptrs[i] = image_ptrs[i];
    y_tmp_array[i] = y_array[i];
    index_array[i] = i;
  }
  // sort the y values into increasing order
  shellsort( num_images, y_tmp_array, index_array );
  // calculate the rank values
  crank( num_images, y_tmp_array, &y_tie_sum_feature);
  // rearrange the ranks in the same order as the original data
  for(i=0; i<num_images; i++) {
    y_rank_array[(int) index_array[i]] = y_tmp_array[i];
  }
}

spearman::~spearman()
{
  if(image_ptrs != NULL)delete[] image_ptrs;
  if(y_rank_array != NULL)delete[] y_rank_array;
  if(y_tmp_array != NULL)delete[] y_tmp_array;
  if(x_tmp_array != NULL)delete[] x_tmp_array;
}

void spearman::fillpage(char *buff, int z, int i)
{
  int x, y, n;
  char *xoptr, *yoptr;
  double x_tie_sum_feature;
  double x_value;
  double dtemp;
  char_or_largest_pixel pixel;
  psytype imgtype[num_images];

  for(n=0; n<num_images; n++) {
    imgtype[n]=image_ptrs[n]->gettype();
    // jtlee 02/20/2015 image_ptrs[n]->initgetpixel(imgtype[n], orig.x, orig.y, z, i);
    image_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
  }
  yoptr=buff;
  for(y=orig.y, yoptr=buff; y<=end.y; y++, yoptr += inc.y) {
    for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
      for(n=0; n<num_images; n++) {
// jtlee 02/20/2015	image_ptrs[n]->getnextpixel(pixel.c);
// jtlee 02/20/2015	type2double(pixel.c, imgtype[n], (char *)&x_value);
 	image_ptrs[n]->getnextpixel((char *)&x_value);
	x_tmp_array[n] = x_value;
	y_tmp_array[n] = y_rank_array[n]; // need to reorder ranks
      }
      // get the x ranks
      shellsort(num_images, x_tmp_array, y_tmp_array);
      crank(num_images, x_tmp_array, &x_tie_sum_feature);

      // calculate the spearman correlation coefficient
      dtemp = spear(num_images, y_tmp_array, y_tie_sum_feature,
		    x_tmp_array, x_tie_sum_feature);
      pixeltypechg((char *)&dtemp, psydouble, xoptr, type);

    } // end for(x=...)
  } // end for(y=...)
  for(n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
  }
}

class spearman2 : public psypgbuff {
 protected:
  int num_images;
  psyimg **image_ptrs;
  psyimg **template_ptrs;
  double *y_sorted_array;
  int *y_invert_sort_indices;
  double *all_y_rank_array;
  double *y_tmp_array;
  double *x_tmp_array;
  int *passed_array;
  double all_y_tie_sum_feature;
  double thresh;
  int mincount;

  void fillpage(char *buff, int z, int i);
 public:
  spearman2(psyimg *image_ptrs[], double y_array[],
	    psyimg *template_ptrs[], int num_images,
	    double thresh, int mincount);
  ~spearman2();
};

spearman2::spearman2(psyimg *image_ptrs[], double y_array[],
		     psyimg *template_ptrs[], int num_images,
		     double thresh, int mincount)
{
  initpgbuff(image_ptrs[0], 1, psydouble);
  // jtlee 02/20/2015 -- playing with scaling images independently showed wordres of first image created a scale factor for whole output image
  // jtlee 02/20/2015 -- basic mistake when initializing psyimglnk class with one of multiple images
  setwordres(1.0); // jtlee 02/20/2015
  spearman2::num_images=num_images;
  spearman2::image_ptrs = new psyimg *[num_images];
  spearman2::template_ptrs = new psyimg *[num_images];
  spearman2::thresh = thresh;
  spearman2::mincount = mincount;

  y_sorted_array = new double[num_images];
  passed_array = new int[num_images];
  all_y_rank_array = new double[num_images];
  x_tmp_array = new double[num_images];
  y_tmp_array = new double[num_images];
  y_invert_sort_indices = new int[num_images];
  double y_sorted_indices[num_images];
  for(int i = 0; i<num_images; i++) {
    spearman2::image_ptrs[i] = image_ptrs[i];
    spearman2::template_ptrs[i] = template_ptrs[i];
    y_sorted_array[i] = y_array[i];
    y_sorted_indices[i] = i;
  }
  // sort the y values into increasing order
  shellsort( num_images, y_sorted_array, y_sorted_indices );
  // create the invert sort array and ranked y array
  for(int n = 0; n<num_images; n++) {
    int i = (int) y_sorted_indices[n];
    y_invert_sort_indices[i] = n;
    all_y_rank_array[n] = y_sorted_array[n];
  }
  crank(num_images, all_y_rank_array, &all_y_tie_sum_feature);
}

spearman2::~spearman2()
{
  if(image_ptrs != NULL)delete[] image_ptrs;
  if(y_sorted_array != NULL)delete[] y_sorted_array;
  if(y_invert_sort_indices != NULL)delete[] y_invert_sort_indices;
  if(passed_array != NULL)delete[] passed_array;
  if(x_tmp_array != NULL)delete[] x_tmp_array;
  if(y_tmp_array != NULL)delete[] y_tmp_array;
}

void spearman2::fillpage(char *buff, int z, int i)
{
  char *xoptr, *yoptr;

  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
    template_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
  }
  yoptr=buff;
  int x, y;
  for(y=orig.y, yoptr=buff; y<=end.y; y++, yoptr += inc.y) {
    for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr += inc.x) {
      int cnt = 0;
      for(int n=0; n<num_images; n++) {
	double x_value;
	image_ptrs[n]->getnextpixel((char *)&x_value);
	double template_value;
	template_ptrs[n]->getnextpixel((char *)&template_value);
	int sorted_y_index = y_invert_sort_indices[n];
	if(template_value >= thresh) {
	  cnt++;
	  passed_array[sorted_y_index] = 1;
	  x_tmp_array[sorted_y_index] = x_value;
	}
	else passed_array[sorted_y_index] = 0;
      }
      double dspear = 0;
      if(cnt >= mincount) {
	double y_tie_sum_feature = all_y_tie_sum_feature;
	if(cnt == num_images) {
	  for(int n=0; n<num_images; n++) {
	    y_tmp_array[n] = all_y_rank_array[n];
	  }
	}
	else {
	  // get only the passed values
	  int current = 0;
	  for(int n=0; n<num_images; n++) {
	    if(passed_array[n]) {
	      y_tmp_array[current] = y_sorted_array[n];
	      x_tmp_array[current] = x_tmp_array[n];
	      current++;
	    }
	  }
	  // get the y ranks
	  crank(cnt, y_tmp_array, &y_tie_sum_feature);
	}
	// get the x ranks
	shellsort(cnt, x_tmp_array, y_tmp_array);
	double x_tie_sum_feature;
	crank(cnt, x_tmp_array, &x_tie_sum_feature);
	// calculate the spearman correlation coefficient
	dspear = spear(cnt, y_tmp_array, y_tie_sum_feature,
		       x_tmp_array, x_tie_sum_feature);
      }
      pixeltypechg((char *)&dspear, psydouble, xoptr, type);

    } // end for(x=...)
  } // end for(y=...)
  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
    template_ptrs[n]->freegetpixel();
  }
}

int main(int argc, char *argv[])
{
  char *outfile=NULL;
  char *listfile=NULL;
  char *templatelistfile=NULL;
  char *yfile=NULL;
  char *scalefactorsfile=NULL;
  double min, max, mean;
  double thresh = 0.5;
  int mincount = 3;
  psytype outtype=psynotype; // will default to largest input type
  psytype biggestintype=psynotype;
  psyfileclass outfileclass=psynoclass; // defaults to first input class
  psypgbuff *newpsypgbuff=NULL;
  psydims pcorr;
  pcorr.x=pcorr.y=pcorr.z=-1;
  pcorr.i=0;
  int print_list=0;
  int across_idim=0;
  int anova=0;
  int spear=0;
  int weighted_sum=0;
// allocate a file list large enough to point to all input file names //
  int numinputfiles=0;
  string commandlinefilelist[argc];
  string *namelist=commandlinefilelist;
// allocate a template list large enough to point to all input file names //
  int numtemplates=0;
  int remainder_templates=0;
  string commandlinetemplatelist[argc];
  string *templatelist=commandlinetemplatelist;

// parse input parameters
  int i;
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" -list listfile | infile1 [infile2 [infile3 [...]]]\n";
      cout<<" -p x,y,z,i | -yf y_file [-print_list]\n";
      cout<<" -o outfile\n";
      cout<<" [-templatelist templatelistfile | -template template1 [template2 [template3 [...]]]\n";
      cout<<" [-mincnt mincount] ]\n";
      cout<<" [-anova | -spear | -wght]\n";
      cout<<" [-scale_factors scale_factors_file]\n";
      cout<<" [-accros_idim]\n";
      cout<<" ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout<<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if((strcmp("-p", argv[i])==0) && ((i+1)<argc)){
      int n=sscanf(argv[++i], "%d,%d,%d,%d",
		   &pcorr.x, &pcorr.y, &pcorr.z, &pcorr.i);
      if(n < 3){
	cerr<<argv[0]<<": error - bad point parameter="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((strcmp("-list", argv[i])==0) && ((i+1)<argc))listfile=argv[++i];
    else if((strcmp("-templatelist", argv[i])==0) && ((i+1)<argc))
      templatelistfile=argv[++i];
    else if((strcmp(argv[i],"-mincnt")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%ld",&mincount) != 1) {
	cerr << argv[0] << ": error parsing minimum count: -min_count ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp("-yf", argv[i])==0) && ((i+1)<argc))yfile=argv[++i];
    else if((strcmp("-scale_factors", argv[i])==0) && ((i+1)<argc))scalefactorsfile=argv[++i];
    else if(strcmp("-print_list", argv[i])==0)print_list=1;
    else if(strcmp("-across_idim", argv[i])==0)across_idim=1;
    else if(strcmp("-anova", argv[i])==0)anova=1;
    else if(strcmp("-spear", argv[i])==0)spear=1;
    else if(strcmp("-wght", argv[i])==0)weighted_sum=1;
    else if((strcmp("-o", argv[i])==0) && ((i+1)<argc))outfile=argv[++i];
    else if(strcmp("-template", argv[i])==0)remainder_templates=1;
    else if(strncmp("-", argv[i], 1) == 0) {
      cerr<<argv[0]<<": error invalid parameter="<<argv[i]<<'\n';
      exit(1);
    }
    else if(remainder_templates) {
      templatelist[numtemplates]=argv[i];
      numtemplates++;
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

// verify pixel location specified
  if((yfile == NULL) && 
     (pcorr.x < 0 || pcorr.y < 0 || pcorr.z < 0 || pcorr.i < 0)) {
    cerr<<argv[0]<<" - invalid or missing pixel coordinate\n";
    exit(1);
  }

  if(templatelistfile && (anova || weighted_sum)) {
    cerr<<argv[0]<<" - sorry, templating currently only supported for Pearson and Spearman correlations\n";
    exit(1);
  }

  if(listfile != NULL) {
// read input list
    namelist=read_list(listfile, &numinputfiles);
  }

  if(templatelistfile != NULL) {
    templatelist=read_list(templatelistfile, &numtemplates);
  }

  if((numtemplates) != 0 && (numtemplates != numinputfiles)) {
    cerr<<argv[0]<<" - number of templates not the same as number of input files\n";
    exit(1);
  }

// verify at least one input file
  if(numinputfiles < 1) {
    cerr<<argv[0]<<" - no input files specified\n";
    exit(1);
  }
  if(numinputfiles != 1 && across_idim) {
    cerr<<argv[0]<<" - only 1 input file allowed with across time option\n";
    exit(1);
  }

// allocate a psyimg list
  psyimg **imageptrs = NULL;
  psyimg **templateptrs = NULL;
  if(across_idim) {
    psyfileclass infileclass;
    psyimg *imgptr=psynewinfile(namelist[0], &infileclass, &biggestintype);
    if(outfileclass == psynoclass)outfileclass=infileclass;
    psydims in_beg=imgptr->getorig();
    psydims in_end=imgptr->getend();
    psydims in_size=imgptr->getsize();
    numinputfiles = in_size.i;
    psyimg *tmpptr=NULL;
    if(numtemplates != 0) {
      tmpptr=psynewinfile(templatelist[0]);
      if(! tmpptr->samedim(imgptr)) {
	cerr<<argv[0]<<" - template must have the same dimensions as the image file\n";
	exit(1);
      }
      numtemplates = in_size.i;
      templateptrs = new psyimg*[numtemplates];
    }
    imageptrs = new psyimg*[numinputfiles];
    for(int i=in_beg.i; i<=in_end.i; i++) {
// select block of image
      psyimg *newimgptr = new psyimgblk(imgptr, in_beg.x, in_beg.y,
					in_beg.z, i,
					in_end.x, in_end.y, in_end.z, i, 1);
// buffer each input i to improve accumulation speed
      newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
// keep a pointer to this buffer
      imageptrs[i]=(psyimg *)newpsypgbuff;
      if(templateptrs != NULL) {
// similar treatment of template
	psyimg *newimgptr = new psyimgblk(tmpptr, in_beg.x, in_beg.y,
					  in_beg.z, i,
					  in_end.x, in_end.y, in_end.z, i, 1);
	newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
	templateptrs[i]=(psyimg *)newpsypgbuff;
      }
    }
  }
  else {
    imageptrs = new psyimg*[numinputfiles];
    if(numtemplates != 0) templateptrs = new psyimg*[numtemplates];
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
      if(templateptrs != NULL) {
// open corresponding template file
	newimgptr=psynewinfile(templatelist[i]);
	newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
	templateptrs[i]=(psyimg *)newpsypgbuff;
      }
    }
  }

  if(scalefactorsfile != NULL) {
// read scale factors from a file
    double factors_array[numinputfiles];
    fstream fd(scalefactorsfile, ios::in);
    if(fd.fail() || fd.bad()) {
      cerr<<argv[0]<<" - unable to open file "<<scalefactorsfile<<'\n';
      exit(1);
    }
    int i=0;
    char linebuff[1024], c;
    while(! fd.eof() && fd.good()) {
      //remove left over carriage return
      if(fd.peek() == '\n')fd.get(c);
      fd.get(linebuff, 1024-1);
      if(fd.good()) {
	if(linebuff[0] == '#' ||
	   linebuff[0] == '\0') ;//ignore blank & comment lines
	else if(i >= numinputfiles) {
	  cerr<<argv[0]<<" - warning - number of input values in "<<scalefactorsfile;
	  cerr<<" greater then number of input images\n";
	}
	else {
	  factors_array[i]=atof(linebuff);
	  imageptrs[i] = (psyimg *) new scaleimg(imageptrs[i], psydouble, factors_array[i], 0.0); 
	  i++;
	}
      }// end if(fd.good())
    }// end while
    if(i != numinputfiles) {
      cerr<<argv[0]<<" - not enough input values in "<<scalefactorsfile<<'\n';
      exit(1);
    }
  }

// get y values
  double y_array[numinputfiles];
  double mean_y=0;
  if(yfile == NULL) {
    char_or_largest_pixel pixel;
    for(i=0; i<numinputfiles; i++) {
      imageptrs[i]->getpixel(pixel.c, imageptrs[i]->gettype(),
			     pcorr.x, pcorr.y, pcorr.z, pcorr.i);
      type2double(pixel.c, imageptrs[i]->gettype(), (char *)&y_array[i]);
      mean_y += y_array[i];
    }
  }
  else {
// read y values from a file
    fstream fd(yfile, ios::in);
    if(fd.fail() || fd.bad()) {
      cerr<<argv[0]<<" - unable to open file "<<yfile<<'\n';
      exit(1);
    }
    int i=0;
    char linebuff[1024], c;
    while(! fd.eof() && fd.good()) {
      //remove left over carriage return
      if(fd.peek() == '\n')fd.get(c);
      fd.get(linebuff, 1024-1);
      if(fd.good()) {
	if(linebuff[0] == '#' ||
	   linebuff[0] == '\0') ;//ignore blank & comment lines
	else if(i >= numinputfiles) {
	  cerr<<argv[0]<<" - warning - number of input values in "<<yfile;
	  cerr<<" greater then number of input images\n";
	}
	else {
	  y_array[i]=atof(linebuff);
	  mean_y += y_array[i];
	  i++;
	}
      }// end if(fd.good())
    }// end while
    if(i != numinputfiles) {
      cerr<<argv[0]<<" - not enough input values in "<<yfile<<'\n';
      exit(1);
    }
  }

  mean_y /= numinputfiles;

// print stuff on y values
  if(print_list) {
    double sumsqrdev=0;
    double dev;
    for(i=0; i<numinputfiles; i++) {
      dev=y_array[i]-mean_y;
      sumsqrdev += dev*dev;
    }
    double stddev = sqrt(sumsqrdev)/numinputfiles;
    for(i=0; i<numinputfiles; i++) {
      cout<<namelist[i];
      cout<<" y["<<i<<"]="<<y_array[i];
      dev=y_array[i]-mean_y;
      cout<<" \tdev="<<dev;
      cout<<" \tdev/std="<<dev/stddev<<'\n';
    }
    cout<<"y mean="<<mean_y<<'\n';
    cout<<"y standard deviation="<<stddev<<'\n';
  }

  psyimg *psyimgptr = NULL;
// now calculate the correlation values
  pcorrelate *pcorrimg = NULL;
  pcorrelate2 *pcorr2img = NULL;
  panova *panovaimg = NULL;
  spearman *spearimg = NULL;
  spearman2 *spear2img = NULL;
  if(spear && (templateptrs != NULL)) {
// Spearman Correlation with templating
    spear2img = new spearman2(imageptrs, y_array,
			      templateptrs, numinputfiles,
			      thresh, mincount);
    psyimgptr = (psyimg *)spear2img;
// set description for output image
    psyimgptr->setdescription("Spearman Correlation on image pixels with templating");
  }
  else if(spear) {
// Spearman Correlation
    spearimg = new spearman(imageptrs, y_array, numinputfiles);
    psyimgptr = (psyimg *)spearimg;
// set description for output image
    psyimgptr->setdescription("Spearman Correlation on image pixels");
  }
  else if(anova) {
// Analysis of variance
    panovaimg = new panova(imageptrs, y_array, numinputfiles, mean_y);
    psyimgptr = (psyimg *)panovaimg;
// set description for output image
    psyimgptr->setdescription("Analysis of variance on image pixels");
  }
  else if(weighted_sum) {
// Weighted sum
    accumulateimgs *acc = new accumulateimgs(psydouble);
    for(int i=0; i<numinputfiles; i++) {
      acc->addimg(imageptrs[i], y_array[i]);
    }
    psyimgptr = (psyimg *) acc;
// set description for output image
    psyimgptr->setdescription("Weighted sum of image pixels");
  }
  else if(templateptrs != NULL) {
// Pearson's Correlation with templating
    pcorr2img = new pcorrelate2(imageptrs, y_array,
				templateptrs, numinputfiles,
				thresh, mincount);
    psyimgptr = (psyimg *)pcorr2img;
// set description for output image
    psyimgptr->setdescription("Pearson's Correlation on image pixels");
  }
  else {
// Pearson's Correlation
    pcorrimg = new pcorrelate(imageptrs, y_array, numinputfiles, mean_y);
    psyimgptr = (psyimg *)pcorrimg;
// set description for output image
    psyimgptr->setdescription("Pearson's Correlation on image pixels");
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
    outpsyimgptr->getstats(&min, &max, &mean);
    cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outpsyimgptr;
  }

// clean up
  if(pcorrimg != NULL) delete pcorrimg;
  if(pcorr2img != NULL) delete pcorr2img;
  if(panovaimg != NULL) delete panovaimg;
  for(i=0; i<numinputfiles; i++){
    // may not be a psypgbuff because of adding scale.  Plus exiting so not really needed?
    // delete ((psypgbuff *)imageptrs[i])->inputpsyimg;
    delete imageptrs[i];
  }
  if(namelist != commandlinefilelist) delete[] namelist;
  if(templatelist != commandlinetemplatelist) delete[] templatelist;
}
