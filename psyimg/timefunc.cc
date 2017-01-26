#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
using namespace std;

int getcurrenttimeinsecs() {
  return time((time_t *)NULL);
}

string currenttime()
{
  time_t basetime=time((time_t *)NULL);
  char *ptr=new char[9];
  strftime(ptr, 9, "%T", localtime(&basetime));
  string timestr = ptr;
  delete[] ptr;
  return(timestr);
}

string currentdate()
{
  time_t basetime=time((time_t *)NULL);
  char *ptr=new char[9];
  strftime(ptr, 9, "%D", localtime(&basetime));
  string datestr = ptr;
  delete[] ptr;
  return(datestr);
}

// Purpose: To convert seconds into hours, minutes, seconds
// Input:   Number of seconds
// Output:  hours, minutes, seconds

void SecToHMS(int sec, int& hours, int& minutes, int& seconds)
{
   hours = sec/3600;
   sec -= (hours*3600);
   minutes = sec/60;
   sec -= (minutes*60);
   seconds = sec;
   return;
}

// Purpose: To print out hours, minutes seconds.
// Input:   hours, minutes, seconds
// Output:  None.

void PrintHMS(int hours, int minutes, int seconds)
{
   cout << hours << "h " << minutes << "m " << seconds << "s";
   return;
}
