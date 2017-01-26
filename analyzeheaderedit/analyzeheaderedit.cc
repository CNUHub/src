#include <unistd.h>
#include <sys/wait.h>
/*#include <strstream.h>*/
#include <sstream>
#include "psyhdr.h"
#include "psyanalyze.h"
// need ecat swap routines
#include "psyecat.h"

// could not find these in include files
extern "C" char *mktemp(char *name);
// local functions defined below
dsr *lreadhdr(dsr *rawhdr, istream *in, char *oldvalues);
int hdrValueChanged(char *name, char *value, char *oldvalues);

int main(int argc, char *argv[])
{
  char *infile=NULL;
  char *outfile=NULL;
  char *editor=NULL;
  int textin = 0;

  // copied from analyze.cc
  // these moved to here because I had problem on Solaris 5.5.1 of
  // arrays being all 0 causing a core dump
  static int hk_off = 0;
  static int dime_off = 40;
  static int hist_off = 148;
  static int header_soff[] = {
    hk_off+36,
    dime_off+0, dime_off+2, dime_off+4, dime_off+6, dime_off+8, dime_off+10,
    dime_off+12, dime_off+14, dime_off+24, dime_off+30, dime_off+32,
    dime_off+34,
    -1 };
  static int header_ioff[] = {
    hk_off+0, hk_off+32,
    dime_off+92, dime_off+96, dime_off+100, dime_off+104,
    -1 };
  static int header_foff[] = {
    dime_off+36, dime_off+40, dime_off+44, dime_off+48, dime_off+52,
    dime_off+56, dime_off+60, dime_off+64, dime_off+68, dime_off+72,
    dime_off+76, dime_off+80, dime_off+84, dime_off+88,
    hist_off+168, hist_off+172, hist_off+176, hist_off+180, hist_off+184,
    hist_off+188, hist_off+192, hist_off+196,
    -1 };

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile [-textin] outfile [-ed editor | - | none]"<<'\n';
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-ed", argv[i]) == 0) && ((i+1) < argc))
      editor = argv[++i];
    else if(strcmp("-textin", argv[i]) == 0) textin = 1;
    else if(infile == NULL) infile=argv[i];
    else if(outfile == NULL) outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
  dsr *rawhdr = NULL;
  psyswaptype swaptype = psynoswap;
// read header from a text file
  if(textin) {
    fstream intmpfs(infile, ios::in);
    rawhdr = lreadhdr(NULL, &intmpfs, NULL);
    intmpfs.close();
  }
// read header from analyze header file
  else rawhdr = readAnalyzeHeader(infile, NULL, NULL, &swaptype);
  if(rawhdr == NULL) {
    cerr << argv[0] << ": error reading header for file: " <<
      infile << '\n';
    exit(1);
  }
// determine the editor
  if(editor == NULL) {
    editor = getenv("EDITOR");
    if(editor == NULL) editor = "vi";
  }
  cout << "editor=" << editor << "\n";

// none writes the header as read
  if(! (strcmp("none", editor) == 0)) {
// save the old values as a string for edit comparison
/*
    ostrstream *ost = new ostrstream();
    analyzefile::showhdr(rawhdr, ost);
    ost->put('\0');
    ost->freeze(1);
    char *oldvalues = ost->str();
    int cnt = ost->pcount();
    delete ost;
*/
    std::stringstream ss;
    analyzefile::showhdr(rawhdr, &ss);
    std::string str = ss.str();
    int cnt = str.size();
    char *oldvalues = new char[cnt + 1];
    strcpy(oldvalues, str.c_str());


    if(strcmp("-", editor) == 0) {
      // edits from standard in
      rawhdr = lreadhdr(rawhdr, &cin, oldvalues);
    }
    else {
// create a temporary file containing the old string
      char *tmpfile = new char[30 + 1];
      strcpy(tmpfile, "/tmp/analyzeheadereditXXXXXX");
      {
	mktemp(tmpfile);
	fstream outtmpfs(tmpfile, ios::out);
	outtmpfs.write(oldvalues, cnt);
	outtmpfs.close();
      }
// fork the editor on the temporary file
      pid_t pid = fork();
      if(pid == 0) {
	// child process
	cout << "in child invoking editor\n";
	execlp(editor, editor, tmpfile, (char *)0);  
	cerr<<"in child error trying to execute editor\n";
	exit(1);
      }
      else if(pid == -1) {
	// fork failed
	cerr<<"Unable to fork editor\n";
	exit(1);
      }
// parent process
// wait for child to finish
      wait(NULL);
// read the edit file and modify header accordingly
      fstream intmpfs(tmpfile, ios::in);
      rawhdr = lreadhdr(rawhdr, &intmpfs, oldvalues);
      intmpfs.close();
      unlink(tmpfile); // remove the temporary file
    }
  }
// keep the original swapping
  if(swaptype == psyreversewordbytes)
    swap_data((char *) rawhdr, header_soff, header_ioff, header_foff);
// write the new header
  writeAnalyzeHeader(rawhdr, outfile);
}

int hdrValueChanged(char *name, char *value, char *oldvalues) {
  if(oldvalues == NULL) return 1;
  if(value == NULL) return 1;
  char *cptr = strstr(oldvalues, name);
  if(cptr != NULL) {
    cptr += strlen(name) + 1;
    char *cr_ptr = strchr(cptr, '\n');
    if(cr_ptr == NULL) return 1;
    int length = cr_ptr - cptr;
    if(strlen(value) != length) return 1;
    return strncmp(value, cptr, length);
  }
  return 1;
}

dsr *lreadhdr(dsr *rawhdr, istream *in, char *oldvalues=NULL) {
  int i;
  char linebuff[1024], c;
  if(rawhdr == NULL) {
    rawhdr = new dsr();
    // initialize analyze header to zeros
    char *ptr, *endptr;
    for(ptr=(char*) rawhdr, endptr=ptr+sizeof(dsr);
	ptr<endptr; ptr++) *ptr=0;
    // set size
    rawhdr->hk.sizeof_hdr = sizeof(*rawhdr);
  }
  int changed = 0;
  int done = 0;
  while((! in->eof()) && in->good() && !done) {
    //remove left over carriage return
    if(in->peek() == '\n') in->get(c);
    in->get(linebuff, 1024-1);
    if(in->good()) {
      if(linebuff[0] != '#') { // ignore comment lines
	// parse line
	char *cptr = strchr(linebuff, '=');
	if(cptr == NULL) {
	  if(strcmp(linebuff, "end") == 0) done = 1;
	  else if(strcmp(linebuff, "exit") == 0) done = 1;
	  else if(strcmp(linebuff, "quit") == 0) done = 1;
	  else if(strcmp(linebuff, "bye") == 0) done = 1;
	  // ignore other lines without assignments
	  else cerr<<" ignoring line=" << linebuff <<"\n";
	}
	else {
	  // convert '=' to end of string character '\0'
	  *cptr='\0'; cptr++;
	  if(hdrValueChanged(linebuff, cptr, oldvalues)) {
	    changed = 1;
	    // now parse assignment
	    int itmp;
	    float ftmp;
	    if(strcmp(linebuff, "header_key.sizeof_hdr") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading header_key.sizeof_hdr\n";
		exit(1);
	      }
	      rawhdr->hk.sizeof_hdr = itmp;
	      if(itmp != sizeof(*rawhdr)) {
		cerr<<"invalid header_key.sizeof_hdr=" << itmp;
		cerr<<" should be " << sizeof(*rawhdr) << "\n";
		exit(1);
	      }
	    }
	    else if(strcmp(linebuff, "header_key.data_type") == 0) {
	      strncpy(rawhdr->hk.data_type, cptr, 10);
	    }
	    else if(strcmp(linebuff, "header_key.db_name") == 0) {
	      strncpy(rawhdr->hk.db_name, cptr, 18);
	    }
	    else if(strcmp(linebuff, "header_key.extents") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading header_key.extents\n";
		exit(1);
	      }
	      rawhdr->hk.extents = itmp;
	    }
	    else if(strcmp(linebuff, "header_key.session_error") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading header_key.session_error\n";
		exit(1);
	      }
	      rawhdr->hk.session_error = itmp;
	    }
	    else if(strcmp(linebuff, "header_key.regular") == 0) {
	      rawhdr->hk.regular = *cptr;
	    }
	    else if(strcmp(linebuff, "header_key.hkey_un0") == 0) {
	      rawhdr->hk.hkey_un0 = *cptr;
	    }
	    else if(strncmp(linebuff, "image_dimension.dim[", 20) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 20, "%d", &index) != 1 || index > 7) {
		cerr<<"error reading image_dimension.dim index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.dim["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->dime.dim[index] = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.vox_units") == 0) {
	      strncpy(rawhdr->dime.vox_units, cptr, 4);
	    }
	    else if(strcmp(linebuff, "image_dimension.cal_units") == 0) {
	      strncpy(rawhdr->dime.cal_units, cptr, 8);
	    }
	    else if(strcmp(linebuff, "image_dimension.unused1") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.unused1\n";
		exit(1);
	      }
	      rawhdr->dime.unused1 = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.datatype") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.datatype\n";
		exit(1);
	      }
	      rawhdr->dime.datatype = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.bitpix") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.bitpix\n";
		exit(1);
	      }
	      rawhdr->dime.bitpix = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.dim_un0") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.dim_un0\n";
		exit(1);
	      }
	      rawhdr->dime.dim_un0 = itmp;
	    }
	    else if(strncmp(linebuff, "image_dimension.pixdim[", 23) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 23, "%d", &index) != 1 || index > 7) {
		cerr<<"error reading image_dimension.pixdim index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.pixdim["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->dime.pixdim[index] = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.vox_offset") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.vox_offset\n";
		exit(1);
	      }
	      rawhdr->dime.vox_offset = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.funused1") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.funused1\n";
		exit(1);
	      }
	      rawhdr->dime.funused1 = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.funused2") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.funused2\n";
		exit(1);
	      }
	      rawhdr->dime.funused2 = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.funused3") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.funused3\n";
		exit(1);
	      }
	      rawhdr->dime.funused3 = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.cal_max") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.cal_max\n";
		exit(1);
	      }
	      rawhdr->dime.cal_max = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.cal_min") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading image_dimension.cal_min\n";
		exit(1);
	      }
	      rawhdr->dime.cal_min = ftmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.compressed") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.compressed\n";
		exit(1);
	      }
	      rawhdr->dime.compressed = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.verified") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.verified\n";
		exit(1);
	      }
	      rawhdr->dime.verified = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.glmax") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.glmax\n";
		exit(1);
	      }
	      rawhdr->dime.glmax = itmp;
	    }
	    else if(strcmp(linebuff, "image_dimension.glmin") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading image_dimension.glmin\n";
		exit(1);
	      }
	      rawhdr->dime.glmin = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.descrip") == 0) {
	      strncpy(rawhdr->hist.descrip, cptr, 80);
	    }
	    else if(strcmp(linebuff, "data_history.aux_file") == 0) {
	      strncpy(rawhdr->hist.aux_file, cptr, 24);
	    }
	    else if(strcmp(linebuff, "data_history.orient") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.orient\n";
		exit(1);
	      }
	      rawhdr->hist.orient = (char) itmp;
	    }
	    else if(strcmp(linebuff, "data_history.originator") == 0) {
	      strncpy(rawhdr->hist.originator, cptr, 10);
	    }
	    else if(strncmp(linebuff, "spm_origin[", 11) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 11, "%d", &index) != 1 || (index > 4)) {
		cerr<<"error reading spm_origin index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading spm_origin["<<index<<"]\n";
		exit(1);
	      }
	      union { char c[10]; short s[5]; } c_or_s;
	      int j;
	      for(j = 0; j<10; j++) c_or_s.c[j] = rawhdr->hist.originator[j];
	      c_or_s.s[index] = itmp;
	      for(j = 0; j<10; j++) rawhdr->hist.originator[j] = c_or_s.c[j];
	    }
	    else if(strcmp(linebuff, "data_history.generated") == 0) {
	      strncpy(rawhdr->hist.generated, cptr, 10);
	    }
	    else if(strcmp(linebuff, "data_history.scannum") == 0) {
	      strncpy(rawhdr->hist.scannum, cptr, 10);
	    }
	    else if(strcmp(linebuff, "data_history.patient_id") == 0) {
	      strncpy(rawhdr->hist.patient_id, cptr, 10);
	    }
	    else if(strcmp(linebuff, "data_history.exp_date") == 0) {
	      strncpy(rawhdr->hist.exp_date, cptr, 10);
	    }
	    else if(strcmp(linebuff, "data_history.exp_time") == 0) {
	      strncpy(rawhdr->hist.exp_time, cptr, 10);
	    }
	    else if(strcmp(linebuff, "data_history.hist_un0") == 0) {
	      strncpy(rawhdr->hist.hist_un0, cptr, 3);
	    }
	    else if(strcmp(linebuff, "data_history.views") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.views\n";
		exit(1);
	      }
	      rawhdr->hist.views = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.vols_added") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.vols_added\n";
		exit(1);
	      }
	      rawhdr->hist.vols_added = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.start_field") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.start_field\n";
		exit(1);
	      }
	      rawhdr->hist.start_field = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.field_skip") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.field_skip\n";
		exit(1);
	      }
	      rawhdr->hist.field_skip = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.omax") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.omax\n";
		exit(1);
	      }
	      rawhdr->hist.omax = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.omin") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.omin\n";
		exit(1);
	      }
	      rawhdr->hist.omin = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.smax") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.smax\n";
		exit(1);
	      }
	      rawhdr->hist.smax = itmp;
	    }
	    else if(strcmp(linebuff, "data_history.smin") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading data_history.smin\n";
		exit(1);
	      }
	      rawhdr->hist.smin = itmp;
	    }
	    else {
	      cerr<<" unknown line=" << linebuff << "=" << cptr << "\n";
	    }
	  } // end if changed
	}
      }
    } // end if(in->good())
  } // end while
  if(! changed) cout <<" no changes made\n";
  else cout <<" changes where made\n";
  return rawhdr;
}
