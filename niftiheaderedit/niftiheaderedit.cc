#include <unistd.h>
#include <sys/wait.h>
/*#include <strstream.h>*/
#include <sstream>
#include "psyhdr.h"
#include "cnunifti.h"
// need ecat swap routines
#include "psyecat.h"
//#include <mcheck.h>

// could not find these in include files
extern "C" char *mktemp(char *name);
// local functions defined below
nifti_1_header *lreadhdr(nifti_1_header *rawhdr, istream *in, char *oldvalues);
int hdrValueChanged(char *valuename, char *value, char *oldvalues);

int main(int argc, char *argv[])
{
  //  mtrace();
  char *infile=NULL;
  char *outfile=NULL;
  char *editor=NULL;
  int textin = 0;
  int headeroutputonly = 1;
  xyzdouble factors;
  factors.x=factors.y=factors.z=1.0;
  int xfactor_set = 0;
  int yfactor_set = 0;
  int zfactor_set = 0;

  // copied from nifti.cc
  // these moved to here because I had problem on Solaris 5.5.1 of
  // arrays being all 0 causing a core dump
  static int header_soff[] = {
    36,
    40, 42, 44, 46, 48, 50, 52, 54,
    68, 70, 72, 74,
    120,
    252, 254,
    -1 };
  static int header_ioff[] = {
    0, 32,
    140, 144,
    -1 };
  static int header_foff[] = {
    56, 60, 64,
    76, 80, 84, 88, 92, 96, 100, 104,
    108, 112, 116,
    124, 128, 132, 136,
    256, 260, 264, 268, 272, 276,
    280, 284, 288, 292,
    296, 300, 304, 308,
    312, 316, 320, 324,
    -1 };

// parse input parameters
  for(int i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<3) {
      cout <<"Usage: "<<argv[0]<<" infile [-textin] outfile [-ed editor | - | none] [-xf x_factor] [-yf y_factor] [-zf z_factor]"<<'\n';
      exit(0);
    }
    else if(i==0); // ignore program name
    else if((strcmp("-ed", argv[i]) == 0) && ((i+1) < argc))
      editor = argv[++i];
    else if(strcmp("-textin", argv[i]) == 0) textin = 1;
    else if((strcmp(argv[i], "-xf")==0) && ((i+1)<argc)) {
      factors.x=atof(argv[++i]); xfactor_set = 1;
      if(editor == NULL) editor = "none"; 
    }
    else if((strcmp(argv[i], "-yf")==0) && ((i+1)<argc)) {
      factors.y=atof(argv[++i]); yfactor_set = 1;
      if(editor == NULL) editor = "none"; 
    }
    else if((strcmp(argv[i], "-zf")==0) && ((i+1)<argc)) {
      factors.z=atof(argv[++i]); zfactor_set = 1;
      if(editor == NULL) editor = "none"; 
    }
    else if(infile == NULL) infile=argv[i];
    else if(outfile == NULL) outfile=argv[i];
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }
  if(infile == NULL) {
    cerr << argv[0] << ": missing input file name\n";
    cerr <<"Usage: "<<argv[0]<<" infile [-textin] outfile [-ed editor | - | none]"<<'\n';
    exit(1);
  }
  else if(outfile == NULL) {
    cerr << argv[0] << ": missing output file name\n";
    cerr <<"Usage: "<<argv[0]<<" infile [-textin] outfile [-ed editor | - | none]"<<'\n';
    exit(1);
  }

  nifti_1_header *rawhdr = NULL;
  niftifile *imgptr = NULL;
  psyswaptype swaptype = psynoswap;
  int outnamelength=strlen(outfile);
  if(strstr(outfile, ".nii") == &outfile[outnamelength-4] ||
     strstr(outfile, ".img") == &outfile[outnamelength-4]) headeroutputonly = 0;
// read header from a text file
  if(textin) {
    if(! headeroutputonly) {
      cerr << argv[0] << ": textin option outputs header only -- output file name must not end in .nii or .img\n";
      exit(1);
    }
    fstream intmpfs(infile, ios::in);
    rawhdr = lreadhdr(NULL, &intmpfs, NULL);
    intmpfs.close();
  }
  else if(headeroutputonly) {
cout<<"header output only\n";
    // read header only
    rawhdr = readNiftiHeader(infile, NULL, NULL, &swaptype);
  }
  else {
    // open input nifti file and take header from it
    imgptr = new niftifile(infile, "r");
    rawhdr = imgptr->getniftiheader();
  }

  if(rawhdr == NULL) {
    cerr << argv[0] << ": error reading header for file: " << infile << '\n';
    exit(1);
  }

// fix resolutions by factors
  if(xfactor_set) rawhdr->pixdim[1] *= factors.x;
  if(yfactor_set) rawhdr->pixdim[2] *= factors.y;
  if(zfactor_set) rawhdr->pixdim[3] *= factors.z;

// determine the editor
  if(editor == NULL) {
    editor = getenv("EDITOR");
    if(editor == NULL) editor = "vi";
  }
  cout << "editor=" << editor << "\n";
// none writes the header as read
  if(! (strcmp("none", editor) == 0)) {
// save the old values as a string for edit comparison
    std::stringstream ss(std::stringstream::out);
    std::string str;
    niftifile::showhdr(rawhdr, &ss);
    str = ss.str();
    int cnt = str.size();
    char oldvalues[cnt + 1];
    strcpy(oldvalues, str.c_str());

    if(strcmp("-", editor) == 0) {
      // edits from standard in
      rawhdr = lreadhdr(rawhdr, &cin, oldvalues);
    }
    else {
      char tmpfile[40];
      strcpy(tmpfile, "/tmp/niftiheadereditXXXXXX");
// create a temporary file containing the old string
      mktemp(tmpfile);
      fstream outtmpfs;
      outtmpfs.open(tmpfile, ios::out);
      outtmpfs << oldvalues; 
      outtmpfs.close();
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
  if(imgptr == NULL) {
// keep the original swapping
    if(swaptype == psyreversewordbytes)
      swap_data((char *) rawhdr, header_soff, header_ioff, header_foff);
// write the new header
    writeNiftiHeader(rawhdr, outfile, NULL);
  }
  else {
    // write complete file
    niftifile outniftifile(outfile, imgptr, psynotype, rawhdr);
  }
  //  muntrace();
  return 0;
}

int hdrValueChanged(char *valuename, char *value, char *oldvalues) {
  if(oldvalues == NULL) return 1;
  if(value == NULL) return 1;
  char *cptr = strstr(oldvalues, valuename);
  if(cptr != NULL) {
    cptr += strlen(valuename) + 1;
    char *cr_ptr = strchr(cptr, '\n');
    if(cr_ptr == NULL) return 1;
    unsigned int length = cr_ptr - cptr;
    if(strlen(value) != length) return 1;
    return strncmp(value, cptr, length);
  }
  return 1;
}

void strncpynoquotes(char *outptr, char *inptr, int cnt) {
  if(cnt > 0) {
    if( *inptr == '"' ) inptr++; // remove quote at first character
    strncpy(outptr, inptr, cnt);
    outptr[cnt-1]= '\0';
    char *ptr = strrchr(outptr, '"');
    if(ptr != NULL) *ptr = '\0'; // remove trailing quote
  }
}


nifti_1_header *lreadhdr(nifti_1_header *rawhdr, istream *in, char *oldvalues=NULL) {
  char linebuff[1024], c;
  if(rawhdr == NULL) {
    rawhdr = new nifti_1_header();
    // initialize analyze header to zeros
    char *ptr, *endptr;
    for(ptr=(char*) rawhdr, endptr=ptr+sizeof(nifti_1_header);
	ptr<endptr; ptr++) *ptr= '\0';
    // set size
    rawhdr->sizeof_hdr = 348;
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
	    if(strcmp(linebuff, "sizeof_hdr") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading header_key.sizeof_hdr\n";
		exit(1);
	      }
	      rawhdr->sizeof_hdr = itmp;
	    }
	    else if(strcmp(linebuff, "data_type") == 0) {
	      strncpynoquotes(rawhdr->data_type, cptr, 10);
	    }
	    else if(strcmp(linebuff, "db_name") == 0) {
	      strncpynoquotes(rawhdr->db_name, cptr, 18);
	    }
	    else if(strcmp(linebuff, "extents") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading extents\n";
		exit(1);
	      }
	      rawhdr->extents = itmp;
	    }
	    else if(strcmp(linebuff, "session_error") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading session_error\n";
		exit(1);
	      }
	      rawhdr->session_error = itmp;
	    }
	    else if(strcmp(linebuff, "regular") == 0) {
	      rawhdr->regular = *cptr;
	    }
	    else if(strcmp(linebuff, "dim_info") == 0) {
// need to process freq_dim, phase_dim and slice_dim
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading dim_info\n";
		exit(1);
	      }
	      rawhdr->dim_info = (char) itmp;
	    }
	    else if(strncmp(linebuff, "dim[", 4) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 4, "%d", &index) != 1 || index > 7) {
		cerr<<"error reading dim index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading dim["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->dim[index] = itmp;
	    }
	    else if(strcmp(linebuff, "intent_p1") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading intent_p1\n";
		exit(1);
	      }
	      rawhdr->intent_p1 = itmp;
	    }
	    else if(strcmp(linebuff, "intent_p2") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading intent_p2\n";
		exit(1);
	      }
	      rawhdr->intent_p2 = itmp;
	    }
	    else if(strcmp(linebuff, "intent_p3") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading intent_p3\n";
		exit(1);
	      }
	      rawhdr->intent_p3 = itmp;
	    }
	    else if(strcmp(linebuff, "intent_code") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading intent_code\n";
		exit(1);
	      }
	      rawhdr->intent_code = itmp;
	    }
	    else if(strcmp(linebuff, "datatype") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading datatype\n";
		exit(1);
	      }
	      rawhdr->datatype = itmp;
	    }
	    else if(strcmp(linebuff, "bitpix") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading bitpix\n";
		exit(1);
	      }
	      rawhdr->bitpix = itmp;
	    }
	    else if(strcmp(linebuff, "slice_start") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading slice_start\n";
		exit(1);
	      }
	      rawhdr->slice_start = itmp;
	    }
	    else if(strncmp(linebuff, "pixdim[", 7) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 7, "%d", &index) != 1 || index > 7) {
		cerr<<"error reading pixdim index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading pixdim["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->pixdim[index] = ftmp;
	    }
	    else if(strcmp(linebuff, "vox_offset") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading vox_offset\n";
		exit(1);
	      }
	      rawhdr->vox_offset = ftmp;
	    }
	    else if(strcmp(linebuff, "scl_slope") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading scl_slope\n";
		exit(1);
	      }
	      rawhdr->scl_slope = ftmp;
	    }
	    else if(strcmp(linebuff, "scl_inter") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading scl_inter\n";
		exit(1);
	      }
	      rawhdr->scl_inter = ftmp;
	    }
	    else if(strcmp(linebuff, "slice_end") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading slice_end\n";
		exit(1);
	      }
	      rawhdr->slice_end = itmp;
	    }
	    else if(strcmp(linebuff, "slice_code") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading slice_code\n";
		exit(1);
	      }
	      rawhdr->slice_code = itmp;
	    }
	    else if(strcmp(linebuff, "xyzt_units") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading xyzt_units\n";
		exit(1);
	      }
	      rawhdr->xyzt_units = itmp;
	    }
	    else if(strcmp(linebuff, "cal_max") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading cal_max\n";
		exit(1);
	      }
	      rawhdr->cal_max = ftmp;
	    }
	    else if(strcmp(linebuff, "cal_min") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading cal_min\n";
		exit(1);
	      }
	      rawhdr->cal_min = ftmp;
	    }
	    else if(strcmp(linebuff, "slice_duration") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading slice_duration\n";
		exit(1);
	      }
	      rawhdr->slice_duration = ftmp;
	    }
	    else if(strcmp(linebuff, "toffset") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading toffset\n";
		exit(1);
	      }
	      rawhdr->toffset = ftmp;
	    }
	    else if(strcmp(linebuff, "glmax") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading glmax\n";
		exit(1);
	      }
	      rawhdr->glmax = itmp;
	    }
	    else if(strcmp(linebuff, "glmin") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading glmin\n";
		exit(1);
	      }
	      rawhdr->glmin = itmp;
	    }
	    else if(strcmp(linebuff, "descrip") == 0) {
	      strncpynoquotes(rawhdr->descrip, cptr, 80);
	    }
	    else if(strcmp(linebuff, "aux_file") == 0) {
	      strncpynoquotes(rawhdr->aux_file, cptr, 24);
	    }
	    else if(strcmp(linebuff, "qform_code") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading qform_code\n";
		exit(1);
	      }
	      rawhdr->qform_code = itmp;
	    }
	    else if(strcmp(linebuff, "sform_code") == 0) {
	      if(sscanf(cptr, "%d", &itmp) != 1) {
		cerr<<"error reading sform_code\n";
		exit(1);
	      }
	      rawhdr->sform_code = itmp;
	    }
	    else if(strcmp(linebuff, "quatern_c") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading quatern_c\n";
		exit(1);
	      }
	      rawhdr->quatern_c = ftmp;
	    }
	    else if(strcmp(linebuff, "quatern_d") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading quatern_d\n";
		exit(1);
	      }
	      rawhdr->quatern_d = ftmp;
	    }
	    else if(strcmp(linebuff, "qoffset_x") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading qoffset_x\n";
		exit(1);
	      }
	      rawhdr->qoffset_x = ftmp;
	    }
	    else if(strcmp(linebuff, "qoffset_y") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading qoffset_y\n";
		exit(1);
	      }
	      rawhdr->qoffset_y = ftmp;
	    }
	    else if(strcmp(linebuff, "qoffset_z") == 0) {
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading qoffset_z\n";
		exit(1);
	      }
	      rawhdr->qoffset_z = ftmp;
	    }
	    else if(strncmp(linebuff, "srow_x[", 7) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 7, "%d", &index) != 1 || index > 3 || index < 0) {
		cerr<<"error reading srow_x index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading srow_x["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->srow_x[index] = ftmp;
	    }
	    else if(strncmp(linebuff, "srow_y[", 7) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 7, "%d", &index) != 1 || index > 3 || index < 0) {
		cerr<<"error reading srow_y index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading srow_y["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->srow_y[index] = ftmp;
	    }
	    else if(strncmp(linebuff, "srow_z[", 7) == 0) {
	      int index = 0;
	      if(sscanf(linebuff + 7, "%d", &index) != 1 || index > 3 || index < 0) {
		cerr<<"error reading srow_z index \n";
		exit(1);
	      }
	      if(sscanf(cptr, "%f", &ftmp) != 1) {
		cerr<<"error reading srow_z["<<index<<"]\n";
		exit(1);
	      }
	      rawhdr->srow_z[index] = ftmp;
	    }
	    else if(strcmp(linebuff, "intent_name") == 0) {
	      strncpynoquotes(rawhdr->intent_name, cptr, 16);
	    }
	    else if(strcmp(linebuff, "magic") == 0) {
	      strncpynoquotes(rawhdr->magic, cptr, 4);
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
