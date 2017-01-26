#include "psyhdr.h"
#include <time.h>
#include <limits.h>

enum {FALSE=0, TRUE};

bldtemplate::bldtemplate() : psypgbuff()
{
  top_most=bottom_most=NULL;
}

bldtemplate::bldtemplate(psyimg *psyimgptr, double threshold,
			   int maxnumpages)
{
  initpgbuff(psyimgptr, maxnumpages, psyuchar);
  bldtemplate::threshold=threshold;
  top_most = new int[size.x];
  bottom_most = new int[size.x];
}

bldtemplate::~bldtemplate()
{
  if(top_most != NULL)delete[] top_most;
  if(bottom_most != NULL)delete[] bottom_most;
}

void bldtemplate::fillpage(char *buff, int z, int i)
{
  int x, y;
  char *xoptr, *yoptr;
  unsigned char value;
  char_or_largest_pixel pixel;
  double dpixel;
  int right_most;
  int most_index;

// initialization
  psytype intype=inputpsyimg->gettype();
  for(x=orig.x, most_index=0; x<=end.x; x++, most_index++) {
    top_most[most_index]=end.y;
    bottom_most[most_index]=orig.y;
  }

// first fill page checking in x from left and right
// keeping track of top bottom right most pixel
// for one pass input processing

  inputpsyimg->initgetpixel(intype, orig.x, orig.y, z, i);

  yoptr=buff;
  for(y=orig.y; y<=end.y; y++, yoptr+=inc.y) {
// set all values to 0 until threshold is reached
    value=0;
// keep track of right most pixel above threshold
    right_most=end.x;
    for(x=orig.x, xoptr=yoptr, most_index=0; x<=end.x;
	x++, xoptr+=inc.x, most_index++) {
      inputpsyimg->getnextpixel(pixel.c);
      type2double(pixel.c, intype, (char *)&dpixel);
      if(dpixel >= threshold) {
	value=1;
	right_most=x;
	top_most[most_index]=min(y, top_most[most_index]);
	bottom_most[most_index]=max(y, bottom_most[most_index]);
      }
      pixeltypechg((char *)&value, psyuchar, xoptr, type);
    } // end for x
// reset pixels from right to last pixel above thresh
    value=0;
    xoptr -= inc.x;
    for(x=end.x; x>right_most; x--, xoptr-=inc.x) {
      pixeltypechg((char *)&value, psyuchar, xoptr, type);
    }
  } // end for y
  inputpsyimg->freegetpixel();

// now fill in y from front and back
  value=0;
  xoptr=buff;
  for(x=orig.x, most_index=0; x<=end.x; x++, xoptr+=inc.x, most_index++) {
// fill from front to front_most
    for(y=orig.y, yoptr=xoptr; y<top_most[most_index]; y++, yoptr+=inc.y) {
      pixeltypechg((char *)&value, psyuchar, yoptr, type);
    }
// fill from bottom to bottom_most;
    if(bottom_most[most_index] >= top_most[most_index]) {
      for(y=end.y, yoptr=xoptr+inc.z-inc.y; y>bottom_most[most_index];
	  y--, yoptr-=inc.y) {
	pixeltypechg((char *)&value, psyuchar, yoptr, type);
      }
    }
  } // end for x
}


templateimg::templateimg() : psypgbuff()
{
  templateptr=NULL;
  threshold=1e-16;
  default_value=0;
}

templateimg::templateimg(psyimg *psyimgptr, psyimg *templateptr,
			 int maxnumpages,
			 double threshold,
			 double default_value)
     : psypgbuff(psyimgptr, maxnumpages)
{
  //  int xdim, ydim, zdim, idim;
  templateimg::templateptr=templateptr;
  templateimg::threshold=threshold;
  templateimg::default_value=default_value;
// make sure template and image are the same size
  if(!samedim((psyimg *)templateptr)) {
    cout<<"templateimg::templateimg - template and image not the same size\n";
    exit(1);
  }
}

void templateimg::setthresholdvalues(double threshold, double default_value) {
  templateimg::threshold=threshold;
  templateimg::default_value=default_value;
  psypgbuff::reset(inputpsyimg);
}

void templateimg::reset(psyimg *psyimgptr, psyimg *templateptr)
{
  psypgbuff::reset(psyimgptr);
  if(templateptr!=NULL)templateimg::templateptr=templateptr;
// make sure template and image are the same size
  if(!samedim((psyimg *)templateimg::templateptr)) {
    cout<<"templateimg::reset - template and image not the same size\n";
    exit(1);
  }
}

void templateimg::fillpage(char *buff, int z, int i)
{
  int x, y;
  char *xoptr, *yoptr;
  double template_value;
  char_or_largest_pixel pixel;

// initialization
  psytype intype=inputpsyimg->gettype();
  psytype templatetype=templateptr->gettype();

  inputpsyimg->initgetpixel(intype, orig.x, orig.y, z, i);
  templateptr->initgetpixel(templatetype, orig.x, orig.y, z, i);

  yoptr=buff;
  for(y=orig.y; y<=end.y; y++, yoptr+=inc.y) {
    for(x=orig.x, xoptr=yoptr; x<=end.x; x++, xoptr+=inc.x) {
      templateptr->getnextpixel(pixel.c);
      type2double(pixel.c, templatetype, (char *)&template_value);
      inputpsyimg->getnextpixel(pixel.c);
      if(template_value > threshold)pixeltypechg(pixel.c, intype, xoptr, type);
      else pixeltypechg((char *)&default_value, psydouble, xoptr, type);
    } // end for x
  } // end for y
  inputpsyimg->freegetpixel();
  templateptr->freegetpixel();
}

void templateimg::gettemplatedstats(double *min, double *max, double *mean,
				    int *pixelsused,
				    double *sum, double *sqrsum,
				    double *var, double *adev,
				    psydims *min_location,
				    psydims *max_location,
				    xyzidouble *center_of_mass)
{
  double EPS = .000000001;
  double localsum, localsqrsum, localmin, localmax, localmean;
  double localadev, localvar;
  localsum=localsqrsum=localmin=localmax=localmean=localadev=localvar=0;
  int localpixelsused;
  psydims local_min_location;
  psydims local_max_location;
  xyzidouble spatial_moments;
  double dpixel;
  char_or_largest_pixel pixel, tpixel;
// initialization
  psytype intype=inputpsyimg->gettype();
  psytype templatetype=templateptr->gettype();

// loop through all pixels
// initialize getpixel to first pixel
  inputpsyimg->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
  templateptr->initgetpixel(templatetype, orig.x, orig.y, orig.z, orig.i);
  localpixelsused=0;
  for(int i=0; i<size.i; i++) {
    for(int z=0; z<size.z; z++) {
      for(int y=0; y<size.y; y++) {
	for(int x=0; x<size.x; x++) {
	  inputpsyimg->getnextpixel(pixel.c);
	  templateptr->getnextpixel(tpixel.c);
	  type2double(tpixel.c, templatetype, (char *)&dpixel);
	  if(dpixel > threshold) {
	    //first to output type
	    pixeltypechg(pixel.c, intype, tpixel.c, type);
	    type2double(tpixel.c, type, (char *)&dpixel);
	    if(localpixelsused == 0) {
	      localmin = localmax = localsum = dpixel;
	      local_min_location.x = x; local_min_location.y = y;
	      local_min_location.z = z; local_min_location.i = i;
	      local_max_location = local_min_location;
	      localsqrsum = dpixel*dpixel;
	      spatial_moments.x = x * dpixel;
	      spatial_moments.y = y * dpixel;
	      spatial_moments.z = z * dpixel;
	      spatial_moments.i = i * dpixel;
	    }
	    else {
	      if(dpixel > localmax){
		localmax=dpixel;
		local_max_location.x = x; local_max_location.y = y;
		local_max_location.z = z; local_max_location.i = i;
	      }
	      else if(dpixel < localmin){
		localmin=dpixel;
		local_min_location.x = x; local_min_location.y = y;
		local_min_location.z = z; local_min_location.i = i;
	      }
	      localsum += dpixel;
	      localsqrsum += dpixel*dpixel;
	      spatial_moments.x += x * dpixel;
	      spatial_moments.y += y * dpixel;
	      spatial_moments.z += z * dpixel;
	      spatial_moments.i += i * dpixel;
	    }
	    localpixelsused++;
	  }
	}
      }
    }
  }
  inputpsyimg->freegetpixel();
  templateptr->freegetpixel();

  if(localpixelsused == 0){
// avoid possible divide by zero and set unset values to zero
    localmean=0;
    localmin=localmax=localsum=localsqrsum=0;
    local_min_location.x = local_min_location.y = -1;
    local_min_location.z = local_min_location.i = -1;
    local_max_location = local_min_location;
    localadev=localvar=0;
    spatial_moments.x = spatial_moments.y = 0;
    spatial_moments.z = spatial_moments.i = 0;
  }
  else {
    localmean=localsum/(localpixelsused);

    if((adev!=NULL) || (var!=NULL)) {
// second pass processing - reduces round off error
// implementation of ideas from Numerical Recipes in C, 2nd edition
      double deviation, pdev, sumdev;
      sumdev=0;
// loop through all pixels
// initialize getpixel to first pixel
      inputpsyimg->initgetpixel(intype, orig.x, orig.y, orig.z, orig.i);
      templateptr->initgetpixel(templatetype, orig.x, orig.y, orig.z, orig.i);
      int firstpixel=1;
      int numwords=size.x * size.y * size.z * size.i;
      for(int i=0; i<numwords; i++) {
        inputpsyimg->getnextpixel(pixel.c);
        templateptr->getnextpixel(tpixel.c);
        type2double(tpixel.c, templatetype, (char *)&dpixel);
	if(dpixel > threshold) {
	  pixeltypechg(pixel.c, intype, tpixel.c, type); //first to output type
	  type2double(tpixel.c, type, (char *)&dpixel);
	  deviation = dpixel-localmean;
	  pdev = deviation * deviation;
	  if(firstpixel == 1) {
	    firstpixel=0;
	    localadev = fabs(deviation);
	    sumdev = deviation;
	    localvar = pdev;
	  }
	  else {
	    localadev += fabs(deviation);
	    sumdev += deviation;
	    localvar += pdev;
	  }
	}
      }
      inputpsyimg->freegetpixel();
      templateptr->freegetpixel();

      localadev /= localpixelsused;
      localvar -= (sumdev * sumdev)/localpixelsused; //mean round off error
      localvar /= localpixelsused - 1;
    } //end if((adev
  }
// return values
  if(min != NULL)*min = localmin;
  if(max != NULL)*max = localmax;
  if(mean != NULL)*mean = localmean;
  if(pixelsused != NULL)*pixelsused = localpixelsused;
  if(sum != NULL)*sum = localsum;
  if(sqrsum != NULL)*sqrsum = localsqrsum;
  if(adev != NULL)*adev = localadev;
  if(var != NULL)*var = localvar;
  if(min_location != NULL) *min_location = local_min_location;
  if(max_location != NULL) *max_location = local_max_location;
  if(center_of_mass != NULL) {
    if(fabs(localsum) >= EPS)
      *center_of_mass = spatial_moments * (1.0/localsum);
    else
      center_of_mass->x = center_of_mass->y =
	center_of_mass->z = center_of_mass->z = 0;
  }
}

void outputindice(FILE *fp, double x, double y, double z, double value)
{
  fprintf(fp, "%lf %lf %lf %lf\n",
	  x, y, z, value);
}

void outputindice2(FILE *fp, double x, double y, double z, double value)
{
  fprintf(fp, "(%lf %lf %lf %lf)",
	  x, y, z, value);
}

void FindBrainExtent(psyimg *inimageptr, double Threshold,
		     int *pass_xMinExtent, int *pass_xMaxExtent, 
		     int *pass_yMinExtent, int *pass_yMaxExtent,
		     int *pass_zMinExtent, int *pass_zMaxExtent,
		     int debug)
{
  xyzint MinExtent, MaxExtent;
  psydims origin, end, Size;
  double dpixel;
  char_or_largest_pixel pixel;
  xyzint Raw;              // x,y,z indices into RawImage  [-] 
  psytype intype=inimageptr->gettype();
  Size = inimageptr->getsize();
  //  int slice;
  int planepixelabovethresh;

  // Find the rectangular brain extent based on the user input threshold.

  if(debug) cout << "Using a threshold pixel value of " << Threshold
    << " to find brain extents." << "\n";

  // Initialize global brain extents.
  origin=inimageptr->getorig();
  end=inimageptr->getend();

  MaxExtent = (xyzint)origin + -1;
  MinExtent = (xyzint)end + 1;

  // search planes forward for y extents to allow tracking min z extent
  for (Raw.z = origin.z; Raw.z <= end.z; Raw.z++) {
    planepixelabovethresh=0;
    // Find the plane y extent
    for (Raw.x = origin.x; Raw.x <= end.x; Raw.x++){
      for (Raw.y = end.y; Raw.y > MaxExtent.y; Raw.y--){
	inimageptr->getpixel(pixel.c, intype,
			     Raw.x, Raw.y, Raw.z, origin.i);
	type2double(pixel.c, intype, (char *)&dpixel);
	if (dpixel >= Threshold){
	  MaxExtent.y=Raw.y;
	  planepixelabovethresh=1;
	  break;
	}
      }
      for (Raw.y = origin.y; Raw.y < MinExtent.y; Raw.y++){
	inimageptr->getpixel(pixel.c, intype,
			     Raw.x, Raw.y, Raw.z, origin.i);
	type2double(pixel.c, intype, (char *)&dpixel);
	if (dpixel >= Threshold){
	  MinExtent.y=Raw.y;
	  planepixelabovethresh=1;
	  break;
	}
      }
    }
    if(planepixelabovethresh && Raw.z < MinExtent.z)MinExtent.z=Raw.z;
  } // end for(Raw.z

  // search planes backward for x extents to allow tracking max z extent
  for (Raw.z = end.z; Raw.z >= origin.z; Raw.z--) {
    planepixelabovethresh=0;
    // Find the slice x extent 
    for (Raw.y = MinExtent.y; Raw.y <= MaxExtent.y; Raw.y++){
      for (Raw.x = end.x; Raw.x > MaxExtent.x; Raw.x--){
	inimageptr->getpixel(pixel.c, intype,
			     Raw.x, Raw.y, Raw.z, origin.i);
	type2double(pixel.c, intype, (char *)&dpixel);
	if (dpixel >= Threshold){
	  MaxExtent.x = Raw.x;
	  planepixelabovethresh=1;
	  break;
	}
      }
      for (Raw.x = origin.x; Raw.x < MinExtent.x; Raw.x++){
	inimageptr->getpixel(pixel.c, intype,
			     Raw.x, Raw.y, Raw.z, origin.i);
	type2double(pixel.c, intype, (char *)&dpixel);
	if (dpixel >= Threshold){
	  MinExtent.x = Raw.x;
	  planepixelabovethresh=1;
	  break;
	}
      }
    }
    if(planepixelabovethresh && Raw.z > MaxExtent.z)MaxExtent.z=Raw.z;
  } // end for(Raw.z

  if(debug)cout << "\n" << "     Global Extents" << "\n"
                << "     --------------" << "\n"
		<< "        x: " << MinExtent.x << "   " 
		<< MaxExtent.x << "\n"
		<< "        y: " << MinExtent.y << "   "
		<< MaxExtent.y << "\n"
		<< "        z: " << MinExtent.z << "   " 
		<< MaxExtent.z
		<< "\n\n";

  *pass_xMinExtent = MinExtent.x;
  *pass_yMinExtent = MinExtent.y;
  *pass_zMinExtent = MinExtent.z;
  *pass_xMaxExtent = MaxExtent.x;
  *pass_yMaxExtent = MaxExtent.y;
  *pass_zMaxExtent = MaxExtent.z; 
  return;
}

void FindExtrema(string CoreName, psyimg *avgimgptr, psyimg *rawimgptr,
		 psyimg *template_imgptr, double Threshold,
		 int CubeSide, int relax_neighbor,
		 int xMinExtent, int yMinExtent, int zMinExtent,
		 int xMaxExtent, int yMaxExtent, int zMaxExtent,
		 const char *command_desc, int DEBUG)
{
  psydims ImageDim;
  avgimgptr->getsize(&ImageDim.x, &ImageDim.y, &ImageDim.z, &ImageDim.i);
  int CubeVolume = CubeSide * CubeSide * CubeSide;
  char_or_largest_pixel pixel;
  double dpixel, centerpixel;
  psytype avgtype, rawtype, templatetype;
  int xStartReg, yStartReg, zStartReg;
  int xStopReg, yStopReg, zStopReg;
  int xStartRegN, yStartRegN, zStartRegN;
  int xStopRegN, yStopRegN, zStopRegN;
  int ALLx, ALLy, ALLz; // Are all x, y or z neighbor cube sum present?
  int All26;            // Are all 26 neighbor cube sums present? 

   int NumExtrema;       // The number of local extrema 
   int NumTaggedExtrema; // The number of "extrema" that did not have
                         // 26 neighbors
   int CompGT1;          // Count of the cases where the components
                         // of the corrected location are not within
                         // 1 of the un-corrected location.
   int LenGTsqrt3;       // Count of the cases where the corrected
                         // location is greater than sqrt(3) away
                         // from the uncorrected location.
   int ActualMeans;      // Actual number of means formed with
                         // roving cube, or means examined to see
                         // if local extrema.
   int NumThreshReject;  // Even if a candidate extrema lies in the
                         // global extent it may not lie in the slice
                         // extent. Counts the number of such cases.
   int TotalMeans;       // Total number of means to form with roving
                         // cube; or to inspect for extrema.
   int xReg, yReg, zReg; // x,y,z indices into RegMean[][][] 
   int LocalExtrema;     // Voxel in RegMean[][][] a local extrema?
   int xRegN, yRegN, zRegN; // x,y,z indices into RegMean[][][] 
   // int xMin, xMax;
   int reset_xMin_xMax;
   // int xRaw;
   double COMsumX, COMsumY, COMsumZ; // Center of Mass weighted sums from 
                                  // RawData[][][]
   int xRawN, yRawN, zRawN;       // x,y,z indices into RawData[][][]
   int hour, minute, second;      // Used in timing portions of code
   int shave;
   int TwoShave;         // 2*shave
   // double SumData;
   int TotalDataSets;
   double temp1, temp2, temp3, temp4;
   double percentile;    // Percentage of means formed
   double CubeSum;       // Sum of the values in the marching cube. 
   double EPS = .000000001;
   string FileName;
   int CharHolder;      // Character holder.
   time_t BaseTime;
   time_t DeltaTime;
   FILE *fpOut;
   FILE *fpTag;
   FILE *fpTemp;

   // Open the extrema output file.

   FileName = CoreName + ".out";
   
   if ((fpOut = fopen(FileName.c_str(), "w")) == 0)
   {
      cout << "Unable to open the extrema output file " << FileName
           << "\n";
      exit(1);
   }

   // Open the tagged extrema output file. Write the header.

   FileName = CoreName + ".tag";
   
   if ((fpTag = fopen(FileName.c_str(), "w")) == 0)
   {
      cout << "Unable to open the tagged output file " << FileName
           << "\n";
      exit(1);
   }

   fprintf(fpTag, "Tagged Extrema File     ");
   fprintf(fpTag, "%s\n", command_desc);
/*
   fprintf(fpTag, asctime(tmptr));
   fprintf(fpTag, "Command Parameters:");
   fprintf(fpTag, " %s ", CoreName);
   fprintf(fpTag, "-t %lf ", ThreshPercent);
   fprintf(fpTag, "-c %d ", CubeSide);
   if(relax_neighbor)fprintf(fpTag, "-r");
   fprintf(fpTag, "\n");
*/   
   // Open the temporary output file.

   if ((fpTemp = tmpfile()) == NULL)
   {
      cout << "Unable to open the temporary output file." << "\n";
      exit(1);
   }

   /* Now examine the regional mean data set for local min/max values.
      Do this by testing the center voxel Vc against its 26 adjacent
      neighbors. Look for the case where:
 
      if RelaxNeighbor is 'y':  |Vc| >  |Vi| i = 1, 26  
      if RelaxNeighbor is 'n':  |Vc| >= |Vi| i = 1, 26  

      If all 26 neighbors are not present Vc can still be an extrema;
      however it is a special case and should not be fed to gammaz. It
      will be written to the .tag file instead of the .out file. First
      it is written to a temp file. Then when all extrema have been
      found a gammaz header is put on the .out file; followed by
      copying the temp file to the .out file. 

      Note: Only the region defined by the global extent is
            examined for extrema.                                     */
    
   cout << "\n" << "Finding the local min/max extrema in the cube "
                << "sum set." << "\n";
   
   if (relax_neighbor)
   {
      cout << "RelaxNeighbor is TRUE. i.e. Extrema >= neighbors.\n";
   }
   else
   {
      cout << "RelaxNeighbor is FALSE. i.e. Extrema > neighbors.\n";
   }

   // Initialize

   avgtype=avgimgptr->gettype();
   rawtype=rawimgptr->gettype();
   templatetype=template_imgptr->gettype();
   NumExtrema = 0;
   NumTaggedExtrema = 0;
   CompGT1 = 0;
   LenGTsqrt3 = 0;
   ActualMeans = 0;
   NumThreshReject = 0; 
   shave=(CubeSide-1)/2;
   TwoShave = 2*shave;
   percentile = .10;
   BaseTime = time(NULL);

   xStartReg=max(shave, xMinExtent);
   xStopReg=min(ImageDim.x-1-shave, xMaxExtent);

   yStartReg=max(shave, yMinExtent);
   yStopReg=min(ImageDim.y-1-shave, yMaxExtent);

   zStartReg=max(shave, zMinExtent);
   zStopReg=min(ImageDim.z-1-shave, zMaxExtent);

   TotalMeans = (xStopReg-xStartReg+1)*
                (yStopReg-yStartReg+1)*
                (zStopReg-zStartReg+1);

   for (zReg = zStartReg; zReg <= zStopReg; zReg++)
   {

      zStartRegN=max(zReg-1,shave);
      zStopRegN=min(zReg+1,ImageDim.z-1 - shave);
      ALLz=(zStartRegN==(zReg-1))&&(zStopRegN==zReg+1);

      for (yReg = yStartReg; yReg <= yStopReg; yReg++)
      {
	 reset_xMin_xMax=TRUE;

	 yStartRegN=max(yReg-1,shave);
	 yStopRegN=min(yReg+1,ImageDim.y-1 - shave);
	 ALLy=(yStartRegN==(yReg-1))&&(yStopRegN==yReg+1);

	 for (xReg = xStartReg; xReg <= xStopReg; xReg++)
         {

	    xStartRegN=max(xReg-1,shave);
	    xStopRegN=min(xReg+1,ImageDim.x-1 - shave);
	    ALLx=(xStartRegN==(xReg-1))&&(xStopRegN==xReg+1);

            All26 = ALLx && ALLy && ALLz;
            LocalExtrema = TRUE;

            // use true templating
	    if(LocalExtrema) {
	      template_imgptr->getpixel(pixel.c, templatetype,
					xReg, yReg, zReg, 0);
	      type2double(pixel.c, templatetype, (char *)&dpixel);
	      if (dpixel < Threshold) {
		LocalExtrema = FALSE; 
		NumThreshReject++;
	      }
	    }

	    if(LocalExtrema) {
	      avgimgptr->getpixel(pixel.c, avgtype,
				  xReg, yReg, zReg, 0);
	      type2double(pixel.c, avgtype, (char *)&centerpixel);
	    }

            for (zRegN = zStartRegN; (zRegN<=zStopRegN)&&LocalExtrema;
		 zRegN++)
            {
	      for (yRegN = yStartRegN; (yRegN<=yStopRegN)&&LocalExtrema;
		   yRegN++)
		{
		  for (xRegN = xStartRegN; (xRegN<=xStopRegN)&&LocalExtrema;
		       xRegN++)
		    {
                     // Only consider adjacent neighbors. Don't compare
                     // RegMean[zReg][xReg][yReg] to itself; only to
                     // its (up to) 26 neighbors.

                     if ( (xRegN != xReg) || (yRegN != yReg) || 
                          (zRegN != zReg) )
                     {
	                avgimgptr->getpixel(pixel.c, avgtype,
				         xRegN, yRegN, zRegN, 0);
	                type2double(pixel.c, avgtype, (char *)&dpixel);
                        if (relax_neighbor)
                        {
                           // Extrema must be >= neighbors
                           if (fabs(centerpixel) < fabs(dpixel))
                           {
                              LocalExtrema = FALSE;
                           }
                        }
                        else
                        {
                           // Extrema must be > neighbors
                           if (fabs(centerpixel) <= fabs(dpixel))
                           {
                              LocalExtrema = FALSE;
                           }
                        }
                     }
                  } // yRegN 
               }    // xRegN
            }       // zRegN 

/* templating already done
            // The rectangular global extent whittled down the area
            // of search. Check to see if the tentative Local Extrema
            // lies outside the thresholded area of the brain. If it
            // does it is not really an extrema.

            if (LocalExtrema)
            {
               if(reset_xMin_xMax) {
                 reset_xMin_xMax=FALSE;
                 xMin = ImageDim.x-1;
                 xMax = 0;

                 for (xRaw = ImageDim.x-1; xRaw >= 0; xRaw--)
                 {
	            template_imgptr->getpixel(pixel.c, templatetype,
				              xRaw, yReg, zReg, 0);
	            type2double(pixel.c, templatetype, (char *)&dpixel);
                    if (dpixel >= Threshold)
                    {
                       xMax = max(xRaw, xMax);
                       break;
                    }
                 }

                 for (xRaw = 0; xRaw < ImageDim.x; xRaw++)
                 {
	            template_imgptr->getpixel(pixel.c, templatetype,
				             xRaw, yReg, zReg, 0);
	            type2double(pixel.c, templatetype, (char *)&dpixel);
                    if (dpixel >= Threshold)
                    {
                       xMin = min(xRaw, xMin);
                       break;
                    }
                 }
               } // end if(reset_xMin_xMax)

               if ( (xReg < xMin) || (xReg > xMax) )
               {
                  LocalExtrema = FALSE; 
                  NumThreshReject++;
               }
            }
*/
            if (LocalExtrema)
            {
               // Bump extrema/tagged extrema counts 

               if (All26)
                  {
                  NumExtrema++;
                  }
               else
                  {
                  NumTaggedExtrema++;
                  }

               if (DEBUG)
               {
                  // Put rough extrema coordinates in .out or .tag
                  // file depending on All26.
                  if (All26)
                  {
                     outputindice2(fpTemp,
                            (double)xReg, (double)yReg, (double)zReg,
                            centerpixel);
                  }
                  else
                  {
                     outputindice2(fpTag,
                            (double)xReg, (double)yReg, (double)zReg,
                            centerpixel);
                  }
               }

               // The location found for the local extrema is only
               // accurate to the nearest voxel. To refine this location
               // we return to the RawData[][][] and use a center of mass
               // method. This method uses a weighted sum of the
               // RawData[][][] values in the roving cube centered at
               // RawData[xReg][yReg][zReg]
 
               COMsumX = 0;
               COMsumY = 0;
               COMsumZ = 0;
               CubeSum=0;

               for (zRawN = zReg-shave; zRawN <= zReg+shave; zRawN++)
               {
                  for (xRawN = xReg-shave; xRawN <= xReg+shave; xRawN++)
                  {
                     for (yRawN=yReg-shave; yRawN <= yReg+shave; yRawN++)
                     {
	                rawimgptr->getpixel(pixel.c, rawtype,
				            xRawN, yRawN, zRawN, 0);
	                type2double(pixel.c, rawtype, (char *)&dpixel);
                        COMsumX += xRawN*dpixel;
                        COMsumY += yRawN*dpixel;
                        COMsumZ += zRawN*dpixel;
                        CubeSum += dpixel;
                     }
                  }
               }

//               CubeSum = GetPixel(zReg, yReg, xReg);

               // Put refined extrema coordinates in .out or .tag
               // file depending on All26.

               if (All26)
               {
                  outputindice(fpTemp,
                               COMsumX/CubeSum, COMsumY/CubeSum,
                               COMsumZ/CubeSum, CubeSum/CubeVolume);
               }
               else
               {
                  outputindice(fpTag,
                               COMsumX/CubeSum, COMsumY/CubeSum, 
                               COMsumZ/CubeSum, CubeSum/CubeVolume);
               }
 
               temp1 = xReg-COMsumX/CubeSum;
               temp2 = yReg-COMsumY/CubeSum;
               temp3 = zReg-COMsumZ/CubeSum;

               if (DEBUG > 1)
               {
                  if (All26)
                     fprintf(fpTemp, "%lf %lf %lf", temp1, temp2, temp3);
                  else
                     fprintf(fpTag, "%lf %lf %lf", temp1, temp2, temp3);
               }

               // Check if all components of the refined locations are
               // within 1 of the unrefined location. If not, then bump
               // the count.

               if ( (fabs(temp1) > (1.0+EPS)) ||
                    (fabs(temp2) > (1.0+EPS)) ||
                    (fabs(temp3) > (1.0+EPS)) )
               {
                  CompGT1++;

                  if (DEBUG > 1)
                  {
                     if (All26)
                     {
                        fprintf(fpTemp, "   ERROR\n");
                     }
                     else
                     {
                        fprintf(fpTag, "   ERROR\n");
                     }
                  }
               }
               else
               {
                  if (DEBUG > 1)
                  {
                     if (All26)
                     {
                        fprintf(fpTemp, "\n");
                     }
                     else
                     {
                        fprintf(fpTag, "\n");
                     }
                  }
               }
              
               temp1 = temp1*temp1;
               temp2 = temp2*temp2;
               temp3 = temp3*temp3;
               temp4 = sqrt(temp1+temp2+temp3);

               // Check if the refined location is within the sqrt(3) of
               // the unrefined location. If not, then bump the count.

               if (temp4 <= (sqrt(3.0)+EPS))
               {
                  if (DEBUG > 1)
                  {
                     if (All26)
                     {
                        fprintf(fpTemp, "%lf\n\n", temp4);
                     }
                     else
                     {
                        fprintf(fpTag, "%lf\n\n", temp4);
                     }
                  }
               }
               else 
               {
                  LenGTsqrt3++;

                  if (DEBUG > 1)
                  { 
                     if (All26)
                     {
                        fprintf(fpTemp, "%lf   ERROR\n\n", temp4);
                     }
                     else
                     {
                        fprintf(fpTag, "%lf   ERROR\n\n", temp4);
                     }
                  }
               }
            }

            ActualMeans++;
        
            // Give estimated time to completion at 10% intervals.

            if (ActualMeans > (TotalMeans*percentile) )
            {
               if (percentile < .99)
               {
                  cout << "   " << (int)(100*(percentile+.005)) 
                       << "% done. ";
                  DeltaTime = time(NULL) - BaseTime;
                  SecToHMS((int)(DeltaTime/percentile)-DeltaTime,
                           hour, minute, second);
                  cout << "Estimated time to completion is ";
                  PrintHMS(hour, minute, second);
                  cout << "." << "\n";
               }

               percentile += .10;
            }
         }   // xReg
      }     // yReg 
   }       // zReg

   cout << "Done looking for local extrema. Took ";
   SecToHMS((int)(time(NULL)-BaseTime), hour, minute, second);
   PrintHMS(hour, minute, second);
   TotalDataSets = ImageDim.z*ImageDim.y*ImageDim.x;
   cout << "." << "\n\n";
   cout << "There were " << NumExtrema << " extrema. This is "
        << ((double)(NumExtrema))/TotalDataSets*100 << "% of the voxels."
        << "\n";
   cout << "There were " << NumTaggedExtrema << " tagged extrema. "
        << "This is " << ((double)(NumTaggedExtrema))/TotalDataSets*100
        << "% of the voxels." << "\n";
   cout << "\n" << "There were " << NumThreshReject
                << " template rejected points within the extents." << "\n";
   cout << "\n\n";

   if (CompGT1 > 0)
   {
      cout << "WARNING: There were " << CompGT1 << " extrema" << "\n"
           << "         where the corrected components exceeded 1."
           << "\n";
   }

   if (LenGTsqrt3 > 0)
   {
      cout << "WARNING: There were " << LenGTsqrt3 << " extrema" << "\n"
           << "         which were corrected in location by more" << "\n"
           << "         than sqrt(3)." << "\n";
   }

   // Form the .out header for use by gammaz; then copy the temp file to
   // the .out file.

   rewind(fpTemp);
   fprintf(fpOut, "Extrema File     ");
   fprintf(fpOut, "%s\n", command_desc);
   fprintf(fpOut, "%d %d\n", NumExtrema, 4);
/*
   fprintf(fpOut, asctime(tmptr));
   fprintf(fpOut, "Command Parameters:");
   fprintf(fpOut, " %s ", CoreName);
   fprintf(fpOut, "-t %lf ", ThreshPercent);
   fprintf(fpOut, "-c %d ", CubeSide);
   if(relax_neighbor)fprintf(fpOut, "-r");
   fprintf(fpOut, "\n");
*/
   while ((CharHolder = getc(fpTemp)) != EOF)
   {
      putc(CharHolder, fpOut);
   }

   fclose(fpOut);
   fclose(fpTag);
   return; 
}
