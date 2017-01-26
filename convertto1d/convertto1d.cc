#include "psyhdr.h"
#include <stdio.h>

class convertto1d : public psybuff {
  psyimg *inputpsyimg;
  psyimg *templateptr;
  double threshold;
  int invert;
public:
  convertto1d(psyimg *psyimgptr, psyimg *psytemplateptr, int invert=0, double threshold=1e-16);
  void fill();
};

convertto1d::convertto1d(psyimg *psyimgptr, psyimg *psytemplateptr,
			 int invert, double threshold) {
  // make sure template and image are the same size
  int onedim;
  convertto1d::inputpsyimg = psyimgptr;
  convertto1d::templateptr = psytemplateptr;
  convertto1d::invert = invert;
  convertto1d::threshold = threshold;
  psytemplateptr->getthreshstats(NULL,NULL,NULL, &onedim, threshold);
  cout<<"onedim="<<onedim<<'\n';
  psydims orig = psytemplateptr->getorig();
  psyres res = psytemplateptr->getres();
  double wordres = psyimgptr->getwordres();
  psydims insize = psyimgptr->getsize();
  psydims templatesize = psytemplateptr->getsize();
  if(! invert) {
    if(!psyimgptr->samedim(psytemplateptr)) {
      output_tree(&cerr);
      cerr<<"convertto1d::convertto1d - template and image not the same size\n";
      exit(1);
    }
    initbuff(onedim, 1, 1, 1, psyimgptr->gettype(),
	     orig.x, orig.y, orig.z, orig.i,
	     res.x, res.y, res.z, res.i, wordres);
  }
  else{
    if(onedim != insize.x || insize.y != 1 ||
       insize.z != 1 || insize.i != 1) {
      output_tree(&cerr);
      cerr << ":converto1d::convertto1d - input size doesn't jive with template\n";
      exit(1);
    }
    initbuff(templatesize.x, templatesize.y,
	     templatesize.z, templatesize.i,
	     psyimgptr->gettype(),
	     orig.x, orig.y, orig.z, orig.i,
	     res.x, res.y, res.z, res.i, wordres);
    
  }
  fill();
}

void convertto1d::fill()
{
  int x, y, z, i;
  char *xoptr;
  double template_value;
  char_or_largest_pixel pixel;
  char zero=0;
  psytype zerotype=psychar;

// initialization
  psytype intype=inputpsyimg->gettype();
  psytype templatetype=templateptr->gettype();

  inputpsyimg->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  templateptr->initgetpixel(templatetype, orig.x, orig.y, orig.z, orig.i);

  psydims inorig = templateptr->getorig();
  psydims inend = templateptr->getend();

  xoptr=getbuff();
  int count = 0;

  switch(invert) {
  default:
  case 0:
    for(i=inorig.i; i<=inend.i; i++) {
      for(z=inorig.z; z<=inend.z; z++) {
	for(y=inorig.y; y<=inend.y; y++) {
	  for(x=inorig.x; x<=inend.x; x++) {
	    templateptr->getnextpixel(pixel.c);
	    type2double(pixel.c, templatetype, (char *)&template_value);
	    inputpsyimg->getnextpixel(pixel.c);
	    if(template_value >= threshold) {
	      count++;
	      pixeltypechg(pixel.c, intype, xoptr, type);
	      xoptr += inc.x;
	    }
	  } // end for x
	} // end for y
      } // end for z
    } // end for i
    if(count != getsize().x) {
      cerr<<"warning - pixels passing threshold="<<count;
      cerr<<" is not equal to calculated size.x="<<getsize().x<<'\n';
    }
    setspatialtransform(NULL);
    setspatialtransform2(NULL);
    break;
  case 1:
    for(i=inorig.i; i<=inend.i; i++) {
      for(z=inorig.z; z<=inend.z; z++) {
	for(y=inorig.y; y<=inend.y; y++) {
	  for(x=inorig.x; x<=inend.x; x++) {
	    templateptr->getnextpixel(pixel.c);
	    type2double(pixel.c, templatetype, (char *)&template_value);
	    if(template_value > threshold) {
	      inputpsyimg->getnextpixel(pixel.c);
	      pixeltypechg(pixel.c, intype, xoptr, type);
	    }
	    else pixeltypechg(&zero, zerotype, xoptr, type);
	    xoptr += inc.x;
	  } // end for x
	} // end for y
      } // end for z
    } // end for i
    setspatialtransform(templateptr->getspatialtransform(), templateptr->getspatialtransformcode());
    setspatialtransform2(templateptr->getspatialtransform2(), templateptr->getspatialtransformcode2());
    break;
  }
  inputpsyimg->freegetpixel();
  templateptr->freegetpixel();
}

enum idimmodes { idim_normal, idim_excempt, idim_templaterepeat };

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max=0.0, mean;
  char *desc;
  char *template_file=NULL;
  int invert = 0;
  idimmodes idimmode = idim_normal;
  int xdim= -1;
  int ydim= -1;
  int zdim= -1;
  int idim= -1;
  psytype outtype=psynotype; // will default to input type
  psyfileclass outfileclass=psynoclass; // will default to input file format
  int *outfileclassinfo = NULL;

  psyimg *templateptr=NULL;
  double threshold=1e-16;

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile\n";
      cout <<"       [-invert [-dim x,y,z,i]]"<<'\n';
      cout <<"       [-tf template_file [-t threshold]]"<<'\n';
      cout <<"       [-iexcempt | -irepeat]"<<'\n';
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-invert")==0) invert = 1;
    else if((strcmp(argv[i],"-tf")==0)&&((i+1)<argc)) {
      template_file=argv[++i];
    }
    else if((strcmp(argv[i],"-t")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf", &threshold) != 1) {
	cerr << argv[0] << ": invalid threshold =: " << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-dim")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%d,%d,%d,%d",
		&xdim, &ydim, &zdim, &idim) != 4) {
	cerr << argv[0] << ": invalid dim values =: " << argv[i] << '\n';
	exit(1);
      }
    }
    else if(strcmp("-iexcempt", argv[i])==0)idimmode=idim_excempt;
    else if(strcmp("-irepeat", argv[i])==0)idimmode=idim_templaterepeat;
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
  psytype infiletype;
  psyimg *inimag=psynewinfile(infile, &infileclass, &infiletype,
			      &outfileclassinfo);
  if(outtype==psynotype)outtype=infiletype;
  if(outfileclass==psynoclass)outfileclass=infileclass;

// buffer the input
  psypgbuff inimagbuff(inimag, 1);

// get the template source
  if(template_file != NULL) {
    if(xdim > 0 | ydim > 0 | xdim > 0 | zdim > 0)
      cerr<<"warning - user input dimensions, via -dim option, ignored\n";
    templateptr = (psyimg *) psynewinfile(template_file);
    cout<<"using template file="<<template_file<<"\n";
  }
  else {
    if(invert) {
      if(xdim < 1 | ydim < 1 | zdim < 1 | idim < 1) {
	cerr << argv[0] << ": need valid dims to invert without template\n";
	exit(1);
      }
      psyres res = inimagbuff.getres();
      psydims orig = inimagbuff.getorig();
      double wres = inimagbuff.getwordres();
      templateptr = new psyimgconstant(1.0, xdim, ydim, zdim, idim,
				       psyshort,
				       orig.x, orig.y, orig.z, orig.i,
				       res.x, res.y, res.z, res.i,
				       wres);
    }
    else templateptr = new psyimgconstant((psyimg *)&inimagbuff);
  }

// buffer the template source
  templateptr= (psyimg *) new psypgbuff(templateptr, 1);
  psyimg *convertedptr = NULL;

// template the input image
  psydims templatesize=templateptr->getsize();
  psydims in_beg=inimagbuff.getorig();
  psydims in_end=inimagbuff.getend();
  psydims in_size=inimagbuff.getsize();
  int catdim = 3; // for not colapsing idim
  switch(idimmode) {
  case idim_templaterepeat:
    catdim = 0;  // otherwise everything the same as idim_excempt
  case idim_excempt:
    if(templatesize.i != 1) {
      cerr<<"error - i dimension of template file must be 1 to excempt idim or repeat template across idim\n";
      exit(1);
    }
    for(int i=0; i<in_size.i; i++) {
      psyimg *tmpptr = new psyimgblk((psyimg *)&inimagbuff,
				     in_beg.x, in_beg.y, in_beg.z, i,
				     in_end.x, in_end.y, in_end.z, i, 1);
      psyimg *convertedtmp = new convertto1d(tmpptr, templateptr, invert, threshold);
      if(convertedptr == NULL) convertedptr = convertedtmp;
      else convertedptr = new concatimgs(convertedptr, convertedtmp, catdim);
    }
    break;
  default:
  case idim_normal:
    convertedptr = new convertto1d((psyimg *)&inimagbuff, templateptr, invert, threshold);
    break;
  }

// build new description
  desc = new char[strlen(infile) + 15];
  strcpy(desc, "convertto1d: ");
  strcat(desc, infile);
  convertedptr->setdescription(desc);
  delete[] desc;
// set date and time to current date and time
  convertedptr->setdate();
  convertedptr->settime();
// output result to file
  psyimg *outpsyimgptr=psynewoutfile(outfile, convertedptr,
				     outfileclass, outtype,
				     outfileclassinfo);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
  if(template_file != NULL)log.loginfilelog(template_file);
// print out convertedto1d images stats
  outpsyimgptr->getstats(&min, &max, &mean);
  cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';

// clean up
  if(convertedptr != NULL) delete convertedptr;
  if(templateptr)delete templateptr;
  delete outpsyimgptr;
  delete inimag;
}
