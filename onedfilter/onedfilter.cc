#include "psyhdr.h"
#include "psyiofunctions.h"
#include "math.h"

typedef unsigned long dimension;
#define PI M_PI
inline void SWAP(float *a, float *b) {float temp= *a; *a = *b; *b=temp;};

void four1(float data[], dimension nn, int isign);

enum fftfilterpadding { fftnoextrapad=0, fftzerofillpad,
			fftduplicateendpointspad, fftwrapraroundends };

class onedfilter : public psyfullbuff {
 protected:
  float *fftfilter;
  int fftsize;
  int filterdim;
  int datasize;
  fftfilterpadding padding;
  void chknfillbuff();
  void setvalues(float *array, int length, psytype wordtype, int leftwrap,
		 fftfilterpadding padding=fftzerofillpad);
 public:
  onedfilter(psyimg *image_ptr, int filterdim=3); // default to filter across i
  ~onedfilter();
  void setimpulsevalues(float *impulsearray, int length,
			psytype wordtype=psyfloat,
			fftfilterpadding padding=fftzerofillpad);
  void setfftvalues(float *fftarray, int length, psytype wordtype,
		    fftfilterpadding padding=fftzerofillpad);
  void output_tree(ostream *out) {
    psyfullbuff::output_tree(out);*out<<"::onedfileter";
  };
  int getfftsize() { return(fftsize); };
};

onedfilter::onedfilter(psyimg *image_ptr, int filterdim)
{
  fftfilter = NULL; fftsize = 0;
  initpsyfullbuff(image_ptr);
  onedfilter::filterdim = filterdim;
  switch (filterdim) {
  case 0:
    datasize = size.x;
    break;
  case 1:
    datasize = size.y;
    break;
  case 2:
    datasize = size.z;
    break;
  case 3:
    datasize = size.i;
    break;
  default:
    output_tree(&cerr);
    cerr<<"::onedfilter - invalid filter across dim="<<filterdim<<"\n";
    exit(1);
  }
}

onedfilter::~onedfilter()
{
  if(fftfilter != NULL) delete[] fftfilter;
}

void onedfilter::setimpulsevalues(float *impulsearray, int length,
				  psytype wordtype, fftfilterpadding padding) {
  // wrap the impulse to shift center to time zero
  int leftwrap = length/2;
  setvalues(impulsearray, length, wordtype, leftwrap, padding);
  // convert impulse to fft
  four1(fftfilter, fftsize, 1);
}

void onedfilter::setfftvalues(float *fftarray, int length,
			      psytype wordtype, fftfilterpadding padding) {
  setvalues(fftarray, length, wordtype, 0, padding);
}

void onedfilter::setvalues(float *array, int length, psytype wordtype,
			   int leftwrap, fftfilterpadding padding) {
  onedfilter::padding = padding;
  // calculate padding
  // use max just in case filter length is greater then datasize
  int padded_length = max(datasize, length);
  switch (padding) {
  // only length/2 padding needed because padded area thrown away
  case fftnoextrapad:
    break;
  case fftzerofillpad:
    padded_length += (length/2);
    break;
  case fftduplicateendpointspad:
    padded_length += length;
    break;
  case fftwrapraroundends:
    // wrap around will default if size is power of 2
    fftsize = 2;
    while(fftsize < padded_length) fftsize *= 2;
    if(fftsize > padded_length) padded_length += length;
    break;
  }
  // must be power of 2 for fft
  fftsize = 2;
  while(fftsize < padded_length) fftsize *= 2;
  // storage for fft filter
  fftfilter = new float[fftsize * 2];
  for(int i=0; i<fftsize*2; i++) fftfilter[i] = 0;

  int i=0;
  float *inptr = array;
  float *outptr;
  switch(wordtype) {
  case psyfloat:
    // input array contains only real values
    if(leftwrap > 0) {
      outptr = fftfilter + 2 * (fftsize - leftwrap);
      for(; i<leftwrap; i++, inptr++, outptr += 2) *outptr = *inptr;
    }
    outptr = fftfilter;
    for(; i<length; i++, inptr++, outptr += 2) *outptr = *inptr;
    break;
  case psycomplex:
    if(leftwrap > 0) {
      outptr = fftfilter + 2 * (fftsize - leftwrap);
      for(; i<leftwrap; i++, inptr += 2, outptr += 2) {
	outptr[0] = inptr[0]; outptr[1] = inptr[1];
      }
    }
    outptr = fftfilter;
    for(; i<length; i++, inptr += 2, outptr += 2) {
      outptr[0] = inptr[0]; outptr[1] = inptr[1];
    }
    break;
  default:
    output_tree(&cerr);
    cerr<<"::setvalues - invalid fft filter array data type\n";
    exit(1);
  }
  /*
    cout<<"filter array=\n";
    for(int i=0; i<fftsize; i++) cout<<fftfilter[2*i]<<"+i"<<fftfilter[2*i+1]<<'\n';
    cout.flush();
  */
}

inline void paddata(float *fftdata, float *endinputdata, float *endfftdata,
		    fftfilterpadding padding) {
  // fill remaining portion of input array
  float *dataptr = endinputdata;
  switch (padding) {
  case fftnoextrapad:
  case fftzerofillpad:
    // clear remaining array
    for(; dataptr < endfftdata; dataptr+=2)
      dataptr[0] = dataptr[1] = 0;
    break;
  case fftduplicateendpointspad:
    {
      // right pad with last value
      float *endrightpad = dataptr + ((endfftdata - endinputdata)/2);
      float rfill = *(endinputdata - 2);
      float ifill = *(endinputdata - 1);
      for(; dataptr < endrightpad; dataptr += 2) {
	dataptr[0] = rfill; dataptr[1] = ifill;
      }
      // left pad with first value
      rfill = fftdata[0]; ifill = fftdata[1];
      for(; dataptr < endfftdata; dataptr+=2) {
	dataptr[0] = rfill; dataptr[1] = ifill;
      }
    }
    break;
  case fftwrapraroundends:
    {
      // right pad with beginning data
      float *endrightpad = endinputdata + ((endfftdata - endinputdata)/2);
      float *padfromptr = fftdata;
      for(; dataptr < endrightpad; dataptr+=2, padfromptr+=2) {
	dataptr[0] = padfromptr[0]; dataptr[1] = padfromptr[1];
      }
      // left pad with endding data
      padfromptr = endinputdata - (endfftdata - dataptr);
      for(; dataptr < endfftdata; dataptr+=2, padfromptr+=2) {
	dataptr[0] = padfromptr[0]; dataptr[1] = padfromptr[1];
      }
    }
    break;
  }
}

inline void filterdata(float *fftdata, float *endfftdata,
		       int fftsize, float *fftfilter) {
  four1(fftdata, fftsize, 1);
  // multiply fft of data by fft filter
  for(float *dataptr = fftdata; dataptr<endfftdata;
      dataptr += 2, fftfilter += 2) {
    float rtmp = dataptr[0];
    float itmp = dataptr[1];
    dataptr[0] = rtmp*fftfilter[0] - itmp*fftfilter[1];
    dataptr[1] = rtmp*fftfilter[1] + itmp*fftfilter[0];
  }
  // invert fft - adds a factor of fftsize
  four1(fftdata, fftsize, -1);
}

inline void transferoutdata(float *fftdata, float *endinputdata,
			    char *outptr, int outinc,
			    psytype outtype, double outfactor) {
  // transfer filtered results to output buffer
  for(; fftdata<endinputdata; outptr += outinc, fftdata += 2) {
    fftdata[0] *= outfactor; // only real part used
    pixeltypechg((char *) fftdata, psycomplex, outptr, outtype);
  }
}

void onedfilter::chknfillbuff() {
  if(buffimage.getbuff() == NULL) {
    if((fftsize < datasize) || (fftfilter == NULL)) {
      output_tree(&cerr);
      cerr<<"onedfilter::chknfillbuff - fft filter not set";
      exit(1);
    }
    // initialize buffer to full image size
    buffimage.initbuff(size.x, size.y, size.z, size.i, type);
    // build storage for the data fft
    float *fftdata = new float[fftsize * 2];
    float *endfftdata = fftdata + (fftsize * 2);
    int complexbytes = gettypebytes(psycomplex);
    float *endinputdata = fftdata + datasize * 2;
    // correction factor for inverse fourier transform
    double invfourfixfactor = 1.0/(double) fftsize;
    switch (filterdim) {
    case 0:
      {
	// filter across x for every 3d image location
	char *ioptr = buffimage.getbuff() +
	  offset(orig.x, orig.y, orig.z, orig.i);
	for(int i=0; i<size.i; i++, ioptr += inc.i) {
	  char *zoptr = ioptr;
	  for(int z=0; z<size.z; z++, zoptr += inc.z) {
	    char *yoptr = zoptr;
	    for(int y=0; y<size.y; y++, yoptr += inc.y) {
	      // transfer input data to fft data array
	      inputpsyimg->copyblock((char *) fftdata,
				     orig.x, y, z, i,
				     end.x, y, z, i,
				     complexbytes, complexbytes,
				     complexbytes, complexbytes,
				     psycomplex);
	      // pad the data according to padding
	      paddata(fftdata, endinputdata, endfftdata, padding);
	      // preform fft on data
	      filterdata(fftdata, endfftdata, fftsize, fftfilter);
	      // transfer results to output buffer
	      transferoutdata(fftdata, endinputdata, yoptr, inc.x,
			      type, invfourfixfactor);
	    }
	  }
	}
      }
      break;
    case 1:
      {
	// filter across y for every 3d image location
	char *ioptr = buffimage.getbuff() +
	  offset(orig.x, orig.y, orig.z, orig.i);
	for(int i=0; i<size.i; i++, ioptr += inc.i) {
	  char *zoptr = ioptr;
	  for(int z=0; z<size.z; z++, zoptr += inc.z) {
	    char *xoptr = zoptr;
	    for(int x=0; x<size.x; x++, xoptr += inc.x) {
	      // transfer input data to fft data array
	      inputpsyimg->copyblock((char *) fftdata,
				     x, orig.y, z, i,
				     x, end.y, z, i,
				     complexbytes, complexbytes,
				     complexbytes, complexbytes,
				     psycomplex);
	      // pad the data according to padding
	      paddata(fftdata, endinputdata, endfftdata, padding);
	      // preform fft on data
	      filterdata(fftdata, endfftdata, fftsize, fftfilter);
	      // transfer results to output buffer
	      transferoutdata(fftdata, endinputdata, xoptr, inc.y,
			      type, invfourfixfactor);
	    }
	  }
	}
      }
      break;
    case 2:
      {
	// filter across z for every 3d image location
	char *ioptr = buffimage.getbuff() + offset(orig.x, orig.y, orig.z, orig.i);
	for(int i=0; i<size.i; i++, ioptr += inc.i) {
	  char *yoptr = ioptr;
	  for(int y=0; y<size.y; y++, yoptr += inc.y) {
	    char *xoptr = yoptr;
	    for(int x=0; x<size.x; x++, xoptr += inc.x) {
	      // transfer input data to fft data array
	      inputpsyimg->copyblock((char *) fftdata,
				     x, y, orig.z, i,
				     x, y, end.z, i,
				     complexbytes, complexbytes,
				     complexbytes, complexbytes,
				     psycomplex);
	      // pad the data according to padding
	      paddata(fftdata, endinputdata, endfftdata, padding);
	      /*
	      if(i == 0 && y==42 && x==37) {
		float *dataptr = fftdata;
		cout<<"data to filter=\n";
		for(; dataptr<endfftdata; dataptr += 2)
		  cout<<dataptr[0]<<"+i"<<dataptr[1]<<'\n';
	      }
	      */
	      // preform fft on data
	      filterdata(fftdata, endfftdata, fftsize, fftfilter);
	      /*
	      if(i == 0 && y==42 && x==37) {
		float *dataptr = fftdata;
		cout<<"filtered data=\n";
		for(; dataptr<endfftdata; dataptr += 2)
		  cout<<dataptr[0]*invfourfixfactor<<"+i"<<dataptr[1]*invfourfixfactor<<'\n';
	      }
	      */
	      // transfer results to output buffer
	      transferoutdata(fftdata, endinputdata, xoptr, inc.z,
			      type, invfourfixfactor);
	    }
	  }
	}
      }
      break;
    case 3:
      {
	// filter across i for every 3d image location
	char *zoptr = buffimage.getbuff() + offset(orig.x, orig.y, orig.z, orig.i);
	for(int z=0; z<size.z; z++, zoptr += inc.z) {
	  char *yoptr = zoptr;
	  for(int y=0; y<size.y; y++, yoptr += inc.y) {
	    char *xoptr = yoptr;
	    for(int x=0; x<size.x; x++, xoptr += inc.x) {
	      // transfer input data to fft data array
	      inputpsyimg->copyblock((char *) fftdata,
				     x, y, z, orig.i,
				     x, y, z, end.i,
				     complexbytes, complexbytes,
				     complexbytes, complexbytes,
				     psycomplex);
	      // pad the data according to padding
	      paddata(fftdata, endinputdata, endfftdata, padding);

	      /*
	      if(z == 33 && y==32 && x==42) {
		float *dataptr = fftdata;
		cout<<"data to filter=\n";
		for(; dataptr<endfftdata; dataptr += 2)
		  cout<<dataptr[0]<<"+i"<<dataptr[1]<<'\n';
	      }
	      */

	      // preform fft on data
	      filterdata(fftdata, endfftdata, fftsize, fftfilter);

	      /*
	      if(z == 33 && y==32 && x==42) {
		float *dataptr = fftdata;
		cout<<"filtered data=\n";
		for(; dataptr<endfftdata; dataptr += 2)
		  cout<<dataptr[0]*invfourfixfactor<<"+i"<<dataptr[1]*invfourfixfactor<<'\n';
	      }
	      */
	      // transfer results to output buffer
	      transferoutdata(fftdata, endinputdata, xoptr, inc.i,
			      type, invfourfixfactor);
	    }
	  }
	}
      }
      break;
    default:
      output_tree(&cerr);
      cerr<<"::chknfillbuff - invalid filter across dim="<<filterdim<<"\n";
      exit(1);
    } // end switch (filterdim)
  }
}

class singleimpulse : public psybuff {
 public:
  singleimpulse(int impulselocation, int isize, double ires);
};

singleimpulse::singleimpulse(int impulselocation,
			     int isize, double ires) {
  initbuff(1, 1, 1, isize, psyfloat,
	   0,0,0,0,
	   0.0,0.0,0.0,ires,1.0);
  float *buff = (float *) getbuff();
  for(int i=0; i<isize; i++) buff[i] = 0.0;
  if(impulselocation >= isize) {
    cerr<<"singleimpulse::singleimpulse - impulse location greater than size\n";
    exit(1);
  }
  buff[impulselocation] = 1;
}

int main(int argc, char *argv[])
{
  char *infile=NULL;
  char *listfile=NULL;
  char *outfile=NULL;
  char *fftshapefile=NULL;
  char *impulsefile=NULL;
  int averagewidth= -1;
  double std = 0;
  int gausswidth= -1;
  psytype outtype=psynotype; // will default to largest input type
  psytype biggestintype=psynotype;
  psyfileclass outfileclass=psynoclass; // defaults to first input class
  int *outfileclassinfo = NULL;
  psyimg *psyimgptr = NULL;
  psyimg *singleimgptr = NULL;
  psypgbuff *inbuffptr = NULL;
  concatimgs *lastconcatimgptr=NULL;
  concatimgs *firstconcatimgptr = NULL;
  singleimpulse *impulseimage = NULL;
  onedfilter *impulseimagefiltered = NULL;
  fftfilterpadding padding = fftzerofillpad;

  int across_dim=3;

// parse input parameters
  int i;
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<4) {
      cout<<"Usage: "<<argv[0];
      cout<<" infile | -list listfile  -o outfile\n";
      cout<<" [-fft fftshapefile | -fir impulsefile | -avg width | -gs std,width]\n";
      cout<<" [-dim x|y|z|i (i)]\n";
      cout<<" [-zeropad | -replicatepad | -wrapdatapad | -noextrapad]\n";
      cout<<" ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout<<" ["; writeoutfileclassargchoices(&cout); cout << "]\n";
       exit(0);
    }
    else if(i==0); // ignore program name
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(strcmp("-zeropad", argv[i])==0)padding = fftzerofillpad;
    else if(strcmp("-replicatepad", argv[i])==0)
      padding = fftduplicateendpointspad;
    else if(strcmp("-wrapdatapad", argv[i])==0)padding = fftwrapraroundends;
    else if(strcmp("-noextrapad", argv[i])==0)padding = fftnoextrapad;
    else if((strcmp("-dim", argv[i])==0) && ((i+1)<argc)){
      switch(argv[++i][0]) {
      case 'x':
      case '0':
	across_dim=0;
	break;
      case 'y':
      case '1':
	across_dim=1;
	break;
      case 'z':
      case '2':
	across_dim=2;
	break;
      case 'i':
      case '3':
	across_dim=3;
	break;
      default:
	cerr<<argv[0]<<" - invalid dimension given\n";
	exit(1);
      }
    }
    else if((strcmp("-fft", argv[i])==0) && ((i+1)<argc))
      fftshapefile=argv[++i];
    else if((strcmp("-fir", argv[i])==0) && ((i+1)<argc))
      impulsefile=argv[++i];
    else if((strcmp("-avg", argv[i])==0) && ((i+1)<argc)){
      int n=sscanf(argv[++i], "%d", &averagewidth);
      if(n != 1 || averagewidth < 1){
	cerr<<argv[0]<<": error - bad average width parameter="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((strcmp("-gs", argv[i])==0) && ((i+1)<argc)){
      int n=sscanf(argv[++i], "%lf,%d", &std, &gausswidth);
      if(n != 2 || gausswidth < 1){
	cerr<<argv[0]<<": error - bad gauss std,width parameter="<<argv[i]<<'\n';
	exit(1);
      }
    }
    else if((strcmp("-o", argv[i])==0) && ((i+1)<argc))outfile=argv[++i];
    else if((strcmp("-listfile", argv[i])==0) && ((i+1)<argc))
      listfile=argv[++i];
    else if(strncmp("-", argv[i], 1) == 0) {
      cerr<<argv[0]<<": error invalid parameter="<<argv[i]<<'\n';
      exit(1);
    }
    else if( (infile == NULL) && (listfile == NULL) ) infile=argv[i];
    else {
      cerr<<argv[0]<<": error unknown parameter="<<argv[i]<<'\n';
      exit(1);
    }
  }

  // verify one and only one input file type set
  if((infile == NULL) && (listfile == NULL)) {
    cerr<<argv[0]<<" - no input file or input list specified\n";
    exit(1);
  }
  else if((infile != NULL) && (listfile != NULL)) {
    cerr<<argv[0]<<" - invalid params - both infile and listfile set\n";
    exit(1);
  }
  // verify output file name set
  if(outfile == NULL) {
    cerr<<argv[0]<<" - no output file specified\n";
    exit(1);
  }
  // verify one and only one filter specified
  int cnt = (fftshapefile == NULL)? 0:1;
  cnt += (impulsefile == NULL) ? 0:1;
  cnt += (averagewidth < 1) ? 0:1;
  cnt += (gausswidth < 1) ? 0:1;
  if(cnt < 1) {
    cerr<<argv[0]<<" - no filter given\n";
    exit(1);
  }
  else if(cnt > 1) {
    cerr<<argv[0]<<" - more then one filter given\n";
    exit(1);
  }

  // open input file or files
  if(infile != NULL) {
    psyfileclass infileclass;
    singleimgptr=psynewinfile(infile, &infileclass, &biggestintype);
    inbuffptr = new psypgbuff(singleimgptr, 1);
    psyimgptr=inbuffptr;
    if(outfileclass == psynoclass)outfileclass=infileclass;
  }
  else {
    // read input list
    int numinputfiles=0;
    string *namelist=read_list(listfile, &numinputfiles);
    for(i=0; i<numinputfiles; i++) {
      // open and concat each input file as we come across it
      psyfileclass infileclass;
      psytype intype;
      psyimg *newinputfile=psynewinfile(namelist[i], &infileclass, &intype);
      // note largest type
      if(biggestintype == psynotype)biggestintype=intype;
      else if(gettypebytes(biggestintype) < gettypebytes(intype))
	biggestintype=intype;
      if(outfileclass == psynoclass)outfileclass=infileclass;
      if(psyimgptr == NULL) {
	psyimgptr = newinputfile;
	if(outfileclass == psynoclass) outfileclass=infileclass;
      }
      else {
	// cat it onto previous images
	lastconcatimgptr=new concatimgs(psyimgptr, (psyimg *)newinputfile, 3);
	// keep track of first concatimg pointer to help clean up
	if(firstconcatimgptr==NULL) firstconcatimgptr=lastconcatimgptr;
	psyimgptr=(psyimg *)lastconcatimgptr;
      }
    }
    delete[] namelist;
  }
  // buffer input completely to speed processing
  psyfullbuff fullbuff(psyimgptr);
  psyimgptr = (psyimg *) &fullbuff;
  // stuff for impulse modeling
  int impulselength = 0;
  int impulselocation = 1;
  double ires;
  fullbuff.getres(NULL, NULL, NULL, &ires);
  // build the filtered image
  onedfilter inputfiltered(psyimgptr, across_dim);
  inputfiltered.settime();
  inputfiltered.setdate();
  // set the filter values
  int numberofpoints=0;
  float *filter_array=NULL;
  if((fftshapefile != NULL) || (impulsefile != NULL)) {
    char *filename = (fftshapefile != NULL) ? fftshapefile : impulsefile;
    string *valuelist = read_list(filename, &numberofpoints);
    if(numberofpoints == 0) {
      cerr<<argv[0]<<" - no data found in filter file="<<filename<<"\n";
      exit(1);
    }
    // convert list to float values
    filter_array = new float[numberofpoints];
    for(int i=0; i<numberofpoints; i++) filter_array[i]=atof(valuelist[i].c_str());
    delete[] valuelist; valuelist=NULL;
    if(fftshapefile != NULL) {
      inputfiltered.setfftvalues(filter_array, numberofpoints, psyfloat,
				 padding);

      impulselength = inputfiltered.getfftsize();
      impulselocation = impulselength/2;
      impulseimage = new singleimpulse(impulselocation, impulselength, ires);
      impulseimagefiltered = new onedfilter(impulseimage);
      impulseimagefiltered->setfftvalues(filter_array, numberofpoints,
					 psyfloat, fftnoextrapad);
    }
    else {
      inputfiltered.setimpulsevalues(filter_array, numberofpoints,
				     psyfloat, padding);

      impulselength = inputfiltered.getfftsize();
      impulselocation = impulselength/2;
      impulseimage = new singleimpulse(impulselocation, impulselength, ires);
      impulseimagefiltered = new onedfilter(impulseimage);
      impulseimagefiltered->setimpulsevalues(filter_array, numberofpoints,
					     psyfloat, fftnoextrapad);
    }
  }
  else if(averagewidth > 0) {
    filter_array = new float[averagewidth];
    float avgfactor=1.0/((float) averagewidth);
    for(int i=0; i<averagewidth; i++) filter_array[i]=avgfactor;
    inputfiltered.setimpulsevalues(filter_array, averagewidth,
				   psyfloat, padding);

    impulselength = inputfiltered.getfftsize();
    impulselocation = impulselength/2;
    impulseimage = new singleimpulse(impulselocation, impulselength, ires);
    impulseimagefiltered = new onedfilter(impulseimage);
    impulseimagefiltered->setimpulsevalues(filter_array, averagewidth,
					   psyfloat, fftnoextrapad);
  }
  else if(gausswidth > 0) {
    filter_array = new float[gausswidth];
    double meanx = ((double) gausswidth-1) / 2.0;
    double factor = 0;
    for(int i=0; i<gausswidth; i++) {
      double x = (((double) i) - meanx)/std;
      filter_array[i]=exp(-0.5 * x * x);
      factor += filter_array[i];
    }
    // normalize to sum of 1.0
    if(factor > 1e-16) factor = 1.0/factor;
    else factor = 1.0;
    for(int i=0; i<gausswidth; i++) filter_array[i] *= factor;
    inputfiltered.setimpulsevalues(filter_array, gausswidth,
				   psyfloat, padding);

    impulselength = inputfiltered.getfftsize();
    impulselocation = impulselength/2;
    impulseimage = new singleimpulse(impulselocation, impulselength, ires);
    impulseimagefiltered = new onedfilter(impulseimage);
    impulseimagefiltered->setimpulsevalues(filter_array, gausswidth,
					   psyfloat, fftnoextrapad);
  }
  // get and print info about the impulse response
  float impulseresponse[impulselength];
  impulseimagefiltered->copyblock((char *)impulseresponse, 0, 0, 0, 0,
				  0, 0, 0, impulselength-1,
				  0, 0, 0, gettypebytes(psyfloat), psyfloat);
  cout<<"impulse response=\n";
  double sum=0;
  double momentsum=0;
  double max=impulseresponse[0];
  int maxlocation = 0;
  for(int i=0; i<impulselength; i++) {
    if(i == impulselocation) cout<<"1 - "<<impulseresponse[i]<<'\n';
    else cout<<"0 - "<<impulseresponse[i]<<'\n';
    sum += impulseresponse[i];
    momentsum += impulseresponse[i] * i;
    if(impulseresponse[i] > max) {
      max = impulseresponse[i];
      maxlocation = i;
    }
  }
  if(fabs(sum - 1.0) > 1e-3)
    cout<<"*** warning - filter sum not equal to 1.0 ***\n"; 
  cout<<"impulse sum="<<sum<<" (should be 1.0 for a properly weighted filter)\n";
  double centerofmoment = momentsum/sum;
  if(fabs(centerofmoment - impulselocation) > 1e-3)
    cout<<"*** warning - peaks may be shifted ***\n"; 
  cout<<"center of moment="<<centerofmoment<<" (should be "<<impulselocation<<" - other values will cause a peak shift)\n";
  cout<<"max value="<<max<<" at "<<maxlocation<<"\n";
  double thresh = 0.5 * max * max;
  int minhalflocation=maxlocation;
  while((minhalflocation > 0) &&
	(impulseresponse[minhalflocation]*impulseresponse[minhalflocation] > thresh))
    minhalflocation--;

  int maxhalflocation=maxlocation;
  while((maxhalflocation < impulselength-1) &&
	(impulseresponse[maxhalflocation]*impulseresponse[maxhalflocation] > thresh))
    maxhalflocation++;
  cout<<"half power locations="<<minhalflocation<<" and "<<maxhalflocation<<'\n';
  if((maxhalflocation <= 0) || (minhalflocation >= (impulselength-1))) {
    cout<<"*** warning - something not right with half power locations - not approximating FWHM ***\n";
  }
  else {
    double sqr1 = impulseresponse[minhalflocation] * 
      impulseresponse[minhalflocation];
    double sqr2 = impulseresponse[minhalflocation+1] * 
      impulseresponse[minhalflocation+1];
    double mininterpolated = (sqr1 * minhalflocation +
			      sqr2 * (minhalflocation+1))/(sqr1 + sqr2);
    sqr1 = impulseresponse[maxhalflocation] * impulseresponse[maxhalflocation];
    sqr2 = impulseresponse[maxhalflocation-1] * impulseresponse[maxhalflocation-1];
    double maxinterpolated = (sqr1 * maxhalflocation +
			      sqr2 * (maxhalflocation-1))/(sqr1 + sqr2);
    cout<<"half power locations interpolated="<<mininterpolated<<
      " and "<<maxinterpolated<<'\n';
    cout<<"approximate FWHM = "<<(maxinterpolated-mininterpolated)<<'\n';
  }
  if(outfile != NULL) {
// output result to file
    if(outtype==psynotype)outtype=biggestintype;
    psyimg *outpsyimgptr=psynewoutfile(outfile, &inputfiltered,
    outfileclass, outtype);
// log
    logfile log(outfile, argc, argv);
    log.loginfilelog(infile);
// print out images stats
    double min, max, mean;
    outpsyimgptr->getstats(&min, &max, &mean);
    cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
// clean up
    delete outpsyimgptr;
  }
  // clean up
  if(filter_array != NULL) delete[] filter_array;
  if(lastconcatimgptr != NULL) {
    concatimgs *prevconcatimgptr;
    while(lastconcatimgptr != firstconcatimgptr) {
      prevconcatimgptr=lastconcatimgptr;
      lastconcatimgptr=(concatimgs *)lastconcatimgptr->getlink1();
      delete prevconcatimgptr->getlink2();
      delete prevconcatimgptr;
    }
    delete firstconcatimgptr->getlink1();
    delete firstconcatimgptr->getlink2();
    delete firstconcatimgptr;
  }
  if(inbuffptr != NULL) delete inbuffptr;
  if(singleimgptr != NULL) delete singleimgptr;
  if(impulseimagefiltered != NULL) delete impulseimagefiltered;
  if(impulseimage != NULL) delete impulseimage;
}

void four1(float data[], dimension nn, int isign) {
// this routine is derived from "Numerical Recipes in C" 2nd edition
//
// Change data so externally it runs from 0 to ntot-1
// here it will run from 1 to ntot
  data = data - 1;

  dimension n = nn << 1;

  // bit reversal
  dimension m, i;
  for(dimension i=1, j=1; i<n; i+=2) {
    if(j > i) {
      SWAP(data+j, data+i);
      SWAP(data+j+1, data+i+1);
    }
    m = n >> 1;
    while((m >= 2) && (j > m)) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }

  double theta, wtemp, wpr, wpi, wr, wi;
  double tempr, tempi;
  dimension istep;
  dimension j;
  // Danielson-Lanczos algorithm
  dimension mmax = 2;
  while(n > mmax) {
    istep = mmax << 1;
    theta = isign * ((2*PI)/mmax);
    wtemp = sin(0.5 * theta);
    wpr = -2.0 * wtemp * wtemp;
    wpi = sin(theta);
    wr = 1.0;
    wi = 0.0;
    for(m = 1; m<mmax; m+=2) {
      for(i = m; i <= n; i += istep) {
	j = i + mmax;
	tempr = (wr*data[j]) - (wi*data[j + 1]);
	tempi = (wr*data[j + 1]) + (wi*data[j]);
	data[j] = data[i] - tempr;
	data[j+1] = data[i+1] - tempi;
	data[i] += tempr;
	data[i+1] += tempi;
      }
      wtemp = wr;
      wr = (wr * wpr) - (wi * wpi) + wr;
      wi = (wi * wpr) + (wtemp * wpi) + wi;
    }
    mmax = istep;
  }
}
