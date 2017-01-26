#include "psyhdr.h"

string bldlogfilename(string fname)
{
// build log file name with .log prefix
  size_t length = fname.length();
  if(length == 0) return "";
  else if(length > 5) {
    if(fname.find_last_of('.') == (length - 5)) fname.erase(length - 5);
  }
  return fname + ".log";
}

logfile::logfile(string fname, int argc, char *argv[])
{
  string fullname=bldlogfilename(fname);
// open log file
  psyopenfile(fullname, "w", &logfd);
// write current command to log file
  logfd << "time: "<< currenttime();
  logfd << " date: "<< currentdate() << '\n';
  logfd << "command:";
  for(int i=0, j=8; i<argc; i++) {
    logfd<<" "<<argv[i];
    j += 1 + strlen(argv[i]);
    if(j>=79) { logfd<<"\\\n"; j=0; }
  }
  logfd<<'\n';
}

logfile::~logfile()
{
  logfd.close();
}

void logfile::loginfilelog(string fname)
{
  fstream inlogfd;
  int status;
  if(fname.length() == 0)return;
  string fullname=bldlogfilename(fname);
  psyopenfile(fullname, "r", &inlogfd, &status);
  if(!status) {
    logfd<<"**  log from input file=\""<<fullname<<"\"  **\n\n";
    char ch;
    while(logfd && inlogfd.get(ch)) logfd<<ch;
  }
  inlogfd.close();
}

void logfile::logmessage(string message)
{
  logfd<<message;
}
