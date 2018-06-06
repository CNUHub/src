#include "psyhdr.h"
#include "psyiofunctions.h"
#include <math.h>
#include "psytools.h"
#include "cnustats.h"
#include <iomanip>

class abstract_regress : public psypgbuff {
 public:
  virtual void fillpage(int z, int i) = 0;
  virtual psyimg *getAimage() = 0;
  virtual psyimg *getBimage() = 0;
  virtual psyimg *getstdyerrimage() = 0;
  virtual psyimg *getcountimage() = 0;
};

class regresspgbuff : public psypgbuff {
 protected:
  void fillpage(char *buff, int z, int i);
 public:
  regresspgbuff(abstract_regress *parentpgbuff, int maxnumpages,
		psytype pixeltype);
};

regresspgbuff::regresspgbuff(abstract_regress *parentpgbuff,
			     int maxnumpages,
			     psytype pixeltype) {
  initpgbuff(parentpgbuff, maxnumpages, pixeltype);
};

void regresspgbuff::fillpage(char *buff, int z, int i) {
  // this is called by getzptr() and the following will have
  // to call getzptrlocked() and hopefully wont cause a loop
  // because the the original getzptr doesn't call fillpage
  // if the buffer already exists.
  ((abstract_regress *) inputpsyimg)->fillpage(z, i);
};

class regress : public abstract_regress {
 protected:
  int num_images;
  psyimg **image_ptrs;

  double *indx_array;
  double *indx_diff_array;
  double sum_indx;
  double mean_indx;
  double sum_sqr_dev_indx;
  double std_dev_indx;
  double interpolate_to_x;

  regresspgbuff *apgbuff;
  regresspgbuff *bpgbuff;
  regresspgbuff *stdyerrpgbuff;
  psyimg *countimage;
  void fillpage(char *buff, int z, int i);
 public:
  regress(psyimg *image_ptrs[], double indx_array[], int num_images,
	  double interpolate_to_x, int numpages);
  void fillpage(int z, int i);
  regresspgbuff *getAimage() { return apgbuff; };
  regresspgbuff *getBimage() { return bpgbuff; };
  regresspgbuff *getstdyerrimage() { return stdyerrpgbuff; };
  psyimg *getcountimage() { return countimage; };
  ~regress();
};

regress::regress(psyimg *image_ptrs[], double indx_array[], int num_images,
		       double interpolate_to_x, int numpages)
{
  initpgbuff(image_ptrs[0], numpages, psydouble);
  regress::num_images=num_images;
  regress::interpolate_to_x=interpolate_to_x;
  regress::image_ptrs = new psyimg *[num_images];
  regress::indx_array = new double[num_images];
  indx_diff_array = new double[num_images];

  sum_indx = 0;
  sum_sqr_dev_indx=0;
  for(int i=0; i<num_images; i++) {
    regress::image_ptrs[i] = image_ptrs[i];
    regress::indx_array[i] = indx_array[i];
    sum_indx += indx_array[i];
    sum_sqr_dev_indx += indx_array[i] * indx_array[i];
  }
  mean_indx = sum_indx / num_images;
  sum_sqr_dev_indx -= sum_indx * mean_indx;

  for(int i=0; i<num_images; i++) {
    indx_diff_array[i] = indx_array[i] - mean_indx;
  }

  std_dev_indx = sqrt(sum_sqr_dev_indx);

  apgbuff = new regresspgbuff(this, numpages, psydouble);
  bpgbuff = new regresspgbuff(this, numpages, psydouble);
  stdyerrpgbuff = new regresspgbuff(this, numpages, psydouble);
  countimage = new psyimgconstant(image_ptrs[0], num_images, psyushort);
}

regress::~regress()
{
  delete[] image_ptrs;
  delete[] indx_diff_array;
  delete apgbuff;
  delete bpgbuff;
  delete stdyerrpgbuff;
  delete countimage;
}


void regress::fillpage(int z, int i)
{
  // call standard fillpage indirectly through getzptrlocked()
  // so fillpage wont get called unless it has to remake the
  // page buffer for (z,i).
  getzptrlocked(z, i);
  unlockzptr(z, i);
}

void regress::fillpage(char *buff, int z, int i)
{
  for(int n=0; n<num_images; n++)
    image_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);

  psytype o_type = gettype();
  psydims o_inc = getinc();
  char *o_yoptr=buff;

  psytype a_type = apgbuff->gettype();
  psydims a_inc = apgbuff->getinc();
  char *a_yoptr = apgbuff->getzptrlocked(z, i);

  psytype b_type = bpgbuff->gettype();
  psydims b_inc = bpgbuff->getinc();
  char *b_yoptr = bpgbuff->getzptrlocked(z, i);

  psytype stdyerr_type = stdyerrpgbuff->gettype();
  psydims stdyerr_inc = stdyerrpgbuff->getinc();
  char *stdyerr_yoptr = stdyerrpgbuff->getzptrlocked(z, i);

  for(int y=orig.y; y<=end.y; y++, o_yoptr += o_inc.y, a_yoptr += a_inc.y,
      b_yoptr += b_inc.y, stdyerr_yoptr += stdyerr_inc.y) {
    char *o_xoptr=o_yoptr;
    char *a_xoptr=a_yoptr;
    char *b_xoptr=b_yoptr;
    char *stdyerr_xoptr=stdyerr_yoptr;
    for(int x=orig.x; x<=end.x; x++,
	o_xoptr += o_inc.x, a_xoptr += a_inc.x, b_xoptr += b_inc.x,
	stdyerr_xoptr += stdyerr_inc.x) {

      double sum_depy = 0;
      double sum_sqr_dev_depy = 0;  //test residual form 2
      double sumxdiffy=0;
      for(int n=0; n<num_images; n++) {
	double depy_value;
	image_ptrs[n]->getnextpixel((char *)&depy_value);
	sum_depy += depy_value;
	sumxdiffy += indx_diff_array[n] * depy_value;
	sum_sqr_dev_depy += depy_value * depy_value;
      }
      sum_sqr_dev_depy -= sum_depy * sum_depy / num_images;

      double residualss = 0;
      double b = 0;
      double a = 0;
      if(sum_sqr_dev_indx > 1e-16) {
	b = sumxdiffy / sum_sqr_dev_indx;
	a = (sum_depy - (sum_indx * b))/num_images;
	residualss = sum_sqr_dev_depy - (b*sumxdiffy);
      }
//if(x == 67 && y==38 && z==36)
//  cout <<setprecision(10)<<"loc=("<<x<<','<<y<<','<<z<<") residualss="<<residualss<<"\n";

      double residual_mean_square = 0;
      if(num_images > 2) residual_mean_square = residualss/(num_images - 2);
      // double sigdat = sqrt(residual_mean_square); // standard error of estimate

      double interpolated_value = a + (b * interpolate_to_x);
      pixeltypechg((char *)&interpolated_value, psydouble, o_xoptr, o_type);

      pixeltypechg((char *)&a, psydouble, a_xoptr, a_type);
      pixeltypechg((char *)&b, psydouble, b_xoptr, b_type);

      double stdyerr = 0;
      if(sum_sqr_dev_indx > 1e-16) {
	double diff = interpolate_to_x - mean_indx;
	stdyerr =
	  sqrt(residual_mean_square *
	       (1.0 + 1.0/num_images +
		(diff*diff)/sum_sqr_dev_indx));
      }
//if(x == 67 && y==38 && z==36) cout <<"a="<<a<<" b="<<b<<" interpolate_to_x="<<interpolate_to_x<<" interpolatedvalue="<<interpolated_value<<" stdyerr="<<stdyerr<<"\n";
      pixeltypechg((char *)&stdyerr, psydouble, stdyerr_xoptr, stdyerr_type);

    } // end for(x=...)
  } // end for(y=...)

  apgbuff->unlockzptr(z, i);
  bpgbuff->unlockzptr(z, i);
  stdyerrpgbuff->unlockzptr(z, i);

  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
  }
}


/** regress2 */
class regress2 : public abstract_regress {
 protected:
  int num_images;
  psyimg **image_ptrs;
  psyimg **template_ptrs;
  double thresh;
  int mincount;
  double *indx_array;
  double interpolate_to_x;

  regresspgbuff *apgbuff;
  regresspgbuff *bpgbuff;
  regresspgbuff *stdyerrpgbuff;
  regresspgbuff *cntpgbuff;
  void fillpage(char *buff, int z, int i);
 public:
  regress2(psyimg *image_ptrs[], double indx_array[],
	   psyimg *template_ptrs[], int num_images,
	   double interpolate_to_x, int numpages,
	   double thresh, int mincount);
  void fillpage(int z, int i);
  regresspgbuff *getAimage() { return apgbuff; };
  regresspgbuff *getBimage() { return bpgbuff; };
  regresspgbuff *getstdyerrimage() { return stdyerrpgbuff; };
  regresspgbuff *getcountimage() { return cntpgbuff; };
  ~regress2();
};

regress2::regress2(psyimg *image_ptrs[], double indx_array[],
		   psyimg *template_ptrs[], int num_images,
		   double interpolate_to_x, int numpages,
		   double thresh, int mincount)
{
  initpgbuff(image_ptrs[0], numpages, psydouble);
  regress2::num_images=num_images;
  regress2::interpolate_to_x=interpolate_to_x;
  regress2::image_ptrs = new psyimg *[num_images];
  regress2::template_ptrs = new psyimg *[num_images];
  regress2::indx_array = new double[num_images];
  regress2::thresh = thresh;
  regress2::mincount = mincount;

  for(int i=0; i<num_images; i++) {
    regress2::image_ptrs[i] = image_ptrs[i];
    regress2::template_ptrs[i] = template_ptrs[i];
    regress2::indx_array[i] = indx_array[i];
  }

  apgbuff = new regresspgbuff(this, numpages, psydouble);
  bpgbuff = new regresspgbuff(this, numpages, psydouble);
  stdyerrpgbuff = new regresspgbuff(this, numpages, psydouble);
  cntpgbuff = new regresspgbuff(this, numpages, psyushort);
}

regress2::~regress2()
{
  delete[] image_ptrs;
  delete[] template_ptrs;
  delete[] indx_array;
  delete apgbuff;
  delete bpgbuff;
  delete stdyerrpgbuff;
  delete cntpgbuff;
}

void regress2::fillpage(int z, int i)
{
  // call standard fillpage indirectly through getzptrlocked()
  // so fillpage wont get called unless it has to remake the
  // page buffer for (z,i).
  getzptrlocked(z, i);
  unlockzptr(z, i);
}

void regress2::fillpage(char *buff, int z, int i)
{
  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
    template_ptrs[n]->initgetpixel(psydouble, orig.x, orig.y, z, i);
  }

  psytype o_type = gettype();
  psydims o_inc = getinc();
  char *o_yoptr=buff;

  psytype a_type = apgbuff->gettype();
  psydims a_inc = apgbuff->getinc();
  char *a_yoptr = apgbuff->getzptrlocked(z, i);

  psytype b_type = bpgbuff->gettype();
  psydims b_inc = bpgbuff->getinc();
  char *b_yoptr = bpgbuff->getzptrlocked(z, i);

  psytype stdyerr_type = stdyerrpgbuff->gettype();
  psydims stdyerr_inc = stdyerrpgbuff->getinc();
  char *stdyerr_yoptr = stdyerrpgbuff->getzptrlocked(z, i);

  psytype cnt_type = cntpgbuff->gettype();
  psydims cnt_inc = cntpgbuff->getinc();
  char *cnt_yoptr = cntpgbuff->getzptrlocked(z, i);

  for(int y=orig.y; y<=end.y; y++, o_yoptr += o_inc.y, a_yoptr += a_inc.y,
      b_yoptr += b_inc.y, stdyerr_yoptr += stdyerr_inc.y,
      cnt_yoptr += cnt_inc.y) {
    char *o_xoptr=o_yoptr;
    char *a_xoptr=a_yoptr;
    char *b_xoptr=b_yoptr;
    char *stdyerr_xoptr=stdyerr_yoptr;
    char *cnt_xoptr=cnt_yoptr;
    for(int x=orig.x; x<=end.x; x++,
	o_xoptr += o_inc.x, a_xoptr += a_inc.x, b_xoptr += b_inc.x,
	stdyerr_xoptr += stdyerr_inc.x, cnt_xoptr += cnt_inc.x) {

      double sum_indx = 0;
      double sum_sqr_dev_indx = 0;

      double sum_depy = 0;
      double sum_sqr_dev_depy = 0;

      double sum_xy_dev = 0;
      
      int count = 0;
      for(int n=0; n<num_images; n++) {
	double depy_value;
	image_ptrs[n]->getnextpixel((char *)&depy_value);;
	double template_value;
	template_ptrs[n]->getnextpixel((char *)&template_value);
	if(template_value >= thresh) {
	  count++;
	  sum_indx += indx_array[n];
	  sum_sqr_dev_indx += indx_array[n] * indx_array[n];

	  sum_depy += depy_value;
	  sum_sqr_dev_depy += depy_value * depy_value;

	  sum_xy_dev += indx_array[n] * depy_value;
	}
      }
      double b = 0;
      double a = 0;
      double interpolated_value = 0;
      double stdyerr = 0;

      if(count < mincount) count=0;
      else {
	double mean_indx = sum_indx / count;
	double mean_depy = sum_depy / count;

	sum_xy_dev -= mean_indx * sum_depy;
	sum_sqr_dev_indx -= sum_indx * mean_indx;
	sum_sqr_dev_depy -= sum_depy * mean_depy;

	double residualss = 0;
	if(sum_sqr_dev_indx > 1e-16) {
	  b = sum_xy_dev / sum_sqr_dev_indx;
	  a = mean_depy - (b * mean_indx);
	  residualss = sum_sqr_dev_depy - (b*sum_xy_dev);
	}

	double residual_mean_square = 0;
	residual_mean_square = residualss/(count - 2);

	interpolated_value = a + (b * interpolate_to_x);

	if(sum_sqr_dev_indx > 1e-16) {
	  double diff = interpolate_to_x - mean_indx;
	  stdyerr =
	    sqrt(residual_mean_square *
		 (1.0 + 1.0/count +
		  (diff*diff)/sum_sqr_dev_indx));
	}
      }

      pixeltypechg((char *)&interpolated_value, psydouble, o_xoptr, o_type);
      pixeltypechg((char *)&a, psydouble, a_xoptr, a_type);
      pixeltypechg((char *)&b, psydouble, b_xoptr, b_type);
      pixeltypechg((char *)&stdyerr, psydouble, stdyerr_xoptr, stdyerr_type);
      pixeltypechg((char *)&count, psyint, cnt_xoptr, cnt_type);

    } // end for(x=...)
  } // end for(y=...)

  apgbuff->unlockzptr(z, i);
  bpgbuff->unlockzptr(z, i);
  stdyerrpgbuff->unlockzptr(z, i);
  cntpgbuff->unlockzptr(z, i);

  for(int n=0; n<num_images; n++) {
    image_ptrs[n]->freegetpixel();
    template_ptrs[n]->freegetpixel();
  }
}
/** end regress2 */

int main(int argc, char *argv[])
{
  char *compare_to_file=NULL;
  char *compareddifffile=NULL;
  char *comparedtvaluefile=NULL;
  char *comparedprobfile=NULL;
  char *interpolatedfile=NULL;
  char *slopefile=NULL;
  char *interceptfile=NULL;
  char *stdyerrfile=NULL;
  char *countfile=NULL;

  char *listfile=NULL;
  char *templatelistfile=NULL;
  char *xfile=NULL;
  double interpolate_to_value;
  int interpolate_value_set=0;
  double thresh = 0.5;
  int mincount = 3;
  double min, max, mean;
  psytype outtype=psynotype; // will default to largest input type
  psytype workingtype=psydouble;
  psytype biggestintype=psynotype;
  psyfileclass outfileclass=psynoclass; // defaults to first input class
  psypgbuff *newpsypgbuff=NULL;
  int numinputfiles=0;
  int print_list=0;
  int across_idim=0;
// allocate a name list large enough to point to all input file names //
  string *commandlinelist=new string[argc];
  string *namelist=commandlinelist;
  string *templatelist = NULL;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" -list listfile | infile1 [infile2 [infile3 [...]]] [-across_idim]\n";
      cout<<" -templatelist templatelistfile [-mincnt mincount]\n";
      cout<<" -xf x_file [-print_list]\n";
      cout<<" [-x interpolate_to_value [-compare compare_to_file\n";
      cout<<" [-odiff compareddifffile] [-otvalue comparedtvaluefile]\n";
      cout<<" [-oprob comparedprobfile]]\n";
      cout<<" [-o interpolatedfile]]\n";
      cout<<" [-oslope slopefile] [-ointercept interceptfile]\n";
      cout<<" [-ostdyerr standardyerrfile] [-ocnt countfile]\n";
      cout<<" [-uchar | -short | -int | -float]\n";
      cout<<" [-analyze | -ecat | -sdt]\n";
      cout<<" ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout<<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-compare", argv[i])==0) && ((i+1)<argc))compare_to_file=argv[++i];
    else if((strcmp("-odiff", argv[i])==0) && ((i+1)<argc))compareddifffile=argv[++i];
    else if((strcmp("-otvalue", argv[i])==0) && ((i+1)<argc))comparedtvaluefile=argv[++i];
    else if((strcmp("-oprob", argv[i])==0) && ((i+1)<argc))comparedprobfile=argv[++i];
    else if((strcmp("-oslope", argv[i])==0) && ((i+1)<argc))slopefile=argv[++i];
    else if((strcmp("-ointercept", argv[i])==0) && ((i+1)<argc))interceptfile=argv[++i];
    else if((strcmp("-ostdyerr", argv[i])==0) && ((i+1)<argc))stdyerrfile=argv[++i];
    else if((strcmp("-ocnt", argv[i])==0) && ((i+1)<argc))countfile=argv[++i];
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if((strcmp("-list", argv[i])==0) && ((i+1)<argc))listfile=argv[++i];
    else if((strcmp("-templatelist", argv[i])==0) && ((i+1)<argc))
      templatelistfile=argv[++i];
    else if((strcmp("-xf", argv[i])==0) && ((i+1)<argc))xfile=argv[++i];
    else if((strcmp(argv[i],"-x")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf",&interpolate_to_value) != 1) {
	cerr << argv[0] << ": error parsing interpolate_to_value: -x ";
	cerr << argv[i] << '\n';
	exit(1);
      }
      interpolate_value_set=1;
    }
    else if((strcmp(argv[i],"-mincnt")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%d",&mincount) != 1) {
	cerr << argv[0] << ": error parsing minimum count: -min_count ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if(strcmp("-print_list", argv[i])==0)print_list=1;
    else if(strcmp("-across_idim", argv[i])==0)across_idim=1;
    else if((strcmp("-o", argv[i])==0) && ((i+1)<argc))interpolatedfile=argv[++i];
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

// count output files
  int numoutputfiles = 0;
  if(interpolatedfile != NULL) numoutputfiles++;
  if(stdyerrfile != NULL) numoutputfiles++;
  if(countfile != NULL) numoutputfiles++;
  if(slopefile != NULL)  numoutputfiles++;
  if(interceptfile != NULL) numoutputfiles++;
  if(compareddifffile != NULL) numoutputfiles++;
  if(comparedtvaluefile != NULL) numoutputfiles++;
  if(comparedprobfile != NULL) numoutputfiles++;

  if(numoutputfiles == 0) {
    cerr<<argv[0]<<" - no output file specified\n";
    exit(1);
  }

  if(compareddifffile != NULL || comparedtvaluefile != NULL || comparedprobfile != NULL) {
    if(compare_to_file == NULL) {
      cerr<<argv[0]<<" - can't generate difference, tvalue or probability file without compare to file\n";
      exit(1);
    }
  }
  else if(compare_to_file != NULL) {
    cerr<<argv[0]<<" - no need to process compare to file if not generating difference, tvalue or probability file\n";
    exit(1);
  }

// verify x values specified
  if(xfile == NULL) {
    cerr<<argv[0]<<" - missing xfile\n";
    exit(1);
  }

  if(listfile != NULL) {
// read input list
    namelist=read_list(listfile, &numinputfiles);
  }

  if(templatelistfile != NULL) {
    int numtemplates;
    templatelist=read_list(templatelistfile, &numtemplates);
    if(numtemplates != numinputfiles) {
      cerr<<argv[0]<<" - number of templates not the same as number of input files\n";
      exit(1);
    }
  }

// verify at least one input file
  if(numinputfiles < 1) {
    cerr<<argv[0]<<" - no input files specified\n";
    exit(1);
  }

  if(numinputfiles != 1 && across_idim) {
    cerr<<argv[0]<<" - only 1 input file allowed with across i (time) option\n";
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
    imageptrs = new psyimg*[numinputfiles];
    for(int i=in_beg.i; i<=in_end.i; i++) {
// select block of image
      psyimg *newimgptr = new psyimgblk(imgptr, in_beg.x, in_beg.y, in_beg.z, i,
					in_end.x, in_end.y, in_end.z, i, 1);
// buffer each input i to improve accumulation speed
      newpsypgbuff=new psypgbuff((psyimg *)newimgptr, 1);
// keep a pointer to this buffer
      imageptrs[i]=(psyimg *)newpsypgbuff;
    }
  }
  else {
    imageptrs = new psyimg*[numinputfiles];
    if(templatelist != NULL) templateptrs = new psyimg*[numinputfiles];
    for(int i=0; i<numinputfiles; i++) {
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
  if(outtype==psynotype)outtype=biggestintype;

// get x values
  double x_array[numinputfiles];
  double mean_x=0;
// read x values from a file
  fstream fd(xfile, ios::in);
  if(fd.fail() || fd.bad()) {
    cerr<<argv[0]<<" - unable to open file "<<xfile<<'\n';
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
	  cerr<<argv[0]<<" - warning - number of input values in "<<xfile;
	  cerr<<" greater then number of input images\n";
	}
      else {
	x_array[i]=atof(linebuff);
	mean_x += x_array[i];
	i++;
      }
    }// end if(fd.good())
  }// end while
  if(i != numinputfiles) {
    cerr<<argv[0]<<" - not enough input values in "<<xfile<<'\n';
    exit(1);
  }

  mean_x /= numinputfiles;

// default to interpolate to mean x
  if(! interpolate_value_set) {
    interpolate_to_value=mean_x;
    if(interpolatedfile != NULL || comparedtvaluefile != NULL || comparedprobfile != NULL ||
       compareddifffile != NULL || stdyerrfile != NULL) {
      cout<<"warning - no interpolate value given - interpolating to mean x value="<<mean_x<<'\n';
    }
  }

// print stuff on x values
  if(print_list) {
    double sumsqrdev=0;
    double dev;
    for(i=0; i<numinputfiles; i++) {
      dev=x_array[i]-mean_x;
      sumsqrdev += dev*dev;
    }
    double stddev = sqrt(sumsqrdev/(numinputfiles-1));
    for(i=0; i<numinputfiles; i++) {
      cout<<namelist[i];
      cout<<" x["<<i<<"]="<<x_array[i];
      if(templatelist != NULL) cout<<" template="<<templatelist[i];
      cout<<'\n';
//      dev=x_array[i]-mean_x;
//      cout<<" \tdev="<<dev;
//      cout<<" \tdev/std="<<dev/stddev<<'\n';
    }
    cout<<"x mean="<<mean_x<<'\n';
    cout<<"x standard deviation="<<stddev<<'\n';
  }

  psyimg *psyimgptr = NULL;
  abstract_regress *linearfitimg = NULL;

// Linear regression across images

  int npages = 1;
  if(numoutputfiles > 1) {
    // set size to total number of pages so pages don't have to be
    // recalculated with multiple output images.
    int isize = 1;
    imageptrs[0]->getsize(NULL, NULL, &npages, &isize);
    npages *= isize;
  }

  if(templateptrs == NULL) {
    linearfitimg = new regress(imageptrs, x_array, numinputfiles,
			       interpolate_to_value, npages);
  }
  else {
    linearfitimg = new regress2(imageptrs, x_array,
				templateptrs, numinputfiles,
				interpolate_to_value, npages,
				thresh, mincount);
  }
  psyimgptr = (psyimg *)linearfitimg;
// set description for output image
  psyimgptr->setdescription("Linear regression across image pixels");

// set date and time to current date and time
  psyimgptr->settime();
  psyimgptr->setdate();

  if(compare_to_file != NULL) {
    // subtract interpolated file from compare_to_file
    psyimg *comparetoimginptr=psynewinfile(compare_to_file);
    psypgbuff comparetoimgbuffed(comparetoimginptr);

    addimgs diffedimgs(&comparetoimgbuffed, linearfitimg, 1.0, -1.0, workingtype);

    if(compareddifffile != NULL) {
      psyimg *outpsyimgptr=psynewoutfile(compareddifffile, &diffedimgs,
					 outfileclass, outtype);
      logfile log(compareddifffile, argc, argv);
      delete outpsyimgptr;
    }
    if(comparedtvaluefile != NULL || comparedprobfile != NULL) {
      // divide by stdyerr to get tvalue
      divideimgs tvaluesimg(&diffedimgs, linearfitimg->getstdyerrimage());

      if(comparedtvaluefile != NULL) {
	psyimg *outpsyimgptr=psynewoutfile(comparedtvaluefile, &tvaluesimg,
					 outfileclass, outtype);
	logfile log(comparedtvaluefile, argc, argv);
	delete outpsyimgptr;
      }

      if(comparedprobfile != NULL) {
	probfromtvalueimg probimg(&tvaluesimg,
				  linearfitimg->getcountimage(), -2.0,
				  outtype);
	psyimg *outpsyimgptr=psynewoutfile(comparedprobfile, &probimg,
					   outfileclass, outtype);
	logfile log(comparedprobfile, argc, argv);
	delete outpsyimgptr;
      }

    }
    delete comparetoimginptr;
  }

  if(interpolatedfile != NULL) {
// output result to file
    psyimg *outpsyimgptr=psynewoutfile(interpolatedfile, psyimgptr,
				       outfileclass, outtype);
// log
    logfile log(interpolatedfile, argc, argv);
// print out images stats
    outpsyimgptr->getstats(&min, &max, &mean);
    cout<<"interpolated output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outpsyimgptr;
  }

  if(stdyerrfile != NULL) {
    psyimg *outpsyimgptr=psynewoutfile(stdyerrfile,
				       linearfitimg->getstdyerrimage(),
				       outfileclass, outtype);
// log
    logfile log(stdyerrfile, argc, argv);

// clean up
    delete outpsyimgptr;
  }

  if(slopefile != NULL) {
    psyimg *outpsyimgptr=psynewoutfile(slopefile,
				       linearfitimg->getBimage(),
				       outfileclass, outtype);
// log
    logfile log(slopefile, argc, argv);

// clean up
    delete outpsyimgptr;
  }

  if(interceptfile != NULL) {
    psyimg *outpsyimgptr=psynewoutfile(interceptfile,
				       linearfitimg->getAimage(),
				       outfileclass, outtype);
// log
    logfile log(interceptfile, argc, argv);

// clean up
    delete outpsyimgptr;
  }

  if(countfile != NULL) {
    psyimg *outpsyimgptr=psynewoutfile(countfile,
				       linearfitimg->getcountimage(),
				       outfileclass, outtype);
// log
    logfile log(countfile, argc, argv);

// clean up
    delete outpsyimgptr;
  }

// clean up
  if(linearfitimg != NULL) delete linearfitimg;
  for(i=0; i<numinputfiles; i++){
    delete ((psypgbuff *)imageptrs[i])->inputpsyimg;
    delete imageptrs[i];
  }
  if(namelist != commandlinelist) delete[] namelist;
  delete[] templatelist;
}
