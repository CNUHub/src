#include <unistd.h>
#include <sys/wait.h>
/*#include <strstream.h>*/
#include <sstream>
#include "psyhdr.h"
#include "psyecat.h"
// could not find these in include files
extern "C" char *mktemp(char *name);
// local functions defined below
int put_ecat_header_line(char *key_name, char *line_string,
			 char *header, data_word header_template[]);
void applyheaderedits(char *main_hdr, data_word main_header_template[],
		      char *frame_hdr, data_word frame_header_template[],
		      istream *in, char *oldvalues=NULL);
int hdrValueChanged(char *name, char *value, char *oldvalues);

int main(int argc, char *argv[])
{
  char *infile=NULL;
  char *outfile=NULL;
  char *editor=NULL;


// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile outfile [-ed editor | - ]"<<'\n';
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-ed", argv[i]) == 0) && ((i+1) < argc))
      editor = argv[++i];
    else if(infile == NULL) infile=argv[i];
    else if(outfile == NULL) outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }

  // open the ecat file reading its main header
  ecatfile inputecatfile(infile, 1, 0, 0);
  int filetype =  inputecatfile.get_ecatfiletype();
  int ecatdatatype = (int) inputecatfile.get_header_value("data_type", 1);
  psytype datatype = ecattype2psytype(ecatdatatype);
  char main_hdr[MatBLKSIZE];
  inputecatfile.get_mainheader(main_hdr);
  psypgbuff inbuffered((psyimg *) &inputecatfile, 1);
  data_word *main_header_template = ecat_main_header;
  char frame_hdr[MatBLKSIZE];
  inputecatfile.get_frameheader(frame_hdr);
  data_word *frame_header_template;
  switch(filetype) {
  default:
  case IMAGE_FILE:
    frame_header_template = ecat_image_subhdr;
    break;
  case SCAN_FILE:
    frame_header_template = ecat_scan_subhdr;
    break;
  case NORM_FILE:
    frame_header_template = ecat_norm_subhdr;
    break;
  case ATTN_FILE:
    frame_header_template = ecat_attn_subhdr;
    break;
  }

// determine the editor
  if(editor == NULL) {
    editor = getenv("EDITOR");
    if(editor == NULL) editor = "vi";
  }

// save the old values as a string for edit comparison
/*
  ostrstream *ost = new ostrstream();
  inputecatfile.showhdr(ost);
  ost->put('\0');
  ost->freeze(1);
  char *oldvalues = ost->str();
  int cnt = ost->pcount();
  delete ost;
*/
  std::stringstream ss;
  inputecatfile.showhdr(&ss);
  std::string str = ss.str();
  int cnt = str.size();
  char *oldvalues = new char[cnt + 1];
  strcpy(oldvalues, str.c_str());

// apply edits
  if(strcmp("-", editor) == 0) {
// edits from standard in
    applyheaderedits(main_hdr, main_header_template,
		     frame_hdr, frame_header_template,
		     &cin, oldvalues);
  }
  else {
// edits via temporary file and user defined editor
// create a temporary file containing the old string
    char *tmpfile = new char[30 + 1];
    strcpy(tmpfile, "/tmp/ecatheadereditXXXXXX");
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
  // edits from stream
    applyheaderedits(main_hdr, main_header_template,
		     frame_hdr, frame_header_template,
		     &intmpfs, oldvalues);
    intmpfs.close(); unlink(tmpfile); // remove the temporary file
  }

  // write the image with the new header
  ecatfile outecatfile(outfile, (psyimg *) &inputecatfile, datatype,
		       filetype, main_hdr, frame_hdr);
// log
  logfile log(outfile, argc, argv);
  log.loginfilelog(infile);
  //  ecatfile outecatfile(outfile, (psyimg *) &inbuffered, datatype,
  //	       filetype, main_hdr, frame_hdr);
}

int hdrValueChanged(char *name, char *value, char *oldvalues) {
  if(oldvalues == NULL) return 1;
  if(value == NULL) return 1;
  char *cptr = strstr(oldvalues, name);
  if(cptr != NULL) {
    cptr += strlen(name) + 1;
    char *cr_ptr = strchr(cptr, '\n');
    if(cr_ptr == NULL) return 1;
    unsigned int length = cr_ptr - cptr;
    if(strlen(value) != length) return 1;
    return strncmp(value, cptr, length);
  }
  return 1;
}

void applyheaderedits(char *main_hdr, data_word main_header_template[],
		      char *frame_hdr, data_word frame_header_template[],
		      istream *in, char *oldvalues) {
  char linebuff[1024], c;
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
	    if(put_ecat_header_line(linebuff, cptr,
				    main_hdr, main_header_template) > -1)
	      ; // found key word in main header
	    else if(put_ecat_header_line(linebuff, cptr,
				    frame_hdr, frame_header_template) > -1)
	      ; // found key word in sub header
	    else {
	      // key word not found
	      cerr<<" unknown line=" << linebuff << "=" << cptr << "\n";
	    }
	  } // end if changed
	}
      }
    } // end if(in->good())
  } // end while
  if(! changed) cout <<" no changes made\n";
  else cout <<" changes where made\n";
}

int put_ecat_header_line(char *key_name, char *line_string,
			 char *header, data_word header_template[]) {

  data_word *word_ptr = header_template;
  int i= -1;
  for(; word_ptr->byte >= 0; word_ptr++) {
    if(strcmp(key_name, word_ptr->name) == 0) {
      header += word_ptr->byte;
      char *tokenptr;
      short *sptr;
      int *iptr;
      float *fptr;
      switch(word_ptr->type) {
      case psychar:
	for(i=0; i<word_ptr->words; i++) header[i] = 0;
	if(line_string[0] == '(') {
	  line_string++;
	  char *last = strrchr(line_string, ')');
	  if(last != NULL) *last='\0';
	}
	strncpy(header, line_string, word_ptr->words);
        break;
      case psyshort:
	sptr = new short[word_ptr->words];
	tokenptr = strtok(line_string, "(,)");
	for(i=0; i<word_ptr->words; i++) {
	  if(tokenptr != NULL) {
	    sscanf(tokenptr, "%hd", sptr+i);
// cout<<"read short word="<< *(sptr+i) <<" i="<<i<<'\n';
	    tokenptr = strtok(NULL, "(,)");
	  }
	  else *(sptr + i) = 0;
	}
	copy_bytes((unsigned char *) sptr,
		   (unsigned char *) header,
		   word_ptr->words * sizeof(short));
	delete[] sptr;
	break;
      case psyint:
	iptr = new int[word_ptr->words];
	tokenptr = strtok(line_string, "(,)");
	for(i=0; i<word_ptr->words; i++) {
	  if(tokenptr != NULL) {
	    sscanf(tokenptr, "%d", iptr + i);
// cout<<"read int word="<< *(iptr + i) <<" i="<<i<<'\n';
	    tokenptr = strtok(NULL, "(,)");
	  }
	  else *(iptr + i) = 0;
	}
	copy_bytes((unsigned char *) iptr,
		   (unsigned char *) header,
		   word_ptr->words * sizeof(int));
	delete[] iptr;
	break;
      case psyfloat:
	fptr = new float[word_ptr->words];
	tokenptr = strtok(line_string, "(,)");
	for(i=0; i<word_ptr->words; i++) {
	  if(tokenptr != NULL) {
	    sscanf(tokenptr, "%f", fptr + i);
// cout<<"read float word="<< *(fptr + i) <<" i="<<i<<'\n';
	    tokenptr = strtok(NULL, "(,)");
// cout<<"next token tokenptr="<<tokenptr<<'\n';
	  }
	  else *(fptr + i) = 0;
	}
	copy_bytes((unsigned char *) fptr,
		   (unsigned char *) header,
		   word_ptr->words * sizeof(float));
	delete[] fptr;
	break;
      default:
	cerr<<"put_ecat_header_string -";
	cerr<<" invalid type="<<word_ptr->type;
	cerr<<" in header_template for key_name="<<key_name<<'\n';
	exit(1);
      }
    }
  }

  return i; // -1 if no key name found or the number of key name words set
}
