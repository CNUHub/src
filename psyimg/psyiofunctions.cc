#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
#include <string>
#include <cstring>
//#include <iostream.h>
#include <iostream>
//#include <fstream.h>
#include <fstream>
using namespace std;

#include "psyiofunctions.h"
#define RAD_TO_DEG (180.0/M_PI)

unitinput::unitinput()
{
  initunitinput();
}

unitinput::unitinput(string type)
{
  initunitinput(type);
}

void unitinput::initunitinput() {
  initunitinput("meters");
}

void unitinput::initunitinput(string type)
{
  int i;
  defaultunits.erase();
  if(type.empty())type="meters";
  setdefaultunits(type);
  setdefaultvalue((double)1.0);
  if(defaultunits.compare("meters")==0) {
// build list of unit names and factors to convert to meters
// changes to this list must be reflected in above unitinput::setpixelunits
    unitlist=new unitpair[27];
    i=0;
    unitlist[i].name="meters"; unitlist[i++].factor=1.0; //0
    unitlist[i].name="meter"; unitlist[i++].factor=1.0;  //1
    unitlist[i].name="m"; unitlist[i++].factor=1.0; //2
    unitlist[i].name="millimeters"; unitlist[i++].factor=0.001; //3
    unitlist[i].name="millimeter"; unitlist[i++].factor=0.001; //4
    unitlist[i].name="milli"; unitlist[i++].factor=0.001; //5
    unitlist[i].name="mm"; unitlist[i++].factor=0.001; //6
    unitlist[i].name="centimeters"; unitlist[i++].factor=0.01; //7
    unitlist[i].name="centimeter"; unitlist[i++].factor=0.01; //8
    unitlist[i].name="cm"; unitlist[i++].factor=0.01; //9
    unitlist[i].name="inches"; unitlist[i++].factor=0.0254; //10
    unitlist[i].name="inch"; unitlist[i++].factor=0.0254; //11
    unitlist[i].name="in"; unitlist[i++].factor=0.0254; //12
    unitlist[i].name="feet"; unitlist[i++].factor=0.3048; //13
    unitlist[i].name="foot"; unitlist[i++].factor=0.3048; //14
    unitlist[i].name="ft"; unitlist[i++].factor=0.3048; //15
    unitlist[i].name="yard"; unitlist[i++].factor=0.9144; //16
    unitlist[i].name="yards"; unitlist[i++].factor=0.9144; //17
    unitlist[i].name="yds"; unitlist[i++].factor=0.9144; //18
    unitlist[i].name="yd"; unitlist[i++].factor=0.9144; //19
    unitlist[i].name="mile"; unitlist[i++].factor=1609.3; //20
    unitlist[i].name="miles"; unitlist[i++].factor=1609.3; //21
    unitlist[i].name="pixels"; unitlist[i++].factor=1.0; //22
    unitlist[i].name="pixel"; unitlist[i++].factor=1.0; //23
    unitlist[i].name="pix"; unitlist[i++].factor=1.0; //24
    unitlist[i].name="p"; unitlist[i++].factor=1.0; //25
    unitlist[i].name.erase(); unitlist[i].factor=0; //26
  }
  else if(defaultunits.compare("degrees")==0){
    unitlist=new unitpair[7];
    i=0;
    unitlist[i].name="degrees"; unitlist[i++].factor=1.0; //0
    unitlist[i].name="deg."; unitlist[i++].factor=1.0;  //1
    unitlist[i].name="deg"; unitlist[i++].factor=1.0; //2
    unitlist[i].name="radians"; unitlist[i++].factor=RAD_TO_DEG; //3
    unitlist[i].name="rad."; unitlist[i++].factor=RAD_TO_DEG; //4
    unitlist[i].name="rad"; unitlist[i++].factor=RAD_TO_DEG; //5
    unitlist[i].name.erase(); unitlist[i].factor=0; //6
  }
  else {
    cerr<<"unitinput::initunitinput - don't know how to input";
    cerr<<defaultunits<<'\n';
    exit(1);
  }
}

unitinput::~unitinput()
{
  delete[] unitlist;
}

void unitinput::setdefaultvalue(double value)
{
  defaultvalue=value;
}

double unitinput::getdefaultvalue()
{
  return(defaultvalue);
}

void unitinput::setdefaultunits(string name)
{
  defaultunits=name;
}

string unitinput::getdefaultunits()
{
  return(defaultunits);
}

void unitinput::setpixelunits(double factor)
{
  unitlist[22].factor=unitlist[23].factor=unitlist[24].factor=
    unitlist[25].factor=factor;
}

double unitinput::get(string prompt, istream *instream)
{
  char str[80], c;
  double value=0;
  unitpair *unitptr;
  double displayvalue=0;
  int status;

  if(! prompt.empty()) {
// get and convert default value for display
    unitptr=unitlist;
    while(! unitptr->name.empty() && (unitptr->name.compare(defaultunits) != 0))
      unitptr++;
      if(unitptr->factor!=0)displayvalue=defaultvalue/unitptr->factor;
      else displayvalue=0;
  }
  while(1) {
    if(! prompt.empty())cout<<prompt<<" ["<<displayvalue<<defaultunits<<"]: ";
//remove initial carriage return
    if(instream->peek() == '\n')instream->get(c);
//will take 2 carriage returns to get default
    instream->get(str, 79); //always leaves carriage return
    instream->get(c); // remove left over carraige return
// parse str
    value=sget(str, &status);
    if(status==0)return(value);
// invalid units
    cout<<"unitinput::get - error invalid units\n";
  }// end while
}

double unitinput::sget(char *str, int *status)
{
  char units[40];
  double value;
  unitpair *unitptr;

  strcpy(units, defaultunits.c_str());
// get and convert default value for return
  unitptr=unitlist;
  while(! unitptr->name.empty() && (unitptr->name.compare(defaultunits) != 0))
    unitptr++;
  if(unitptr->factor!=0)value=defaultvalue/unitptr->factor;
  else value=0;
  sscanf(str,"%lf %40s", &value, units);
// parse unit list
  for(unitptr=unitlist; ! unitptr->name.empty();
      unitptr++) {
    if(unitptr->name.compare(units)==0) {
      if(defaultunits.compare(units) != 0)setdefaultunits(units);
      if(status!=NULL)*status=0;
      return(value * unitptr->factor);
    }
  }
  if(status!=NULL)*status=1;
  return(0);
}

string *read_list(const char *name, int *numberoffiles)
{
  char linebuff[1024], c; //, *cptr;
  fstream fd;
  fd.open(name, ios::in);
  if(fd.fail() || fd.bad()) {
    cerr<<"read_list - unable to open file "<<name<<'\n';
    exit(1);
  }
// first count number of files
  int numfiles=0;
  while(! fd.eof() && fd.good()) {
    //remove left over carriage return
    if(fd.peek() == '\n')fd.get(c);
    fd.get(linebuff, 1024-1);
    if(fd.good()) {
      // parse line
      if(linebuff[0] == '#' ||
	 linebuff[0] == '\0') ;//ignore blank & comment lines
      else {
	numfiles++;
      }// end else linebuff[0] != '#'
    }// end if(fd.good())
  }// end while
// next build pointer array and read values
  string *namelist=new string[numfiles+1];
  int i=0;
  for(i=0; i<numfiles+1; i++)namelist[i].erase();
  i=0;
  //  int numtemplates=0;
  //  fd.close();
  fd.clear();
  // fd.open(name, ios::in);
  fd.seekg(ios::beg);
  if(fd.fail() || fd.bad()) {
    cerr<<"read_list -  unable to seek to beginning of file "<<name<<'\n';
    exit(1);
  }
  while(! fd.eof() && fd.good()) {
    //remove left over carriage return
    if(fd.peek() == '\n')fd.get(c);
    fd.get(linebuff, 1024-1);
    if(fd.good()) {
      // parse line
      if(linebuff[0] == '#' ||
	 linebuff[0] == '\0') ;//ignore blank & comment lines
      else {
	char *token;
	token=strtok(linebuff," =\t\n\0");
	namelist[i]=token;
	i++;
      }// end else linebuff[0] != '#'
    }// end if(fd.good())
  }// end while

  fd.close();
  if(numberoffiles != NULL)*numberoffiles=numfiles;
  return(namelist);
}

void free_read_list(string* ptr)
{
  if(ptr != NULL) delete[] ptr;
}
