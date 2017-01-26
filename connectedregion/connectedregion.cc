#include "psyhdr.h"

// define the integer type for region storage.
// note - unsigned char is usually to small
#define PSYTYPE psyushort
#define CCTYPE unsigned short
// largest positive integer that can be stored in the above type
#define LARGEST_REGION_NUMBER 65535

class regionStats {
  int region;
  int cnt;
  xyzint moments;
  xyzint min;
  xyzint max;
public:
  regionStats(int region, int x, int y, int z) {
    regionStats::region = region;
    cnt = 1;
    moments.x = min.x = max.x = x;
    moments.y = min.y = max.y = y;
    moments.z = min.z = max.z = z;
  }
  void increment(int x, int y, int z) {
    cnt++;
    moments.x += x; moments.y += y; moments.z += z;
    if(x < min.x) min.x = x;
    else if(x > max.x) max.x = x;
    if(y < min.y) min.y = y;
    else if(y > max.y) max.y = y;
    if(z < min.z) min.z = z;
    else if(z > max.z) max.z = z;
  }
  int getregion() { return region; }
  int getcount() { return cnt; }
  xyzint getmoments() { return moments; }
  xyzdouble getcenterofmass() {
    xyzdouble cofm;
    if(cnt > 0) {
      cofm.x = (double) moments.x / (double) cnt;
      cofm.y = (double) moments.y / (double) cnt;
      cofm.z = (double) moments.z / (double) cnt;
    }
    return cofm;
  }
  xyzint getmin() { return min; }
  xyzint getmax() { return max; }
  void printRegion(ostream &output) {
    output << "region=" << region;
    output << " count=" << cnt;
    xyzdouble cofm = getcenterofmass();
    output << " center_of_mass=(" << cofm.x << ", " << cofm.y << ", " 
      << cofm.z << ")";
    output << " min_location=(" << min.x << ", " << min.y << ", " 
      << min.z << ")";
    output << " max_location=(" << max.x << ", " << max.y << ", " 
      << max.z << ")";
  }
};

class regionList {
public:
  int region;
  int associatedRegion;
  regionList *link;
  regionList *equivLink;
  regionList(int region, int associatedRegion, regionList *link);
  void swapAssociation() {
    int tmp = region;
    region = associatedRegion;
    associatedRegion = tmp;
  }
  int add2equiv(regionList *ptr);
};

regionList::regionList(int region, int associatedRegion, regionList *link) {
  regionList::region = region;
  regionList::associatedRegion = associatedRegion;
  regionList::link = link;
  equivLink = NULL;
}

int regionList::add2equiv(regionList *ptr) {
  regionList *lastPtr = this;
  regionList *equivPtr = this;
  while( equivPtr != NULL ) {
    if(ptr->region == equivPtr->region &&
       ptr->associatedRegion == equivPtr->associatedRegion) return 0;
    lastPtr = equivPtr;
    equivPtr = lastPtr->equivLink;
  }
  lastPtr->equivLink = ptr;
  ptr->equivLink = NULL;
  return 1;
}

class connectedRegion : public psyfullbuff {
  int lastRegion;
  int lastRegionStats;
  regionStats **regionStatsArray;
  int regionStatsArraySize;
  regionList *associations;
  int getPrimaryRegion(int region);
public:
  connectedRegion(psyimg *psyimgptr);
  ~connectedRegion();
  void chknfillbuff();
  void printList(ostream &output, regionList *ptr);
  void printRegionStats(ostream &output);
  void associateRegions(int region1, int region2);
  void mergeAssociations(regionList *ptr);
  int newRegion();
};

connectedRegion::connectedRegion(psyimg *psyimgptr) {
  initpsyfullbuff(psyimgptr, PSYTYPE);
  lastRegion = 0;
  lastRegionStats = 0;
  regionStatsArray = NULL;
  regionStatsArraySize = 0;
  associations = NULL;
}

connectedRegion::~connectedRegion() {
  // free up the stats array
  if(regionStatsArray != NULL) {
    for( int j=0; j < regionStatsArraySize; j++ ) {
      if(regionStatsArray[j] != NULL) delete regionStatsArray[j];
    }
    delete[] regionStatsArray;
  }
  // free up the associations list
  regionList *ptr = associations;
  regionList *delPtr = NULL;
  while( ptr != NULL) {
    regionList *equivPtr = ptr->equivLink;
    while(equivPtr != NULL) {
      delPtr = equivPtr;
      equivPtr = equivPtr->equivLink;
      delete delPtr;
    }
    delPtr = ptr;
    ptr = ptr->link;
    delete delPtr;
  }
}

int connectedRegion::newRegion() {
  if(lastRegion == LARGEST_REGION_NUMBER)
    cerr << "Warning -- largest region number " << lastRegion << " exceeded"
      << "\n further regions will be merged into this last region\n";
  else ++lastRegion;
  return lastRegion;
}

void connectedRegion::associateRegions( int region1, int region2) {
  if(region1 == region2) return;
  if(region2 < region1) {
    int tmp=region2; region2 = region1; region1 = tmp;
  }
/*
// no duplicate associations
  regionList *ptr = associations;
  while( ptr != NULL) {
    if(ptr->region == region1 && ptr->associatedRegion == region2) return;
    ptr = ptr->link;
  }
*/
  associations = new regionList(region1, region2, associations);
}

int connectedRegion::getPrimaryRegion(int region) {
  regionList *ptr = associations;
  while( ptr != NULL) {
    if(ptr->region == region) return ptr->region;
    if(ptr->associatedRegion == region) return ptr->region;
    regionList *equivPtr = ptr->equivLink;
    while(equivPtr != NULL) {
      if(equivPtr->associatedRegion == region) return ptr->region;
      equivPtr = equivPtr->equivLink;
    }
    ptr = ptr->link;
  }
  return region;
}

void connectedRegion::printList(ostream &output, regionList *ptr) {
  while( ptr != NULL) {
    output<< "(" << ptr->region <<", "<< ptr->associatedRegion << ")";
    regionList *equivPtr = ptr->equivLink;
    while(equivPtr != NULL) {
      output<< " <" << equivPtr->region <<", "<< equivPtr->associatedRegion;
      output<< ">";

      equivPtr = equivPtr->equivLink;
    }
    output<<"\n";
    ptr = ptr->link;
  }
}

void connectedRegion::printRegionStats(ostream &output) {
  chknfillbuff();
// print the region stats
  if(regionStatsArray != NULL) {
    for(int j=0; j < regionStatsArraySize; j++) {
      if(regionStatsArray[j] != NULL) {
	regionStatsArray[j]->printRegion(output);
	output << "\n";
      }
    }
  }
}

void connectedRegion::chknfillbuff() {
  if(buffimage.getbuff() != NULL) return;
  psyfullbuff::chknfillbuff();
  // find connections
  psydims inc = buffimage.getinc();
  // negative flags to insure negative indices wont reach behind origin
  enum  negativeFlags {negX=1, negY=2, negZ=4};
  // relative indices to search backwords for previously assigned regions
  int negFlags[13];
  int index[13];
  // closest
  index[0] = -inc.x;                  negFlags[0] = negX;
  index[1] = -inc.y + inc.x;          negFlags[1] = negY;
  index[2] = -inc.y;                  negFlags[2] = negY;
  index[3] = -inc.y - inc.x;          negFlags[3] = negX | negY;
  index[4] = -inc.z + inc.y + inc.x;  negFlags[4] = negZ;
  index[5] = -inc.z + inc.y;          negFlags[5] = negZ;
  index[6] = -inc.z + inc.y - inc.x;  negFlags[6] = negX | negZ;
  index[7] = -inc.z + inc.x;          negFlags[7] = negZ;
  index[8] = -inc.z;                  negFlags[8] = negZ;
  index[9] = -inc.z - inc.x;          negFlags[9] = negX | negZ;
  index[10] = -inc.z - inc.y + inc.x; negFlags[10] = negY | negZ;
  index[11] = -inc.z - inc.y;         negFlags[11] = negY | negZ;
  index[12] = -inc.z - inc.y - inc.x; negFlags[12] = negX | negY | negZ;
  // farthest

//cerr<<" in chknfillbuff starting first loop\n";
// now loop through data
  char *buff = buffimage.getbuff();
  psydims orig = buffimage.getorig();
  psydims end = buffimage.getend();
  char *iptr = buff;
  int i=0;
  for(i = orig.i; i <= end.i; i++, iptr += inc.i) {
    int negFlagsZ = negZ;
    char *zptr = iptr;
    for(int z = orig.z; z <= end.z; z++, zptr += inc.z, negFlagsZ = 0) {
      int negFlagsY = negFlagsZ | negY;
      char *yptr = zptr;
      for(int y = orig.y; y <= end.y; y++, yptr += inc.y,
	  negFlagsY = negFlagsZ) {
	int negFlagsX = negFlagsY | negX;
	char *xptr = yptr;
	for(int x = orig.x; x <= end.x; x++, xptr += inc.x,
	    negFlagsX = negFlagsY) {
	  CCTYPE *vptr = (CCTYPE *) xptr;
          if( (*vptr) < 1) *vptr = 0;
	  else {
	    // search for previous processed point connected to this
	    *vptr = 0;
	    for( int j = 0; j < 13; j++) {
	      if( (negFlagsX & negFlags[j]) == 0) {
		CCTYPE *v2ptr = (CCTYPE *) (xptr + index[j]);
		if( *v2ptr != 0 ) {
		  if(*vptr == 0) *vptr = *v2ptr;
		  else associateRegions(*vptr, *v2ptr);
		}
	      }
	    }
	    if(*vptr == 0) *vptr = newRegion();
	  }
	}
      }
    }
  }
//cerr<<"number of regions allocated="<<lastRegion<<"\n";
//cerr<<" merging associations\n";
  mergeAssociations(associations);
//  printList(&cout, associations);
  // build array to track region stats and assign continous integers to regions
  regionStatsArraySize = lastRegion;
  regionStatsArray = new regionStats *[regionStatsArraySize];
  int j=0;
  for( j=0; j<regionStatsArraySize; j++ ) regionStatsArray[j] = NULL;
//cerr<<" starting second loop\n";
  // one more pass to apply the merged values
  iptr = buff;
  for(i = orig.i; i <= end.i; i++, iptr += inc.i) {
    char *zptr = iptr;
    for(int z = orig.z; z <= end.z; z++, zptr += inc.z) {
      char *yptr = zptr;
      for(int y = orig.y; y <= end.y; y++, yptr += inc.y) {
	char *xptr = yptr;
	for(int x = orig.x; x <= end.x; x++, xptr += inc.x) {
	  CCTYPE *vptr = (CCTYPE *) xptr;
	  if( *vptr != 0) {
	    *vptr = getPrimaryRegion(*vptr);
	    int indice = *vptr - 1;
	    if(regionStatsArray[indice] == NULL) {
	      regionStatsArray[indice] =
		new regionStats(++lastRegionStats, x, y, z);
	    }
	    else regionStatsArray[indice]->increment(x, y, z);
	    *vptr = regionStatsArray[indice]->getregion();
	  }
	}
      }
    }
  }

  // arrange region stats by region number
  int tmpStatsArraySize = lastRegionStats;
  regionStats **tmpStatsArray = new regionStats *[tmpStatsArraySize];
  for(j=0; j < regionStatsArraySize; j++) {
    if(regionStatsArray[j] != NULL)
      tmpStatsArray[regionStatsArray[j]->getregion()-1] = regionStatsArray[j];
  }
  delete[] regionStatsArray;
  regionStatsArray = tmpStatsArray;
  regionStatsArraySize = tmpStatsArraySize;
  

//cerr<<"number of regions used="<<lastRegionStats<<"\n";
  
}

void connectedRegion::mergeAssociations(regionList *ptr) {
  while(ptr != NULL) {
    int primaryRegion = ptr->region;
    // first find all associations containing this primary region
    regionList *prevPtr = ptr;
    regionList *ptr2 = prevPtr->link;
    while(ptr2 != NULL) {
      if(ptr2->associatedRegion == primaryRegion) ptr2->swapAssociation();
      if(ptr2->region == primaryRegion) {
	regionList *addPtr = ptr2;  // save association to add to equiv list
	ptr2 = ptr2->link; prevPtr->link = ptr2; // remove from link list
	// add to equiv
	if( ptr->add2equiv(addPtr) == 0 ) delete addPtr; // not added delete
      }
      else {
        prevPtr = ptr2; ptr2 = ptr2->link; // jump to next pointer
      }
    }
    // Now find all associations containing secondary regions
    regionList *equivPtr = ptr;
    while(equivPtr != NULL) {
      int secondaryRegion = equivPtr->associatedRegion;
      prevPtr = ptr;
      ptr2 = prevPtr->link;
      while(ptr2 != NULL) {
	if(ptr2->associatedRegion == secondaryRegion)ptr2->swapAssociation();
	if(ptr2->region == secondaryRegion) {
	  // change region to primary region
	  ptr2->region = primaryRegion;

	  regionList *addPtr = ptr2;  // save association to add to equiv list
	  ptr2 = ptr2->link; prevPtr->link = ptr2; // remove from link list
	  // add to equiv
	  if( ptr->add2equiv(addPtr) == 0 ) delete addPtr; // not added delete
	}
	else {
	  prevPtr = ptr2; ptr2 = ptr2->link;
	}
      }
      equivPtr = equivPtr->equivLink;
    }
    ptr = ptr->link;
  }
}

int main(int argc, char *argv[])
{
  char *infile=NULL, *outfile=NULL;
  int stats = 0;
  psytype outtype=psynotype; //defaults to input type
  psyfileclass outfileclass=psynoclass; //defaults to in file format

// parse input parameters
  psytype parsedouttype=psynotype;
  psyfileclass parsedfileclass=psynoclass;
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<2) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-stats]"<<'\n';
      cout <<"       ["; writeouttypeargchoices(&cout); cout << "]\n";
      cout <<"       ["; writeoutfileclassargchoices(&cout); cout << "]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp("-stats", argv[i])==0)stats=1;
    else if((parsedouttype=checkouttypearg(argv[i])) != psynotype)outtype=parsedouttype;
    else if((parsedfileclass = checkoutfileclassarg(argv[i])) != psynoclass)outfileclass=parsedfileclass;
    else if(infile == NULL)infile=argv[i];
    else if(outfile == NULL)outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
// open input files
  psytype intype;
  psyfileclass infileclass;
  psyimg *inputimgptr = psynewinfile(infile, &infileclass, &intype);
  if(outtype == psynotype) outtype=intype;
  if(outfileclass == psynoclass) outfileclass=infileclass;
  psypgbuff inputpgbuff(inputimgptr);
  psyimg *psyimgptr = &inputpgbuff;


// generate connected region
  connectedRegion *connectedimgptr = new connectedRegion(psyimgptr);
  psyimgptr = connectedimgptr;

// print out region statistics
  if(stats) connectedimgptr->printRegionStats(cout);

  if(outfile != NULL) {
    // build new description
    char *program_desc=" connected region image: ";
    char *desc = new char[strlen(infile) + strlen(program_desc) + 1];
    strcpy(desc, program_desc);
    strcat(desc, infile);
    psyimgptr->setdescription(desc);
    delete[] desc;
    // set date and time to current date and time
    psyimgptr->setdate();
    psyimgptr->settime();
    // output result to an image file
    psyimg *outpsyimgptr=psynewoutfile(outfile, psyimgptr,
				       outfileclass, outtype);
    // log
    logfile log(outfile, argc, argv);
    log.loginfilelog(infile);
    // print output filtered images stats
    cout<<"output image stats:";
    outpsyimgptr->showstats();
    if(outpsyimgptr != NULL)delete outpsyimgptr;
  }
  if(connectedimgptr != NULL)delete connectedimgptr;
  if(inputimgptr != NULL)delete inputimgptr;
}
