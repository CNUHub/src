#include "psyhdr.h"
#include <stdio.h>
//#include <strstream.h>
#include <sstream>

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean, sum;
  char *desc;
  int histset=0;
  int histbins=256;
  int histsmoothsize=11;
  int histlocalradius = 4;
  int histcofmradius=2;
  int histmaxset=0;
  double histmax;
  double histmin=0;
  int histverbose=0;
  double threshold_percent=40;
  int thresholdset=0;
  double threshold=0;
  int nofill=0;
  psyfileclass outfileclass=psynoclass;
  psytype outtype=psynotype;
  int *outfileclassinfo = NULL;

  //  ostrstream *logostr = new ostrstream();
  std::stringstream *logostr = new std::stringstream();
// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile\n";
      cout <<"       [-tp threshold_percent | -t threshold |"<<'\n';
      cout <<"        -hist [-hb histbins] [-hf histsmoothsize] [-hv]"<<'\n';
      cout <<"        [-hmin histmin] [-hmax histmax]"<<'\n';
      cout <<"        [-hlr histlocalmintestradius] [-hcofmr histcofmradius]]"<<'\n';
      cout <<"       [-nofill]"<<'\n';
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp(argv[i],"-tp")==0)&&((i+1)<argc))
      sscanf(argv[++i],"%lf",&threshold_percent);
    else if((strcmp(argv[i],"-t")==0)&&((i+1)<argc)) {
      thresholdset = 1;
      sscanf(argv[++i],"%lf",&threshold);
    }
    else if(strcmp(argv[i],"-hist")==0) histset = 1;
    else if((strcmp(argv[i],"-hb")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%ld",&histbins);
    }
    else if((strcmp(argv[i],"-hf")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%ld",&histsmoothsize);
    }
    else if((strcmp(argv[i],"-hmin")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%lf",&histmin);
    }
    else if((strcmp(argv[i],"-hmax")==0)&&((i+1)<argc)) {
      histmaxset = 1;
      sscanf(argv[++i],"%lf",&histmax);
    }
    else if((strcmp(argv[i],"-hlr")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%ld",&histlocalradius);
      if(histlocalradius < 1) {
	histlocalradius = 1;
	cerr << argv[0] << ": histogram local radius reset to 1\n";
      }
    }
    else if((strcmp(argv[i],"-hcofmr")==0)&&((i+1)<argc)) {
      sscanf(argv[++i],"%ld",&histcofmradius);
    }
    else if((strcmp(argv[i],"-hv")==0)) histverbose = 1;
    else if(strcmp("-nofill", argv[i])==0)nofill = 1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input file
  psyfileclass infileclass;
  psytype intype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &intype, &outfileclassinfo);
  if(outfileclass==psynoclass)outfileclass=infileclass;
  if(outtype==psynotype) {
    // unsigned char doesn't work well with ecat files
    if(outfileclass == ecatfileclass) outtype=psyshort;
    else outtype=psyuchar;
  }

// buffer input image
  psypgbuff inimage_buffered(inimag, 1);

  if(histset) {
    inimage_buffered.getstats(&min, &max, &mean);
//    if(! histminset) histmin = min;
    if(! histmaxset) histmax = max;
    psyhistogram histogram(&inimage_buffered, histmin, histmax, histbins);
    double histres;
    histogram.getres(&histres, NULL, NULL, NULL);
    (*logostr)<<"histmin="<<histmin<<" histmax="<<histmax<<" histbins="<<histbins<<" histres="<<histres<<"\n";
    float *histbuffer = new float[histbins];
    int bytesperword = gettypebytes(psyint);
    if(histsmoothsize > 1) {
      pixelavg histsmoothed(&histogram, psyfloat, histsmoothsize);
      histsmoothed.copyblock((char *) histbuffer, 0,0,0,0, histbins-1,0,0,0,
			     bytesperword,
			     bytesperword*histbins,
			     bytesperword*histbins,
			     bytesperword*histbins,
			     psyfloat);
      (*logostr)<<"histogram smoothed by averaging over "<<histsmoothsize<<" bins\n";
    }
    else
      histogram.copyblock((char *) histbuffer, 0,0,0,0, histbins-1,0,0,0,
			  bytesperword,
			  bytesperword*histbins,
			  bytesperword*histbins,
			  bytesperword*histbins,
			  psyfloat);
    double histminctr = histmin + (0.5 * histres);
    if(histverbose) {
      cout<<"histmin="<<histmin<<" histmax="<<histmax<<" histbins="<<histbins<<" histres="<<histres<<"\n";
      cout<<"histminctr="<<histminctr<<"\n";
    }
    (*logostr)<<"histminctr="<<histminctr<<"\n";
/*
    if(histverbose) {
      cout<<"histogram values:\n";
      for(int i=0; i<histbins; i++) {
	cout<<histbuffer[i]<<" at "<<i<<" or "<<(i*histres) + histminctr<<'\n';
      }
    }
*/

    psydims insize = inimage_buffered.getsize();
    int greatest_thresh_loc = histbins-1;
    if(! histmaxset) {
      int minpixels = (int) (0.05 * (insize.x * insize.y * insize.z * insize.i));
      float total = 0;
      while((greatest_thresh_loc >= 0) && (total < minpixels)) {
	total += histbuffer[greatest_thresh_loc];
	greatest_thresh_loc--;
      }
      greatest_thresh_loc++;

      if(histverbose) cout<<"minpixels="<<minpixels<<" total="<<total<<'\n';
    }
    if(histverbose) cout<<"greatest thresh loc="<<greatest_thresh_loc<<"\n";
    (*logostr)<<"histogram values beyond bin="<<greatest_thresh_loc<<" ignored when searching for minimum\n";

    float min1;
    int min1loc = -1;
    int start = histlocalradius; int stop = histbins - histlocalradius - 1;
    int rstart = start - histlocalradius;
    int rstop = start + histlocalradius;
    for(int x=start; x<stop; x++, rstart++, rstop++) {
      int localmax = 1;
      int localmin = 1;
      for(int r = rstart; r < rstop; r++) {
	if(r < x) {
	  if(histbuffer[r] > histbuffer[x]) localmax = 0;
	  else if(histbuffer[r] < histbuffer[x]) localmin = 0;
	  else {
	    // not peak if equal to previous pixels
	    localmax = 0; localmin = 0;
	  }
	}
	else if(r > x) {
	  // still a peak if equal to trailing pixels
	  if(histbuffer[r] > histbuffer[x]) localmax = 0;
	  else if(histbuffer[r] < histbuffer[x]) localmin = 0;
	}
      }
      if(localmin) {
	if(histverbose) cout<<"min="<<histbuffer[x]<<" at "<<x<<" or "<<(x * histres) + histminctr<<"\n";
	if((min1loc < 0) ||
	   ((histbuffer[x] < min1) && (x < greatest_thresh_loc))) {
	  min1 = histbuffer[x]; min1loc = x;
	}
      }
      if(localmax) {
	if(histverbose) cout<<"max="<<histbuffer[x]<<" at "<<x<<" or "<<(x * histres) + histminctr<<"\n";
      }
    }

    if(histverbose) cout<<"min1="<<min1<<" min1loc="<<min1loc<<" or "<<(min1loc * histres) + histminctr<<'\n';

    (*logostr)<<"histogram min search based on local radius="<<histlocalradius<<'\n';
    (*logostr)<<"histogram found min="<<min1<<" at bin location="<<min1loc<<" or "<<(min1loc * histres) + histminctr<<'\n';
    if(histcofmradius < 0) {
      // use the same radius as defined local min
      histcofmradius = histlocalradius;
    }
    if(histcofmradius > 0) {
      // find max over histlocalradius around min location
      rstart = min1loc - histcofmradius; if(rstart < 0) rstart = 0;
      rstop = min1loc + histcofmradius; if(rstop >= histbins) rstop = histbins - 1;
      double max = 0;
      for(int x = rstart; x < rstop; x++) {
	if(histbuffer[x] > max) max = histbuffer[x];
      }
      // find center of lowest mass
      double mass_product_sum = 0;
      double mass_sum = 0;
      for(int x = rstart; x < rstop; x++) {
	double inverse_mass = max - histbuffer[x];
	mass_product_sum += x * inverse_mass;
	mass_sum += inverse_mass;
      }
      if(fabs(mass_sum < 1e-16)) {
	cerr << "bldtemplate - problem calculating center of mass\n";
	exit(1);
      }
      threshold = ((mass_product_sum/mass_sum) * histres) + histminctr;

      (*logostr)<<"histogram center of mass calculated over radius="<<histcofmradius<<'\n';
      cout<<"histogram center of mass threshold="<<threshold<<"\n";
      (*logostr)<<"histogram center of mass threshold="<<threshold<<"\n";
    }
    else {
      threshold = (min1loc * histres) + histminctr;
      cout<<"histogram threshold="<<threshold<<"\n";
      (*logostr)<<"histogram threshold="<<threshold<<"\n";
    }
  }
  else if(!thresholdset) {
    inimage_buffered.getstats(&min, &max, &mean);
    threshold=threshold_percent * .01 * max;
    cout<<"threshold="<<threshold<<" ="<<100*threshold/max<<"% of max\n";
    (*logostr)<<"threshold="<<threshold<<" ="<<100*threshold/max<<"% of max\n";
  }
  else {
    cout<<"threshold="<<threshold<<"\n";
    (*logostr)<<"threshold="<<threshold<<"\n";
  }

// template the image
  psyimg *templated = NULL;
  if(nofill) {
    scaleimg *scalethresholdimg =
      new scaleimg((psyimg *)&inimage_buffered,	outtype);
    // template pixels >= threshold set to 1
    // and pixels < threshold set to 0
    // Note - scaleimg compares min thresh first so
    // all pixels < thresholded set to 0
    scalethresholdimg->set_min_thresh(threshold, 0);
    // reduce the threshold for max so that 
    // all pixels not thresholded to 0 are set to 1
    scalethresholdimg->set_max_thresh(threshold-1, 1);
    templated = (psyimg *) scalethresholdimg;
    (*logostr)<<"built template without filling\n";
  }
  else {
    templated =
    (psyimg *) new bldtemplate((psyimg *)&inimage_buffered, threshold, 1);
    (*logostr)<<"built template with filling\n";
  }
// build new description
  desc = new char[strlen(infile) + 15];
  strcpy(desc, "bldtemplate: ");
  strcat(desc, infile);
  templated->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  templated->setdate();
  templated->settime();
// output result to analyze file
  psyimg *outpsyimgptr=psynewoutfile(outfile, templated, outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
// log messages
  (*logostr)<<'\0';
  //  logostr->freeze(1); log.logmessage(logostr->str()); logostr->freeze(0);
  log.logmessage((char *) logostr->str().c_str());
// append input file logs
  log.loginfilelog(infile);
// print out templated images stats
  outpsyimgptr->getstats(&min, &max, &mean, &sum);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<" sum="<<sum<<'\n';

// clean up
  delete logostr;
  delete outpsyimgptr;
  delete templated;
  delete inimag;
}
