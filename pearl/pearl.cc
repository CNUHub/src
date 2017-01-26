/*
pearl.30.cc

   Purpose: read a ANALYZE .hdr and .img files and perform outlier
            analysis. Generate input files for gammaz.

   .1:
   .2: Variation -- Special case: cube sums with < 26 neighbors can be
       searched for extrema. For now they are tagged as special cases.
  
   .3: Actually write the cases with < 26 neighbors to a seperate file.

   .4: Added 2 counters: 1 for component shifts of extrema > 1;
       the other for a location shift of extrema > sqrt(3). 
       Cleaned up the DEBUGS.

       Note: There is some question as to whether these limits
             (i.e. 1 and sqrt(3) ) are meaningful.

   .5: Break early once an extrema case is not possible.

   .6: Use coherence to speed up forming the mean cube values.

   .7: Added functions imin, imax. On each slice use thresholding to
       get x,y brain extent. Also keep track of global x,y,z extent.

   .8: Use global extent to limit the range of the roving cube.
       Changed TotalMeans computation to reflect the extent.
       Also eliminate extrema from the corners of the rectangular
       brain extent.

   .9: Allow the relaxation of the extrema being absolute maxima.
  
   .10: Switch to a command line input format.

   .11: Write *.out to a temp file first; then write .out header;
        then copy temp file to .out file.

   .12: Break into modules: ProcessCommandLine

   .13: Make RawData[][][] a global.  This speeds up the cube averages
        and the neighbor extrema search.

        Break into modules: ReadImage 

   .14: Make xMinExt[MAX_Z_DIM], xMaxExt[MAX_Z_DIM], yMinExt[MAX_Z_DIM],
        and yMaxExt[MAX_Z_DIM] global.

        Break into modules: FindBrainExtent

   .15: Make RegMean[][][] global.

        Break into modules: CubeMean

   .16: Break into modules: FindExtrema 
--------------------------------------------------------------------- 
   .17: Start conversion to C++

        - #defines
        - comments
        - i/o
        - SupportFuncs.h

   .18: ProcessCommandLine became Image :: PearlInput.

   .19: FindBrainExtent became Image :: FindBrainExtent.

   .20: CubeMean became Image :: CubeSum.

   .21: FindExtrema became Image :: FindExtrema.

   .30: Converted to standard psyimg classes mainly for image i/o
        and improving C++ class usage

   .40: Modified to specify pearl maxima search space and to not template
        if if not given a template file and added check to avoid divide
	by zero when calculating center of mass.  Added optional output
	file core name with default, as before, to the same core name as
	the input file.
*/

#include "psyhdr.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

const int VERSION   =  40;

enum {FALSE=0, TRUE};

void outputindice(FILE *fp, double x, double y, double z, double value);

void outputindice2(FILE *fp, double x, double y, double z, double value);

void FindExtrema2(char *CoreName, psyimg *avgimgptr, psyimg *rawimgptr,
		 psyimg *template_imgptr, double Threshold,
		 int CubeSide,
		 int xCompareRadius, int yCompareRadius, int zCompareRadius,
		 int relax_neighbor,
		 int xMinExtent, int yMinExtent, int zMinExtent,
		 int xMaxExtent, int yMaxExtent, int zMaxExtent,
		 char *command_desc, int DEBUG=0)
{
  psydims ImageDim;
  avgimgptr->getsize(&ImageDim.x, &ImageDim.y, &ImageDim.z, &ImageDim.i);
  char_or_largest_pixel pixel;
  double dpixel, centerpixel;
  psytype avgtype, rawtype, templatetype;
  int xStartReg, yStartReg, zStartReg;
  int xStopReg, yStopReg, zStopReg;
  int xStartRegN, yStartRegN, zStartRegN;
  int xStopRegN, yStopRegN, zStopRegN;
  int ALLx, ALLy, ALLz; // Are all x, y or z neighbor cube sum present?
  int AllNbrs;            // Are all 26 neighbor cube sums present? 

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
   int xMin, xMax, reset_xMin_xMax;
   int xRaw;
   double COMsumX, COMsumY, COMsumZ; // Center of Mass weighted sums from 
                                  // RawData[][][]
   int xRawN, yRawN, zRawN;       // x,y,z indices into RawData[][][]
   int hour, minute, second;      // Used in timing portions of code
   int shave;
   double SumData;
   int TotalDataSets;
   double temp1, temp2, temp3, temp4;
   double percentile;    // Percentage of means formed
   double CubeSum;       // Sum of the values in the marching cube. 
   int CubeCnt;       // Count of number of values contributing to sum
   double EPS = .000000001;
   char *FileName = NULL;
   int CharHolder;      // Character holder.
   time_t BaseTime;
   time_t DeltaTime;
   FILE *fpOut;
   FILE *fpTag;
   FILE *fpTemp;

   // Open the extrema output file.

   FileName = new char[strlen(CoreName)+5];
   sprintf(FileName, "%s.out", CoreName);
   
   if ((fpOut = fopen(FileName, "w")) == 0)
   {
      cout << "Unable to open the extrema output file " << FileName
           << "\n";
      exit(1);
   }
   cout << "Output file is " << FileName << "\n";

   // Open the tagged extrema output file. Write the header.

   sprintf(FileName, "%s.tag", CoreName);
   
   if ((fpTag = fopen(FileName, "w")) == 0)
   {
      cout << "Unable to open the tagged output file " << FileName
           << "\n";
      exit(1);
   }

   cout << "Tagged output file is " << FileName << "\n";

   fprintf(fpTag, "Tagged Extrema File     ");
   fprintf(fpTag, "%s\n", command_desc);

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
   if(template_imgptr != NULL) templatetype=template_imgptr->gettype();
   NumExtrema = 0;
   NumTaggedExtrema = 0;
   CompGT1 = 0;
   LenGTsqrt3 = 0;
   ActualMeans = 0;
   NumThreshReject = 0; 
   shave=(CubeSide-1)/2;
   percentile = .10;
   BaseTime = time(NULL);

   xStartReg=max(shave, xMinExtent);
   xStartReg=max(xCompareRadius, xStartReg);
   xStopReg=min(ImageDim.x-1-shave, xMaxExtent);
   xStopReg=min(ImageDim.x-1-xCompareRadius, xStopReg);

   yStartReg=max(shave, yMinExtent);
   yStartReg=max(yCompareRadius, yStartReg);
   yStopReg=min(ImageDim.y-1-shave, yMaxExtent);
   yStopReg=min(ImageDim.y-1-yCompareRadius, yStopReg);

   zStartReg=max(shave, zMinExtent);
   zStartReg=max(zCompareRadius, zStartReg);
   zStopReg=min(ImageDim.z-1-shave, zMaxExtent);
   zStopReg=min(ImageDim.z-1-zCompareRadius, zStopReg);

   TotalMeans = (xStopReg-xStartReg+1)*
                (yStopReg-yStartReg+1)*
                (zStopReg-zStartReg+1);

   for (zReg = zStartReg; zReg <= zStopReg; zReg++)
   {

      zStartRegN=max(zReg-zCompareRadius,shave);
      zStopRegN=min(zReg+zCompareRadius,ImageDim.z-1 - shave);
      ALLz=(zStartRegN==(zReg-zCompareRadius)) && 
	(zStopRegN==(zReg+zCompareRadius));

      for (yReg = yStartReg; yReg <= yStopReg; yReg++)
      {
	 reset_xMin_xMax=TRUE;

	 yStartRegN=max(yReg-yCompareRadius,shave);
	 yStopRegN=min(yReg+yCompareRadius,ImageDim.y-1 - shave);
	 ALLy=(yStartRegN==(yReg-yCompareRadius)) &&
	   (yStopRegN==(yReg+yCompareRadius));

	 for (xReg = xStartReg; xReg <= xStopReg; xReg++)
         {

	    xStartRegN=max(xReg-xCompareRadius,shave);
	    xStopRegN=min(xReg+xCompareRadius,ImageDim.x-1 - shave);
	    ALLx=(xStartRegN==(xReg-xCompareRadius)) &&
	      (xStopRegN==(xReg+xCompareRadius));

            AllNbrs = ALLx && ALLy && ALLz;
            LocalExtrema = TRUE;

            // use true templating
	    if(LocalExtrema && (template_imgptr != NULL)) {
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

            if (LocalExtrema)
            {
               // Bump extrema/tagged extrema counts 

               if (AllNbrs)
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
                  // file depending on AllNbrs.
                  if (AllNbrs)
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
               CubeCnt=0;

               for (zRawN = zReg-shave; zRawN <= zReg+shave; zRawN++)
               {
                  for (xRawN = xReg-shave; xRawN <= xReg+shave; xRawN++)
                  {
                     for (yRawN = yReg-shave; yRawN <= yReg+shave; yRawN++)
                     {
	                rawimgptr->getpixel(pixel.c, rawtype,
				            xRawN, yRawN, zRawN, 0);
	                type2double(pixel.c, rawtype, (char *)&dpixel);
                        COMsumX += xRawN*dpixel;
                        COMsumY += yRawN*dpixel;
                        COMsumZ += zRawN*dpixel;
                        CubeSum += dpixel;
			CubeCnt++;
                     }
                  }
               }

	       if(fabs(CubeSum) < EPS) {
		 // if sum zero center of mass should be at center pixel
		 // avoids divide by zero
		 temp1 = xReg;
		 temp2 = yReg;
		 temp3 = zReg;
	       }
	       else {
		 temp1 = COMsumX/CubeSum;
		 temp2 = COMsumY/CubeSum;
		 temp3 = COMsumZ/CubeSum;
	       }
	       if(CubeCnt != 0) temp4 = CubeSum/CubeCnt;
	       else temp4 = 0;


               // Put refined extrema coordinates in .out or .tag
               // file depending on AllNbrs.

               if (AllNbrs) outputindice(fpTemp, temp1, temp2, temp3, temp4);
               else outputindice(fpTag, temp1, temp2, temp3, temp4);
 
               temp1 = xReg-temp1;
               temp2 = yReg-temp2;
               temp3 = zReg-temp3;

               if (DEBUG > 1)
               {
                  if (AllNbrs)
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
                     if (AllNbrs)
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
                     if (AllNbrs)
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
                     if (AllNbrs)
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
                     if (AllNbrs)
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

   while ((CharHolder = getc(fpTemp)) != EOF)
   {
      putc(CharHolder, fpOut);
   }

   fclose(fpOut);
   fclose(fpTag);
   return; 
}

int main(int argc, char *argv[])
{
   char *infile=NULL;
   char *template_file=NULL;
   char *CoreName = NULL;
   int namelength;
   char *command_desc, *ascii_time;
   int command_desc_length;
   double minvalue, maxvalue, meanvalue;
   int CubeSide=3;            // Length of marching cube side      [voxels]
   int relax_neighbor=0;    // indicates whether the
                            // extrema check should allow a cube
                            // sum to be >= to a neighbor as
                            // oppossed to >.
   double ThreshPercent=40;     // Used to threshold brain extent
   double Threshold;
   psydims ImageDim;
   int MinImageDim;         // minimum of ImageDim
   psyimg *averaged_psyimgptr;
   psyimg *template_psyimgptr;
   int xCompareRadius = 1;
   int yCompareRadius = 1;
   int zCompareRadius = 1;
   int no_filter=0;

   // Global brain extents as defined by ThreshPercent

   int xMinExtent, xMaxExtent,
       yMinExtent, yMaxExtent,
       zMinExtent, zMaxExtent;


   // Function prototypes

   time_t BaseTime;
   char *RawFileName = NULL;

   int debug=0;

   // Get the date and time for file headers.

   BaseTime = time(NULL);
   struct tm *tmptr = localtime(&BaseTime);
   cout << "\n" << "pearl." << VERSION << "   " << asctime(tmptr);

   // Parse and error check the command line.
   int i;
   for(i=0; i<argc; i++) {
     if(strcmp(argv[i], "-help") == 0 || argc<2) {
       cout <<"Usage: "<<argv[0]<<" infile [outCoreName]\n";
       cout <<"       [-c CubeSide] [-nof[ilter]]\n";
       cout <<"       [-r[elax_neighbor]] [-debug]\n";
       cout <<"       [-tf template_file | -tp threshpercent]\n";
       cout <<"       [-xr xCompareRadius] [-yr yCompareRadius] [-zr zCompareRadius]\n";
       exit(0);
     }
     else if(i==0); // ignore program name
     else if((strcmp(argv[i],"-c")==0) && ((i+1)<argc))
       sscanf(argv[++i], "%d", &CubeSide);
     else if((strcmp(argv[i],"-tp")==0) && ((i+1)<argc))
       sscanf(argv[++i], "%lf", &ThreshPercent);
     else if((strcmp(argv[i],"-tf")==0) && ((i+1)<argc))
       template_file=argv[++i];
     else if((strcmp(argv[i],"-xr")==0) && ((i+1)<argc)) {
       xCompareRadius = -1;
       sscanf(argv[++i], "%d", &xCompareRadius);
       if(xCompareRadius < 0) {
	 cerr << argv[0] << ": invalid xCompareRadius: " << argv[i] << '\n';
	 exit(1);
       }
     }
     else if((strcmp(argv[i],"-yr")==0) && ((i+1)<argc)) {
       yCompareRadius = -1;
       sscanf(argv[++i], "%d", &yCompareRadius);
       if(yCompareRadius < 0) {
	 cerr << argv[0] << ": invalid yCompareRadius: " << argv[i] << '\n';
	 exit(1);
       }
     }
     else if((strcmp(argv[i],"-zr")==0) && ((i+1)<argc)) {
       zCompareRadius = -1;
       sscanf(argv[++i], "%d", &zCompareRadius);
       if(zCompareRadius < 0) {
	 cerr << argv[0] << ": invalid zCompareRadius: " << argv[i] << '\n';
	 exit(1);
       }
     }
     else if(strcmp(argv[i],"-debug")==0)debug=1;
     else if(strncmp(argv[i],"-nof",4)==0)no_filter=1;
     else if(strncmp(argv[i],"-r",2)==0)relax_neighbor=1;
     else if(infile == NULL)infile=argv[i];
     else if(CoreName== NULL){
       CoreName = new char[strlen(argv[i]) + 1];
       strcpy(CoreName, argv[i]);
     }
     else {
       cerr << argv[0] << ": invalid parameter: " << argv[i] << '\n';
       exit(1);
     }
   }

   // build command description to put at beginning of extrema file
   ascii_time=asctime(tmptr);
   command_desc_length=strlen(ascii_time) + 1;
   command_desc_length += 11;
   for(i=0; i<argc; i++) {
     command_desc_length += strlen(argv[i]) + 1;
   }
   command_desc=new char[command_desc_length];
   strcpy(command_desc, ascii_time);
   strcat(command_desc, "command:");
   for(i=0; i<argc; i++) {
     strcat(command_desc, " ");
     strcat(command_desc, argv[i]);
   }
   // Print out parameters (some are defaults)

   cout << "\n";
   cout << "Parameters" << "\n";
   cout << "------------------" << "\n";
   cout << "infile="<< infile << "\n";
   cout << "CubeSide=" << CubeSide << "\n";
   cout << "xCompareRadius=" << xCompareRadius << "\n";
   cout << "yCompareRadius=" << yCompareRadius << "\n";
   cout << "zCompareRadius=" << zCompareRadius << "\n";
   if(no_filter)cout << "no filtering\n";
   else "filtering\n";
   if(relax_neighbor)cout << "relaxed neighbor\n";
   else cout << "no relaxed neighbor\n";
   if(template_file != NULL)cout <<"template_file="<<template_file<<'\n';
   cout << "threshpercent=" << ThreshPercent << "\n";
   cout << "\n";
   //error checks
   if(infile == NULL) {
     cerr << argv[0] << " - no input file given\n";
     exit(1);
   }
   if(CubeSide < 0 || !(CubeSide%2)) {
     cerr << argv[0] << " - invalid Cube Side\n";
     exit(1);
   }
   if ( (ThreshPercent < 0.) || (ThreshPercent > 100.) )
   {
     cerr << argv[0] << " - invalid Threshold Percent\n";
     exit(1);
   }

   // open the analyze file
   //   analyzefile inimag(infile, "r");
   psyimg *inimag=psynewinfile(infile);
   // buffer the file
   psypgbuff inimage_pgbuffered(inimag, CubeSide);

   // check image size restrictions
   ImageDim=inimage_pgbuffered.getsize();
   MinImageDim = ImageDim.x;
   MinImageDim = min(MinImageDim, ImageDim.y);
   MinImageDim = min(MinImageDim, ImageDim.z);

   if (CubeSide > MinImageDim)
   {
     cerr << argv[0] << " Cube has\n";
     cerr << "\n" << " - Cube has " << CubeSide
       << " voxels on a side."
	 << "\n" << "       This exceeds the minimum image dimension "
	   << "of " << MinImageDim << "\n";
     exit(1);
   }
   // get a template
   if(template_file == NULL) template_psyimgptr = NULL;
/*  {
     // create a template from the input image using threshpercent
     inimage_pgbuffered.getstats(&minvalue, &maxvalue, &meanvalue);
     Threshold = ThreshPercent * .01 * maxvalue;
     cout<<"using a Threshold="<<Threshold<<" to build template\n";
     template_psyimgptr=new bldtemplate((psyimg *)&inimage_pgbuffered,
					Threshold, 1);
     // template now filled with ones and zeros so set new threshold
     Threshold = .5;
   }
*/
   else {
     // open and buffer the template file
     template_psyimgptr=psynewinfile(template_file);
     template_psyimgptr=(psyimg *) new psypgbuff(template_psyimgptr, 1);
     // find the template threshold base on threshprecent
     // this input template may not be ones and zeros
     template_psyimgptr->getstats(&minvalue, &maxvalue, &meanvalue);
     Threshold = ThreshPercent * .01 * maxvalue;
   }

   if(template_psyimgptr == NULL) {
     // Find the global rectangle based on image extents
     inimage_pgbuffered.getorig(&xMinExtent, &yMinExtent, &zMinExtent, NULL);
     inimage_pgbuffered.getend(&xMaxExtent, &yMaxExtent, &zMaxExtent, NULL);
   }
   else {
     // Find the global rectangular brain extent based on the template
     FindBrainExtent(template_psyimgptr, Threshold,
		     &xMinExtent, &xMaxExtent,
		     &yMinExtent, &yMaxExtent, &zMinExtent,
		     &zMaxExtent);
   }

   if(no_filter||(CubeSide==1))
     averaged_psyimgptr=(psyimg *)&inimage_pgbuffered;
   else {
     // calculate averaged data.
     averaged_psyimgptr=
       (psyimg *)new pixelavg((psyimg *)&inimage_pgbuffered,
			      psydouble, CubeSide);
     averaged_psyimgptr=
       (psyimg *)new psypgbuff(averaged_psyimgptr, CubeSide);
   }

   if(CoreName == NULL) {
     //remove .img from input file name to form corename
     namelength=strlen(infile);
     CoreName=new char[namelength + 1];
     strcpy(CoreName, infile);
     if(namelength > 4) if(strstr(CoreName, ".img") == &CoreName[namelength-4])
       CoreName[namelength-4]='\0';
   }

   FindExtrema2(CoreName, averaged_psyimgptr, (psyimg *)&inimage_pgbuffered,
		template_psyimgptr, Threshold,
		CubeSide,
		xCompareRadius, yCompareRadius, zCompareRadius,
		relax_neighbor,
		xMinExtent, yMinExtent, zMinExtent,
		xMaxExtent, yMaxExtent, zMaxExtent,
		command_desc, debug);

// clean up
  delete[] CoreName;
  delete[] command_desc;
  delete inimag;

   cout << "-- Program completed --" << "\n\n";
}
