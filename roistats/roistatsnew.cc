#include "psyhdr.h"
#include <stdio.h>

main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  double min, max, mean, sum, sqrsum;
  int pixelsused;
  char *desc;
  psytype outtype=psyuchar;
  psyfileclass outfileclass=psynoclass;
  char *template_file=NULL;

  xyzdouble center;
  double radius;
  xyzdouble length;
  psydims box_orig, box_end;
  int single_pixel=0;
  double threshold_percent=40;
  double threshold=0;

  psyimg *templateptr=NULL;
  psyimg *templateptrtmp=NULL;

  psyimg *template_img_buff_ptr=NULL;

  int prtmin=-1, prtmax=-1, prtmean=-1;
  int prtminloc=0, prtmaxloc=0;
  int prtpixelsused=0, prtsum=0, prtsqrsum=0;
  int prtvariance=0, prtstddev=0;
  int prtcm = 0;
  int prtadev=0;
  int prttext=1;
  int tindices[argc - 1];
  int nt=0;

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" infile [templated_outfile]\n";
      cout <<"      [ -b xorig,yorig,zorig,xend,yend,zend ]\n";
      cout <<"      [ -bc xcenter,ycenter,zcenter,xlength,ylength,zlength ]\n";
      cout <<"      [ -c xcenter,ycenter,zcenter,radius ]\n";
      cout <<"      [ -e xcenter,ycenter,zcenter,xlength,ylength,zlength ]\n";
      cout <<"      [ -tf template_file ]\n";
      cout <<"      [ -tp threshold_percent ]\n";
      cout <<"      [ -t threshold ]"<<'\n';
      cout <<"      [-min][-minloc][-max][-maxloc]\n";
      cout <<"      [-mean][-npixels][-sum][-sumsqr]\n";
      cout <<"      [-var][-stddev][-adev][-cm] [-notext]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-min")==0)prtmin=1;
    else if(strcmp(argv[i],"-max")==0)prtmax=1;
    else if(strcmp(argv[i],"-minloc")==0)prtminloc=1;
    else if(strcmp(argv[i],"-maxloc")==0)prtmaxloc=1;
    else if(strcmp(argv[i],"-mean")==0)prtmean=1;
    else if(strcmp(argv[i],"-npixels")==0)prtpixelsused=1;
    else if(strcmp(argv[i],"-sum")==0)prtsum=1;
    else if(strcmp(argv[i],"-sumsqr")==0)prtsqrsum=1;
    else if(strcmp(argv[i],"-var")==0)prtvariance=1;
    else if(strcmp(argv[i],"-stddev")==0)prtstddev=1;
    else if(strcmp(argv[i],"-adev")==0)prtadev=1;
    else if(strcmp(argv[i],"-cm")==0)prtcm=1;
    else if(strcmp(argv[i],"-notext")==0)prttext=0;
    else if((strcmp(argv[i],"-b")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-bc")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-c")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-e")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tf")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-tp")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if((strcmp(argv[i],"-t")==0)&&((i+1)<argc)) tindices[nt++] = i++;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }

// reset default print options if no print requested
  if(prtmin<=0 && prtmax<=0 && prtminloc<=0 && prtmaxloc<=0 &&
     prtmean<=0 && prtpixelsused<=0 && prtsum<=0 && prtsqrsum<=0 &&
     prtvariance<=0 && prtstddev<=0 && prtadev<=0) {
    prtmin=prtmax=prtminloc=prtmaxloc=prtmean=1;
    prtpixelsused=prtsum=prtsqrsum=1;
    prtvariance=prtstddev=prtadev=1;
    prtcm=1;
  }

// open input files
//  analyzefile inimag(infile, "r");
  psyimg *inimag=psynewinfile(infile, &outfileclass, &outtype);

// buffer the input
  psypgbuff inimagbuff(inimag, 1);

// And templates together
  for(int t=0; t<nt; t++) {
    if((strcmp(argv[tindices[t]],"-b")==0) ||
       (strcmp(argv[tindices[t]],"-bc")==0)) {
      if(strcmp(argv[tindices[t]],"-b")==0) {
	if(sscanf(argv[tindices[t]+1],"%d,%d,%d,%d,%d,%d",
		  &box_orig.x, &box_orig.y, &box_orig.z,
		  &box_end.x, &box_end.y, &box_end.z) != 6) {
	  cerr << argv[0] << ": error parsing box origin and end: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
      }
      else { // -bc
	if(sscanf(argv[tindices[t]+1],"%lf,%lf,%lf,%lf,%lf,%lf",
		  &center.x, &center.y, &center.z,
		  &length.x, &length.y, &length.z) != 6) {
	  cerr << argv[0] << ": error parsing box center and lengths: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
// calculate box region
	(xyzint)box_orig = xyzdouble2int((center - (length * 0.5)) + 0.5);
	(xyzint)box_end =  xyzdouble2int((center + (length * 0.5)) + 0.5);
      }
// keep stats box on image
      psydims orig = inimagbuff.getorig();
      psydims end = inimagbuff.getend();
      box_orig.x = clip(box_orig.x, orig.x, end.x);
      box_orig.y = clip(box_orig.y, orig.y, end.y);
      box_orig.z = clip(box_orig.z, orig.z, end.z);
      box_orig.i = orig.i;
      box_end.x = clip(box_end.x, orig.x, end.x);
      box_end.y = clip(box_end.y, orig.y, end.y);
      box_end.z = clip(box_end.z, orig.z, end.z);
      box_end.i = box_orig.i;
// create box template
      templateptrtmp=(psyimg *)
        new psyimgbox((psyimg *)&inimagbuff, box_orig, box_end);
      if((box_orig.x == box_end.x) && (box_orig.y == box_end.y) &&
         (box_orig.z == box_end.z) && (nt == 1)) {
        single_pixel=1;
        center.x=(double)box_orig.x; center.y=(double)box_orig.y;
        center.z=(double)box_orig.z;
      }
    }
    else if(strcmp(argv[tindices[t]],"-c")==0) {
      if(sscanf(argv[tindices[t]+1],"%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z, &radius) != 4) {
	cerr << argv[0] << ": error parsing sphere center and radius: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
      templateptrtmp=(psyimg *)
	new psyimgsphere((psyimg *)&inimagbuff, radius,
			 center.x, center.y, center.z);
      if((fabs(radius) <= 1e-20) && (nt == 1))single_pixel=1;
    }
    else if(strcmp(argv[tindices[t]],"-e")==0) {
      if(sscanf(argv[tindices[t]+1],"%lf,%lf,%lf,%lf,%lf,%lf",
		&center.x, &center.y, &center.z,
		&length.x, &length.y, &length.z) != 6) {
	cerr << argv[0] << ": error parsing ellipsoid center and lengths: ";
	cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	exit(1);
      }
      templateptrtmp=(psyimg *)
	new psyimgellipsoid((psyimg *)&inimagbuff, center, length);
      if((fabs(length.x) <= 1e-20) && (fabs(length.y) <= 1e-20) &&
	 (fabs(length.z) <= 1e-20) && (nt == 1))single_pixel=1;
    }
    else if(strcmp(argv[tindices[t]],"-tf")==0) {
      template_file=argv[tindices[t]+1];
// read the template
      templateptrtmp = psynewinfile(template_file);
      templateptrtmp = (psyimg *)new psypgbuff(templateptrtmp,1);
    }
    else if((strcmp(argv[tindices[t]],"-tp")==0) ||
	    (strcmp(argv[tindices[t]],"-t")==0)) {
      if(strcmp(argv[tindices[t]],"-t")==0) {
	if(sscanf(argv[tindices[t]+1],"%lf",&threshold) != 1) {
	  cerr << argv[0] << ": error parsing threshold: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
      }
      else { // -tp
	if(sscanf(argv[tindices[t]+1],"%lf",&threshold_percent) != 1) {
	  cerr << argv[0] << ": error parsing threshold percent: ";
	  cerr << argv[tindices[t]] << " " << argv[tindices[t]+1] << '\n';
	  exit(1);
	}
	inimagbuff.getstats(&min, &max, &mean);
	threshold=threshold_percent * .01 * max;
      }
// build the template with threshold
      if(prttext)
        cout<<"threshold="<<threshold<<" ="<<100*threshold/max<<"% of max\n";
// buffer the input again to build the template
      template_img_buff_ptr = (psyimg *) new psypgbuff(inimag, 1);
      templateptrtmp = (psyimg *)
        new bldtemplate(template_img_buff_ptr, threshold, 1);
    }

    if(templateptr == NULL) templateptr = templateptrtmp;
    else if(templateptrtmp != NULL) {
      templateptr = (psyimg *) new templateimg(templateptr, templateptrtmp, 1);
    }
  }

// defaults to stats on whole image
  if(templateptr == NULL)
    templateptr=(psyimg *) new psyimgconstant((psyimg *)&inimagbuff, 1.0);


// template the image
  templateimg templated((psyimg *)&inimagbuff, templateptr, 1);
// print stats
  double adev, var;
  psydims min_location, max_location;
  xyzidouble center_of_mass;
  if(single_pixel) {
// allow quicker retrieval of single pixel values
    int x=irint(center.x); int y=irint(center.y);
    int z=irint(center.z);
    if((fabs(center.x - (double)x) > 1e-20) ||
       (fabs(center.y - (double)y) > 1e-20) ||
       (fabs(center.z - (double)z) > 1e-20)) {
// to be consistent with templated, set pixelsused to zero if not exactly
// on pixel
      pixelsused=0;
    }
    else {
      char_or_largest_pixel inpixel;
      templated.getpixel(inpixel.c, templated.gettype(),
			 x, y, z, 0);
      pixeltypechg(inpixel.c, templated.gettype(), (char *)&min, psydouble);
      pixelsused=1;
      min_location.x=x; min_location.y=y; min_location.z=z; min_location.i=0;
      max_location=min_location;
      max=mean=sum=min;
      sqrsum=min*min;
      var=adev=0;
      center_of_mass = xyziint2double(min_location);
    }
  }
  else {
    templated.gettemplatedstats(&min, &max, &mean, &pixelsused, &sum, &sqrsum,
				&var, &adev, &min_location, &max_location,
				&center_of_mass);
  }
  if(pixelsused == 0) {
// just insuring consistent results
    min_location.x=-1;
    min_location.y=min_location.z=min_location.i=0;
    max_location=min_location;
    max=mean=sum=min=sqrsum=var=adev=0;
    center_of_mass = xyziint2double(min_location);
  }

  if(prttext)cout<<"stats over template region only:\n";
  if(prtmin==1){ if(prttext)cout<<"min="; cout<<min<<'\n'; }
  if(prtminloc==1){
    if(prttext)cout<<"min location=";
    cout<<'('<<min_location.x<<','<<min_location.y<<',';
    cout<<min_location.z<<','<<min_location.i<<")\n";
  }
  if(prtmax==1){ if(prttext)cout<<"max="; cout<<max<<'\n'; }
  if(prtmaxloc==1){
    if(prttext)cout<<"max location=";
    cout<<'('<<max_location.x<<','<<max_location.y<<',';
    cout<<max_location.z<<','<<max_location.i<<")\n";
  }
  if(prtmean==1){ if(prttext)cout<<"mean="; cout<<mean<<'\n'; }
  if(prtpixelsused==1){if(prttext)cout<<"pixelsused="; cout<<pixelsused<<'\n';}
  if(prtsum){ if(prttext)cout<<"sum="; cout<<sum<<'\n'; }
  if(prtsqrsum){ if(prttext)cout<<"square sum="; cout<<sqrsum<<'\n'; }
  if(prtvariance){ if(prttext)cout<<"variance="; cout<<var<<'\n'; }
  if(prtstddev){if(prttext)cout<<"standard deviation=";cout<<sqrt(var)<<'\n';}
  if(prtadev){ if(prttext)cout<<"mean absolute deviation="; cout<<adev<<'\n'; }
  if(prtcm) {
    if(prttext)cout<<"center of mass=";
    cout<<'('<<center_of_mass.x<<','<<center_of_mass.y<<',';
    cout<<center_of_mass.z<<','<<center_of_mass.i<<")\n";
  }

  if(outfile) {
// build new description
    desc = new char[strlen(infile) + strlen("roistats-templated output: ") +1];
    strcpy(desc, "roistats-templated output: ");
    strcat(desc, infile);
    templated.setdescription(desc);
    delete desc;
// set date and time to current date and time
    templated.setdate();
    templated.settime();
// output result to analyze file
    psyimg *outpsyimgptr=psynewoutfile(outfile, (psyimg*)&templated,
				       outfileclass, outtype);
// log
    logfile log(outfile, argc, argv);
    log.loginfilelog(infile);
    if(template_file != NULL)log.loginfilelog(template_file);
// print out templated images stats
    outpsyimgptr->getstats(&min, &max, &mean);
    cout<<"output image min="<<min<<" max="<<max<<" mean="<<mean<<'\n';
    delete outpsyimgptr;
  }

// clean up
  delete inimag;
/*
  if(templateptr)delete templateptr;
  if(templateimgptr)delete templateimgptr;
  if(template_img_buff_ptr)delete template_img_buff_ptr;
*/
}
