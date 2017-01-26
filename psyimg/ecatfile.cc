#include "psyhdr.h"
#include "psyecat.h"
/*#include <strstream.h>*/
#include <time.h>
#include <typeinfo>
#include <sstream>

data_word ecat_main_header[]= {
  {0,psyshort,"fill0",14},
  {28,psychar,"original_file_name",20},
  {48,psyshort,"sw_version",1},
  {50,psyshort,"data_type",1},
  {52,psyshort,"system_type",1},
  {54,psyshort,"file_type",1},
  {56,psychar,"node_id",10},
  {66,psyshort,"scan_start_day",1},
  {68,psyshort,"scan_start_month",1},
  {70,psyshort,"scan_start_year",1},
  {72,psyshort,"scan_start_hour",1},
  {74,psyshort,"scan_start_minute",1},
  {76,psyshort,"scan_start_second",1},
  {78,psychar,"isotope_code",8},
  {86,psyfloat,"isotope_halflife",1},
  {90,psychar,"radiopharmaceutical",32},
  {122,psyfloat,"gantry_bed_stuff",3},
  {134,psyshort,"mcs_stuff",3},
  {140,psyfloat,"axial_fov",1},
  {144,psyfloat,"transaxial_fov",1},
  {148,psyshort,"sampling_stuff",3},
  {154,psyfloat,"calibration_factor",1},
  {158,psyshort,"calibration_units",1},
  {160,psyshort,"compression_code",1},
  {162,psychar,"study_name",12},
  {174,psychar,"patient_id",16},
  {190,psychar,"patient_name",32},
  {222,psychar,"patient_sex",1},
  {223,psychar,"patient_age",10},
  {233,psychar,"patient_height",10},
  {243,psychar,"patient_weight",10},
  {253,psychar,"patient_dexterity",1},
  {254,psychar,"physician_name",32},
  {286,psychar,"operator_name",32},
  {318,psychar,"study_description",32},
  {350,psyshort,"acquisition_type",1},
  {352,psyshort,"bed_type",1},
  {354,psyshort,"septa_type",1},
  {356,psychar,"facility_name",20},
  {376,psyshort,"num_planes",1},
  {378,psyshort,"num_frames",1},
  {380,psyshort,"num_gates",1},
  {382,psyshort,"num_bed_pos",1},
  {384,psyfloat,"init_bed_position",1},
  {388,psyfloat,"bed_offset",15},
  {448,psyfloat,"plane_separation",1},
  {452,psyshort,"lwr_sctr_thres",1},
  {454,psyshort,"lwr_true_thres",1},
  {456,psyshort,"upr_true_thres",1},
  {458,psyfloat,"collimator",1},
  {462,psychar,"user_process_code",10},
  {472,psyshort,"acquisition_mode",1},
  {474,psyshort,"fill1",19},
  {-1,psychar,"",0}
};

/*
	; gantry_bed_stuff consists of:
	;	gantry_tilt, gantry_rotation, bed_elevation
	; mcs_stuff consists of:
	;	rot_source_speed, wobble_speed, transm_source_type
	; sampling_stuff consists of:
	;	transaxial_samp_mode, coin_samp_mode, axial_samp_mode
*/

data_word ecat_image_subhdr[]= {
  {0,psyshort,"fill0",63},
  {126,psyshort,"data_type",1},
  {128,psyshort,"num_dimensions",1},
  {130,psyshort,"fill1",1},
  {132,psyshort,"dimension_1",1},
  {134,psyshort,"dimension_2",1},
  {136,psyshort,"fill2",12},
  {160,psyfloat,"x_origin",1},
  {164,psyfloat,"y_origin",1},
  {168,psyfloat,"recon_scale",1},
  {172,psyfloat,"quant_scale",1},
  {176,psyshort,"image_min",1},
  {178,psyshort,"image_max",1},
  {180,psyshort,"fill3",2},
  {184,psyfloat,"pixel_size",1},
  {188,psyfloat,"slice_width",1},
  {192,psyint,"frame_duration",1},
  {196,psyint,"frame_start_time",1},
  {200,psyshort,"slice_location",1},
  {202,psyshort,"recon_start_hour",1},
  {204,psyshort,"recon_start_minute",1},
  {206,psyshort,"recon_start_sec",1},
  {208,psyint,"recon_duration",1},
  {212,psyshort,"fill4",12},
  {236,psyshort,"filter_code",1},
  {238,psyint,"scan_matrix_num",1},
  {242,psyint,"norm_matrix_num",1},
  {246,psyint,"atten_cor_matrix_num",1},
  {250,psyshort,"fill5",23},
  {296,psyfloat,"image_rotation",1},
  {300,psyfloat,"plane_eff_corr_fctr",1},
  {304,psyfloat,"decay_corr_fctr",1},
  {308,psyfloat,"loss_corr_fctr",1},
  {312,psyfloat,"intrinsic_tilt",1},
  {316,psyshort,"fill6",30},
  {376,psyshort,"processing_code",1},
  {378,psyshort,"fill7",1},
  {380,psyshort,"quant_units",1},
  {382,psyshort,"recon_start_day",1},
  {384,psyshort,"recon_start_month",1},
  {386,psyshort,"recon_start_year",1},
  {388,psyfloat,"ecat_calibration_fctr",1},
  {392,psyfloat,"well_counter_cal_fctr",1},
  {396,psyfloat,"filter_params",6},
  {420,psychar,"annotation",40},
  {460,psychar,"fill8",26},
  {-1,psychar,"",0}
};

data_word ecat_scan_subhdr[]= {
  {0,psyshort,"fill0",63},
  {126,psyshort,"data_type",1},
  {128,psyshort,"fill1",2},
  {132,psyshort,"dimension_1",1},
  {134,psyshort,"dimension_2",1},
  {136,psyshort,"smoothing",1},
  {138,psyshort,"processing_code",1},
  {140,psyshort,"fill2",3},
  {146,psyfloat,"sample_distance",1},
  {150,psyshort,"fill3",8},
  {166,psyfloat,"isotope_halflife",1},
  {170,psyshort,"frame_duration_sec",1},
  {172,psyint,"gate_duration",1},
  {176,psyint,"r_wave_offset",1},
  {180,psyshort,"fill4",1},
  {182,psyfloat,"scale_factor",1},
  {186,psyshort,"fill5",3},
  {192,psyshort,"scan_min",1},
  {194,psyshort,"scan_max",1},
  {196,psyint,"prompts",1},
  {200,psyint,"delayed",1},
  {204,psyint,"multiples",1},
  {208,psyint,"net_trues",1},
  {212,psyshort,"fill6",52},
  {316,psyfloat,"cor_singles",16},
  {380,psyfloat,"uncor_singles",16},
  {444,psyfloat,"tot_avg_cor",1},
  {448,psyfloat,"tot_avg_uncor",1},
  {452,psyint,"total_coin_rate",1},
  {456,psyint,"frame_start_time",1},
  {460,psyint,"frame_duration",1},
  {464,psyfloat,"loss_correction_fctr",1},
  {468,psyint,"phy_planes",8},
  {500,psyshort,"fill7",6},
  {-1,psychar,"",0}
};

data_word ecat_norm_subhdr[]= {
  {0,psyshort,"fill0",63},
  {126,psyshort,"data_type",1},
  {128,psyshort,"fill1",2},
  {132,psyshort,"dimension_1",1},
  {134,psyshort,"dimension_2",1},
  {136,psyshort,"fill2",23},
  {182,psyfloat,"scale_factor",1},
  {186,psyshort,"norm_hour",1},
  {188,psyshort,"norm_minute",1},
  {190,psyshort,"norm_second",1},
  {192,psyshort,"norm_day",1},
  {194,psyshort,"norm_month",1},
  {196,psyshort,"norm_year",1},
  {198,psyfloat,"fov_source_width",1},
  {202,psyfloat,"ecat_calib_factor",1},
  {206,psyshort,"fill3",153},
  {-1,psychar,"",0}
};

data_word ecat_attn_subhdr[]= {
  {0,psyshort,"fill0",63},
  {126,psyshort,"data_type",1},
  {128,psyshort,"attenuation_type",1},
  {130,psyshort,"fill1",1},
  {132,psyshort,"dimension_1",1},
  {134,psyshort,"dimension_2",1},
  {136,psyshort,"fill2",23},
  {182,psyfloat,"scale_factor",1},
  {186,psyfloat,"x_origin",1},
  {190,psyfloat,"y_origin",1},
  {194,psyfloat,"x_radius",1},
  {198,psyfloat,"y_radius",1},
  {202,psyfloat,"tilt_angle",1},
  {206,psyfloat,"attenuation_coeff",1},
  {210,psyfloat,"sample_distance",1},
  {214,psyshort,"fill3",149},
  {-1,psychar,"",0}
};

psytype getecatclosesttype(psytype type) {
  switch (type) {
  case psyuchar:
    return psyuchar;
  case psychar:
  case psyshortsw:
  case psyshort:
    return psyshort;
  case psyushort:
  case psyuint:
  case psyint:
    return psyint;
  case psydouble:
  case psyfloat:
    return psyfloat;
  case psycomplex:
  case psydate:
  case psystring:
  case psydicomdataelement:
  case psyrgb:
  case psyargb:
  case psynotype:
  default:
    return psynotype;
  }
}

psytype ecattype2psytype(int ecattype)
{
  switch(ecattype) {
  case GENERIC:
  case BYTE_TYPE:
    return(psyuchar);
  case VAX_I2:
  case SUN_I2:
    return(psyshort);
  case VAX_I4:
  case SUN_I4:
    return(psyint);
  case VAX_R4:
  case SUN_R4:
    return(psyfloat);
  default:
    cerr<<"ecattype2psytype - unknown ecat type = "<<ecattype<<'\n';
    exit(1);
  }
}

int psytype2ecattype(psytype intype)
{
  switch(intype) {
  case psyuchar:
    return(BYTE_TYPE);
  case psyshort:
    return(VAX_I2);
  case psyint:
    return(VAX_I4);
  case psyfloat:
    return(VAX_R4);
  default:
    cerr<<"psytype2ecattype - equivalent type unknown = "<<intype<<'\n';
    exit(1);
  }
}

void put_ecat_header_string(string str, string key_name, char *hdr,
			    const data_word header_template[])
{
  char *ptr;
  for(int i=0; header_template[i].byte >= 0; i++) {
    if(key_name.compare(header_template[i].name) == 0) {
      if(header_template[i].type != psychar) {
	cerr<<"put_ecat_header_string -";
	cerr<<" invalid type="<<header_template[i].type;
	cerr<<" in header_template for key_name="<<key_name<<'\n';
	exit(1);
      }
      ptr=hdr + header_template[i].byte;
//clear old string
      for(int j=0;j<header_template[i].words;j++)ptr[j]='\0';
//store new string
      strncpy(ptr, str.c_str(), header_template[i].words);
      return;
    }
  }
  cerr<<"put_ecat_header_string - key name="<<key_name<<" not found\n";
  exit(1);
}

string get_ecat_header_string(string key_name, const char *hdr,
			      const data_word header_template[])
{
  for(int i=0; header_template[i].byte >= 0; i++) {
    if(key_name.compare(header_template[i].name) == 0) {
      if(header_template[i].type != psychar) {
	cerr<<"get_ecat_header_string - invalid type=\n";
	exit(1);
      }
      string s1(hdr + header_template[i].byte, header_template[i].words);
      return s1;
    }
  }
  cerr<<"get_ecat_header_string - key name="<<key_name<<" not found\n";
  exit(1);
}

void put_ecat_header_value(float value, string key_name, char *hdr,
			   const data_word header_template[])
{
  unsigned char *uptr;
  short svalue;
  int ivalue;
// first locate key name in template
  for(int i=0; header_template[i].byte >= 0; i++) {
    if(key_name.compare(header_template[i].name) == 0) {
      uptr=(unsigned char *)hdr + header_template[i].byte;
      switch(header_template[i].type) {
      case psychar:
	*uptr = (char)value;
	return;
      case psyshort:
	svalue=(short)value;
	copy_bytes((unsigned char *)&svalue, uptr, sizeof(svalue));
	return;
      case psyint:
        ivalue=(int)value;
	copy_bytes((unsigned char *)&ivalue, uptr, sizeof(ivalue));
	return;
      case psyfloat:
	copy_bytes((unsigned char *)&value, uptr, sizeof(value));
	return;
      default:
	cerr<<"put_ecat_header_value -";
	cerr<<" invalid type="<<header_template[i].type;
	cerr<<" in header_template for key_name="<<key_name<<'\n';
	exit(1);
      }
    }
  }
  cerr<<"put_ecat_header_value - key name="<<key_name<<" not found\n";
  exit(1);
}

float get_ecat_header_value(string key_name, const char *hdr,
			    const data_word header_template[])
{
  const char *uptr;
// first locate key name in template
  for(int i=0; header_template[i].byte >= 0; i++) {
    if(key_name.compare(header_template[i].name) == 0) {
      uptr=hdr + header_template[i].byte;
      switch(header_template[i].type) {
      case psychar:
	return((float)*uptr);
      case psyshort:
	return((float)get_short((unsigned char *)uptr));
      case psyint:
      case psydate:
	return((float)get_int((unsigned char *)uptr));
      case psyfloat:
	return(get_float((unsigned char *)uptr));
      default:
	cerr<<"get_ecat_header_value - unknown type in header_template\n";
	exit(1);
      }
    }
  }
  cerr<<"get_ecat_header_value - key name="<<key_name<<" not found\n";
  exit(1);
}

string get_ecat_patientid(const char *hdr, const data_word header_template[])
{
  string pid=get_ecat_header_string("patient_id", hdr, header_template);
// remove spaces and dashes
  for(size_t i=0; i<pid.length();) {
    if(pid[i] != ' ' && pid[i] != '-') pid.erase(i,1);
    else i++;
  }
  return(pid);
}

string get_ecat_date(char *hdr, data_word header_template[])
{
  int day, month, year;
  day=(int)get_ecat_header_value("scan_start_day", hdr,
				 header_template);
  month=(int)get_ecat_header_value("scan_start_month", hdr,
				    header_template);
  year=(int)get_ecat_header_value("scan_start_year", hdr,
				   header_template);
  while(year > 100) year -= 100;

  stringstream ss(std::stringstream::out);
  ss<<month<<'/'<<day<<'/'<<year<<'\0';

  return ss.str();
}

string get_ecat_time(const char *hdr, const data_word header_template[])
{
  int hour, minute, second;
  hour=(int)get_ecat_header_value("scan_start_hour", hdr,
				  header_template);
  minute=(int)get_ecat_header_value("scan_start_minute", hdr,
				    header_template);
  second=(int)get_ecat_header_value("scan_start_second", hdr,
				    header_template);
  stringstream ss(std::stringstream::out);
  ss<<hour<<':'<<minute<<':'<<second<<'\0';
  return(ss.str());
}

void show_ecat_header(char *hdr, data_word header_template[], ostream *out)
{
  int i, j;
  char *uptr;

  for(i=0; header_template[i].byte >= 0; i++) {
    uptr = hdr + header_template[i].byte;
    *out<<header_template[i].name<<"=(";
    switch(header_template[i].type) {
    case psychar:
      for(j=0; j<header_template[i].words && *uptr!='\0'; j++)
	*out<<*uptr++;
      break;
    case psyshort:
      for(j=0; j<header_template[i].words; j++, uptr += sizeof(short)) {
	if(j!=0) *out << ',';
	*out<<get_short((unsigned char *)uptr);
      }
      break;
    case psyint:
      for(j=0; j<header_template[i].words; j++, uptr += sizeof(int)) {
	if(j!=0) *out << ',';
	*out<<get_int((unsigned char *)uptr);
      }
      break;
    case psyfloat:
      for(j=0; j<header_template[i].words; j++, uptr += sizeof(float)) {
	if(j!=0) *out << ',';
	*out<<get_float((unsigned char *)uptr);
      }
      break;
    case psydate:
      int t_int;
      time_t time;
      char ptr[32];
      for(j=0; j<header_template[i].words; j++, uptr += sizeof(int)) {
	if(j!=0)*out<<',';
	t_int=get_int((unsigned char *)uptr);
	time=(time_t) t_int;
	strftime(ptr, 32, "%a %b %d %T %Y", localtime(&time));
	*out<<t_int<<"="<<ptr;
      }
      break;
    default:
      cerr<<"show_ecat_header: unknown type in header_template\n";
      exit(1);
    }
    *out<<")\n";
  }
}

string bldecatfilename(string name)
{
  int namelength=name.length();
  if(namelength == 0) return name;
  else if(namelength > 5) {
    string ending=name.substr(namelength-4,4);
    if((ending.compare(".img") == 0) ||
       (ending.compare(".atn") == 0) ||
       (ending.compare(".scn") == 0) ||
       (ending.compare(".nrm") == 0))
      return name;
  }
  return name + ".img";
}

int isecatfile(string name, FILE **fp, char *main_hdr)
{
  FILE *local_fp;
  char *local_main_hdr;
  if(main_hdr == NULL)local_main_hdr=new char[MatBLKSIZE];
  else local_main_hdr=main_hdr;
// open file with standard c constructs
  string fullname=bldecatfilename(name);
  local_fp=fopen(fullname.c_str(), "r");
  if(local_fp != NULL) {
// read header
    int err=read_header(local_fp, 1, (unsigned char *)local_main_hdr,
			main_soff, main_loff, main_foff);
    if(!err){
// check header
      int file_type = (int) get_ecat_header_value("file_type", local_main_hdr,
						  ecat_main_header);
      if(file_type == IMAGE_FILE || file_type == SCAN_FILE ||
	 file_type == ATTN_FILE || file_type == NORM_FILE)
      {
// clean up and return true
	if(fp != NULL)*fp=local_fp;
	else fclose(local_fp);
	if(main_hdr == NULL)delete[] local_main_hdr;
	return(file_type);
      }
    } // end if(!err)
  } // end if(local_fp != NULL)
// clean up and return false
  if(local_fp != NULL)fclose(local_fp);
  if(main_hdr == NULL)delete[] local_main_hdr;
  return(0);
}

ecatfile::ecatfile(string fname, int firstframe, int lastframe, int quantify)
{
// initialize like ecatfile() here also although
// not a problem like write constructor
  fp=NULL;mlist=NULL;main_hdr_set=0;frame_hdr_frame=0;
  file_type=IMAGE_FILE;ecat_frame_offset=ecat_plane_offset=1;
  subhdr=ecat_image_subhdr;ecat_plane_list=NULL;

  initecatfile(fname, firstframe, lastframe, quantify);
}

ecatfile::ecatfile(string fname, psyimg *psyimgptr,
		   psytype outtype, int ecatfiletype,
		   char *in_main_hdr, char *in_frame_hdr)
{
// need to initialize like ecatfile()
// because mlist doesn't seem to initialized and
// caused problems in ~ecatfile on some machines
  fp=NULL;mlist=NULL;main_hdr_set=0;frame_hdr_frame=0;
  file_type=IMAGE_FILE;ecat_frame_offset=ecat_plane_offset=1;
  subhdr=ecat_image_subhdr;ecat_plane_list=NULL;

  initpsyimglnk(psyimgptr, outtype);
  file_type = ecatfiletype;
  switch(file_type) {
  default:
  case IMAGE_FILE:
    subhdr = ecat_image_subhdr;
    break;
  case SCAN_FILE:
    subhdr = ecat_scan_subhdr;
    break;
  case NORM_FILE:
    subhdr = ecat_norm_subhdr;
    break;
  case ATTN_FILE:
    subhdr = ecat_attn_subhdr;
    break;
  }
  ecat_frame_offset=1-orig.i;
  ecat_plane_offset=1-orig.z;
  outtype=gettype();
  if(fname.empty()) {
    output_tree(&cerr);
    cerr<<":ecatfile::ecatfile - empty file name\n";
    exit(1);
  }
  string fullname=bldecatfilename(fname);
// open file with standard c constructs

  if((fp=fopen(fullname.c_str(), "w")) == NULL) {
    output_tree(&cerr);
    cerr<<":ecatfile::ecatfile - error opening file: "<<fullname<<'\n';
    exit(1);
  }

// initialize
  int i;
  if(in_main_hdr != NULL) {
    for(i=0; i<MatBLKSIZE; i++) main_hdr[i] = in_main_hdr[i];
  }
  else {
    for(i=0; i<MatBLKSIZE; i++) main_hdr[i] = 0;

// fill in as much of the main header as known
    put_ecat_header_value((float)file_type, "file_type", main_hdr,
			  ecat_main_header);
    put_ecat_header_value((float)psytype2ecattype(outtype),
			  "data_type", main_hdr, ecat_main_header);
    int day, month, year;
    getdate(&month, &day, &year);
    put_ecat_header_value((float)day, "scan_start_day", main_hdr,
			  ecat_main_header);
    put_ecat_header_value((float)month, "scan_start_month", main_hdr,
			  ecat_main_header);
    put_ecat_header_value((float)year, "scan_start_year", main_hdr,
			  ecat_main_header);
    int hour, minute, second;
    gettime(&hour, &minute, &second);
    put_ecat_header_value((float)hour, "scan_start_hour",main_hdr,
			  ecat_main_header);
    put_ecat_header_value((float)minute, "scan_start_minute",main_hdr,
			  ecat_main_header);
    put_ecat_header_value((float)second, "scan_start_second",main_hdr,
			  ecat_main_header);
    put_ecat_header_string(getpatientid().c_str(), "patient_id",main_hdr, ecat_main_header);
    put_ecat_header_string(getdescription().c_str(), "study_description",main_hdr, ecat_main_header);
    put_ecat_header_string("notset", "original_file_name",main_hdr,
			   ecat_main_header);
    put_ecat_header_string("va_mpls", "facility_name",main_hdr,ecat_main_header);
    put_ecat_header_string("pardo", "physician_name",main_hdr, ecat_main_header);
    put_ecat_header_string("jtlee", "operator_name",main_hdr, ecat_main_header);
    put_ecat_header_string("none", "radiopharmaceutical",main_hdr,
			   ecat_main_header);
    psydims size=getsize();
    put_ecat_header_value((float)size.z, "num_planes", main_hdr,
			  ecat_main_header);
    if((int)get_ecat_header_value("num_planes", main_hdr, ecat_main_header) != size.z) {
      output_tree(&cerr);
      cerr<<":ecatfile - error storing z(plane) size="<<size.z<<" in ecat main header\n";
      exit(1);
    }
    put_ecat_header_value((float)size.i, "num_frames", main_hdr,
			  ecat_main_header);
    if((int)get_ecat_header_value("num_frames", main_hdr, ecat_main_header) != size.i) {
      output_tree(&cerr);
      cerr<<":ecatfile - error storing i(frame) size="<<size.i<<" in ecat main header\n";
      exit(1);
    }
    psyres res=getres();
    put_ecat_header_value((float)res.z*100,"plane_separation", main_hdr,
			  ecat_main_header);
  }

  main_hdr_set=1;
// write out main header
  write_header(fp, 1, (unsigned char *)main_hdr, main_soff, main_loff,
	       main_foff);

// initialize common sub header stuff
  if(in_frame_hdr != NULL) {
    for(i=0; i<MatBLKSIZE; i++) frame_hdr[i] = in_frame_hdr[i];
  }
  else {
    for(i=0; i<MatBLKSIZE; i++) frame_hdr[i] = 0;
    put_ecat_header_value((float)psytype2ecattype(outtype), "data_type",
			  frame_hdr, subhdr);
    put_ecat_header_value((float)size.x, "dimension_1", frame_hdr,
			  subhdr);
    if((int)get_ecat_header_value("dimension_1", frame_hdr, subhdr) != size.x) {
      output_tree(&cerr);
      cerr<<":ecatfile - error storing x size="<<size.x<<" in ecat subheader\n";
      exit(1);
    }
    put_ecat_header_value((float)size.y, "dimension_2", frame_hdr,
			  subhdr);
    if((int)get_ecat_header_value("dimension_2", frame_hdr, subhdr) != size.y) {
      output_tree(&cerr);
      cerr<<":ecatfile - error storing y size="<<size.y<<" in ecat subheader\n";
      exit(1);
    }
// initialize sub header stuff that varies with file type
    double recon_scale=1.0;
    switch(file_type) {
    default:
    case IMAGE_FILE:
      put_ecat_header_value((float)2, "num_dimensions", frame_hdr,
			    subhdr);
      put_ecat_header_value((float)0, "x_origin", frame_hdr, subhdr);
      put_ecat_header_value((float)0, "y_origin", frame_hdr, subhdr);
      // set reconstruction scale factor according to our PET camera
      // with 128x128 50cm normal scan size
      if(res.x > 1e-16) recon_scale=0.50/(128*res.x);
      put_ecat_header_value((float)recon_scale, "recon_scale", frame_hdr,
			    subhdr);
      put_ecat_header_value((float)res.x * 100, "pixel_size", frame_hdr,
			    subhdr);
      // changed slice_width back to 1 not based on res.z 9/26/96 jtl
      put_ecat_header_value((float)1, "slice_width", frame_hdr,
			    subhdr);
      break;
    case SCAN_FILE:
      put_ecat_header_value((float)res.x * 100, "sample_distance", frame_hdr,
			    subhdr);
      break;
    case NORM_FILE:
      break;
    case ATTN_FILE:
      put_ecat_header_value((float)1, "attenuation_type", frame_hdr,
			    subhdr);
      put_ecat_header_value((float)0, "x_origin", frame_hdr, subhdr);
      put_ecat_header_value((float)0, "y_origin", frame_hdr, subhdr);
      put_ecat_header_value((float)res.x * 100, "sample_distance", frame_hdr,
			    subhdr);
      break;
    }
  }

/*
  cout<<"*psyimgptr typeid name is: " << typeid(*psyimgptr).name() << '\n';
  cout<<"this typeid name is: " << typeid(this).name() << '\n';
  if(typeid(this) == typeid(*psyimgptr))
    cout<<"type ids equivalent\n";
  else cout<<"type ids not equivalent\n";
*/

  ecatfile *inecatfileptr = dynamic_cast<ecatfile *> (psyimgptr);

  frame_hdr_frame=0;
  frame_hdr_plane=0;
  quantify=1;
  if(inecatfileptr != NULL) quantify = inecatfileptr->get_quantify();

// create and write matrix directory
  mlist = new MatDir[MAXMAT];
  nmat=0;
  psydims linc=getinc();
  int gate=1, data=0, bed=0;

  nmat=writeMatrixDirectory(mlist, MAXMAT, size.i, size.z, inc.z,
			    gate, data, bed, fp);

// classes needed to scale planes individually
  psyimgblk blocked;
  scaleimg scaled(psyimgptr, outtype);
// initialize output buffer for one plane at a time
  psybuff buffimage(size.x, size.y, 1, 1, outtype,
		    orig.x, orig.y, orig.z, orig.i);
  psydims buffinc=buffimage.getinc();
// loop thru planes
  double scale_factor;
  double localwordres;
  double min, max, mean, sum, sqrsum;
  min=max=mean=sum=sqrsum=0;
  double tmpmin, tmpmax, tmpmean, tmpsum, tmpsqrsum;
  int firsttime=1;
  for(i=orig.i; i<=end.i; i++) {
    for(int z=orig.z; z<=end.z; z++) {

      if(quantify) {
// calculate and set scale factor for this plan
	blocked.init(psyimgptr, orig.x, orig.y, z, i, end.x, end.y, z, i);
	blocked.getstats(&tmpmin, &tmpmax, NULL);
	scale_factor=scale_factor_for_max_res(tmpmin, tmpmax, outtype);
	scaled.set_scale_factor(scale_factor);
	localwordres=scaled.getwordres();
// transfer one plane of data to output buffer
	scaled.copyblock(buffimage.getbuff(), orig.x, orig.y, z, i,
			 end.x, end.y, z, i,
			 buffinc.x, buffinc.y, buffinc.z, buffinc.i,
			 outtype);
      }
      else {
	inecatfileptr->copyblock(buffimage.getbuff(), orig.x, orig.y, z, i,
				 end.x, end.y, z, i,
				 buffinc.x, buffinc.y, buffinc.z, buffinc.i,
				 outtype);
	localwordres = inecatfileptr->get_frame_quantification(i, z);
      }

// keep running stats
      buffimage.unsetstats();
      buffimage.getstats(&tmpmin, &tmpmax, &tmpmean, &tmpsum, &tmpsqrsum);
      if(localwordres < 0) {
	double tmp = tmpmax;
	tmpmax = tmpmin * localwordres;
	tmpmin = tmp * localwordres;
      }
      else {
	tmpmin *= localwordres; tmpmax *= localwordres;
      }
      tmpmean *= localwordres; tmpsum *= localwordres;
      tmpsqrsum *= localwordres*localwordres;
      if(firsttime){
	firsttime=0;
	min=tmpmin; max=tmpmax; mean=tmpmean;
	sum=tmpsum; sqrsum=tmpsqrsum;
      }
      else {
	min = (tmpmin < min)? tmpmin : min;
	max = (tmpmax > max)? tmpmax : max;
	mean += tmpmean;
	sum += tmpsum;
	sqrsum += tmpsqrsum;
      }

// write header and plane to the ecat file
      frame_hdr_frame=i+ecat_frame_offset;
      frame_hdr_plane=z+ecat_plane_offset;
      switch(file_type) {
      default:
      case IMAGE_FILE:
	// set differently for each plane file type
	put_ecat_header_value((float)localwordres,"quant_scale", frame_hdr,
			      subhdr);
	put_ecat_header_value((float)tmpmin/localwordres, "image_min",
			      frame_hdr, subhdr);
	put_ecat_header_value((float)tmpmax/localwordres, "image_max",
			      frame_hdr, subhdr);
	write_plane(fp, mlist, nmat, i+ecat_frame_offset, z+ecat_plane_offset,
		    buffimage.getbuff(), buffinc.z,
		    frame_hdr, image_soff, image_loff, image_foff);
	break;
      case SCAN_FILE:
	// set differently for each plane file type
	put_ecat_header_value((float)localwordres,"scale_factor", frame_hdr,
			      subhdr);
	put_ecat_header_value((float)tmpmin/localwordres, "scan_min",
			      frame_hdr, subhdr);
	put_ecat_header_value((float)tmpmax/localwordres, "scan_max",
			      frame_hdr, subhdr);
	put_ecat_header_value(1.0, "loss_correction_fctr",
			      frame_hdr, subhdr);
	write_plane(fp, mlist, nmat, i+ecat_frame_offset, z+ecat_plane_offset,
		    buffimage.getbuff(), buffinc.z,
		    frame_hdr, scan_soff, scan_loff, scan_foff);
	break;
      case NORM_FILE:
	// set differently for each plane file type
	put_ecat_header_value((float)localwordres,"scale_factor", frame_hdr,
			      subhdr);
	write_plane(fp, mlist, nmat, i+ecat_frame_offset, z+ecat_plane_offset,
		    buffimage.getbuff(), buffinc.z,
		    frame_hdr, norm_soff, norm_loff, norm_foff);
	break;
      case ATTN_FILE:
	// set differently for each plane file type
	put_ecat_header_value((float)localwordres,"scale_factor", frame_hdr,
			      subhdr);
	write_plane(fp, mlist, nmat, i+ecat_frame_offset, z+ecat_plane_offset,
		    buffimage.getbuff(), buffinc.z,
		    frame_hdr, attn_soff, attn_loff, attn_foff);
	break;
      }
    } // end for(z=
  } // end for(i=
// set stats of output image
  int count=size.z * size.i;
  if(count != 0)mean /= count;
  setstats(min, max, mean, sum, sqrsum);
}

void ecatfile::initecatfile(string fname, int firstframe, int lastframe, int quantify)
{
  //  char *fullname;
  //  int namelength;
  int xdim, ydim, zdim, idim, xorig, yorig, zorig, iorig;
  double xres, yres, zres; //, ires, wres;
  psytype pixeltype;

  if(fname.empty()) {
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - empty file name\n";
    exit(1);
  }
// frame and plane offsets to allow ecat file to start at 1 and image at 0
  ecat_frame_offset=ecat_plane_offset=1;
// isecatfile opens ecat file for reading, reads header and checks if valid
  file_type = isecatfile(fname, &fp, main_hdr);
  if(! file_type) {
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - error invalid ECAT file: "<<fname<<'\n';
    exit(1);
  }
// set sub header data word array according to file type
  switch (file_type) {
  default:
  case IMAGE_FILE:
    subhdr = ecat_image_subhdr;
    break;
  case SCAN_FILE:
    subhdr = ecat_scan_subhdr;
    break;
  case NORM_FILE:
    subhdr = ecat_norm_subhdr;
    break;
  case ATTN_FILE:
    subhdr = ecat_attn_subhdr;
    break;
  }
// isecatfile read main_hdr for us
  main_hdr_set=1;
// build the matrix directory
  mlist = new MatDir[MAXMAT];
  nmat = buildMatrixDirectory(fp, mlist, MAXMAT);
  if(nmat < 1) {
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - error - matrix directory size=" << nmat;
    cerr<<'\n';
    exit(1);
  }

// firstframe < 1 causes problems and value of 0 should default to 1
  if(firstframe < 1) firstframe=1;
// derive z and i(plane and frame) information from main header
  iorig = firstframe - ecat_frame_offset;
  idim = (int) get_header_value("num_frames");
  // num_frames may not have been set
  if(idim < 1) idim = maxFrameNumber(mlist, nmat);
// reset i dimensions based on first and last frames
  if(lastframe == 0) lastframe = idim;
  if(firstframe > lastframe || lastframe > idim) {
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - error - requested frames=(";
    cerr<<firstframe<<","<<lastframe<<") with max frame number="<<idim;
    cerr<<'\n';
    exit(1);
  }
  idim= lastframe-firstframe+1;
  int maxPlane = maxPlaneNumber(mlist, nmat, 0);
  if(maxPlane < 1) {
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - error - no planes found\n";
    exit(1);
  }
  zorig=0;
  zdim= (int) get_header_value("num_planes");
  if(zdim < 1) {
    // num_planes may not have been set
    zdim = maxPlane; // best guess until we build a list of planes
  }
// build an ordered list of planes for the first frame
  ecat_plane_list = new int[zdim];
  int blk0, nblk;
  int z = -1; // no planes found yet
  for( int ecatPlane = 1; (ecatPlane <= maxPlane) && (z < zdim); ecatPlane++) {
    if(findBlocks(mlist, nmat, firstframe, ecatPlane, &blk0, &nblk) == 0) {
      z++;
      ecat_plane_list[z] = ecatPlane;
    }
  }
  if(z < 0) {
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - error - no planes found";
    cerr<<'\n';
    exit(1);
  }
  if(z != zdim-1) {
    zdim = z+1;
    output_tree(&cerr);
    cerr<<":ecatfile::initecatfile - warning - missing planes";
    cerr<<" - plane numbers will not correspond\n";
    cerr<<" dropping max plane from "<<maxPlane<<" to "<<zdim; 
    cerr<<'\n';
  }
// zres should be set from plane_separation not slice_width 9/26/96 jtl
  zres=(double)get_header_value("plane_separation")/100.0; //cm to meters
// no frame header read yet
  frame_hdr_frame=0;
  frame_hdr_plane=0;
// initialize x and y image dimensions to values found in frame sub header
  xdim=(int)get_header_value("dimension_1", firstframe);
  ydim=(int)get_header_value("dimension_2", firstframe);
  xorig=yorig=0;
// set resolutions based on file type
  switch(file_type) {
  default:
  case IMAGE_FILE:
    xres=yres=(double)get_header_value("pixel_size", firstframe
				       )/100.0;//cm to meters
    break;
  case NORM_FILE:
    xres=yres=1;
    break;
  case SCAN_FILE:
  case ATTN_FILE:
    xres=yres=(double)get_header_value("sample_distance", firstframe
				       )/100.0;//cm to meters
    break;
  }

//  zres=(double)get_header_value("slice_width", firstframe
//				     )/100.0; //cm to meters
// set quantify default ecat type
  ecatfile::quantify=quantify;
  if(quantify) pixeltype=psyfloat;
  else pixeltype = ecattype2psytype((int) get_header_value("data_type", firstframe));
// initialize psyimglnk information
  initpsyimglnk(NULL, xdim, ydim, zdim, idim, pixeltype,
		xorig, yorig, zorig, iorig,
		0, xres, yres, zres);
  setpatientid(get_ecat_patientid(main_hdr,
				  ecat_main_header));
  setdate(get_ecat_date(main_hdr, ecat_main_header));
  settime(get_ecat_time(main_hdr, ecat_main_header));
  setdescription(get_ecat_header_string("study_description",
					main_hdr,
					ecat_main_header));
}

ecatfile::~ecatfile()
{
  if(fp != NULL) fclose(fp);
  if(ecat_plane_list != NULL) delete[] ecat_plane_list;
  if(mlist != NULL) delete[] mlist;
}

char *ecatfile::get_mainheader(char *out_main_hdr) {
  if(main_hdr_set) {
    if(out_main_hdr == NULL) out_main_hdr = new char[MatBLKSIZE];
    for(int i=0; i<MatBLKSIZE; i++) out_main_hdr[i] = main_hdr[i];
    return out_main_hdr;
  }
  else return NULL;
}

char *ecatfile::get_frameheader(char *out_frame_hdr) {
  if(main_hdr_set) {
    if(out_frame_hdr == NULL) out_frame_hdr = new char[MatBLKSIZE];
    for(int i=0; i<MatBLKSIZE; i++) out_frame_hdr[i] = frame_hdr[i];
    return out_frame_hdr;
  }
  else return NULL;
}

void ecatfile::copyblock(char *outbuff, int xorig, int yorig, int zorig,
		 int iorig, int xend, int yend, int zend, int iend,
		 int xinc, int yinc, int zinc, int iinc,
		 psytype pixeltype)
{
  double quantscale;
  if(!inside(xorig, yorig, zorig, iorig) || !inside(xend, yend, zend, iend)) {
    output_tree(&cerr);
    cerr<<":ecatfile::copyblock - region outside of image\n";
    exit(1);
  }
  if(xorig != orig.x || xend != end.x || yorig != orig.y ||
     yend != end.y || zorig != zend || iorig != iend) {
    output_tree(&cerr);
    cerr<<":ecatfile::copyblock - invalid extents requested\n";
    cerr<<"                      copies only full planes(use psypgbuff)\n";
    exit(1);
  }
  if(iorig == iend) {
    iinc = inc.i; // ignore iinc
    if(zorig == zend) zinc = inc.z; // ignore zinc
  }
  if(xinc != inc.x || yinc != inc.y || zinc != inc.z || iinc != inc.i) {
    output_tree(&cerr);
    cerr<<":ecatfile::copyblock - invalid increment requested\n";
    cerr<<"                      reads only continuous regions\n";
    exit(1);
  }
  if(pixeltype != type) {
    output_tree(&cerr);
    cerr<<":ecatfile::copyblock - invalid pixel type\n";
    exit(1);
  }
  read_plane(fp, file_type, mlist, nmat, iorig+ecat_frame_offset,
	     ecat_plane_list[zorig], quantify, outbuff, zinc, &quantscale);
//	     zorig+ecat_plane_offset, quantify, outbuff, zinc, &quantscale);
}

void ecatfile::getpixel(char *pixel, psytype pixeltype,
			int x, int y, int z, int i)
{
  output_tree(&cerr);
  cerr<<":ecatfile::getpixel - sorry not implimented\n";
  exit(1);
}

void ecatfile::showhdr(ostream *out)
{
  showmainhdr(out);
  showsubhdr(out);
}

void ecatfile::showmainhdr(ostream *out)
{
  *out<<"main header\n";
  show_ecat_header(main_hdr, ecat_main_header, out);
}

void ecatfile::showsubhdr(ostream *out)
{
  *out<<"frame "<<frame_hdr_frame<<"  plane "<<frame_hdr_plane<<" subheader\n";
  show_ecat_header(frame_hdr, subhdr, out);
}

float ecatfile::get_header_value(string key_name, int frame, int plane)
{
  int blk0, nblk;
// if no frame then return main header value
  if(frame == 0) {
    if(!main_hdr_set){
      read_header(fp, 1, (unsigned char *)main_hdr,
		  main_soff, main_loff, main_foff);
      main_hdr_set=1;
    }
    return(get_ecat_header_value(key_name, main_hdr, ecat_main_header));
  }
  if(frame != frame_hdr_frame || plane != frame_hdr_plane) {
// get frame header
// first locate it
    if (findBlocks(mlist, nmat, frame, plane, &blk0,&nblk)) {
      output_tree(&cerr);
      cerr<<":ecatfile::get_header_value - error locating frame "<<frame;
      cerr<<", plane "<<plane<<'\n';
      exit(1);
    }
// then read it
    switch(file_type) {
    default:
    case IMAGE_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  image_soff, image_loff, image_foff);
      break;
    case SCAN_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  scan_soff, scan_loff, scan_foff);
      break;
    case NORM_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  norm_soff, norm_loff, norm_foff);
      break;
    case ATTN_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  attn_soff, attn_loff, attn_foff);
      break;
    }
    frame_hdr_frame=frame;
    frame_hdr_plane=plane;
  }
  return(get_ecat_header_value(key_name, frame_hdr, subhdr));
}

float ecatfile::get_frame_quantification(int i, int z) {
  switch(file_type) {
  default:
  case IMAGE_FILE:
    return get_header_value("quant_scale", i+ecat_frame_offset, ecat_plane_list[z]);
  case SCAN_FILE:
  case NORM_FILE:
  case ATTN_FILE:
    return get_header_value("scale_factor", i+ecat_frame_offset, ecat_plane_list[z]);
  }
}

string ecatfile::get_header_string(string key_name, int frame, int plane)
{
  int blk0, nblk;
// if no frame then return main header value
  if(frame == 0) {
    if(!main_hdr_set){
      read_header(fp, 1, (unsigned char *)main_hdr,
		  main_soff, main_loff, main_foff);
      main_hdr_set=1;
    }
    return(get_ecat_header_string(key_name, main_hdr, ecat_main_header));
  }
  if(frame != frame_hdr_frame || plane != frame_hdr_plane) {
// get frame header
// first locate it
    if (findBlocks(mlist, nmat, frame, plane, &blk0,&nblk)) {
      output_tree(&cerr);
      cerr<<":ecatfile::get_header_string - error locating frame "<<1;
      cerr<<", plane "<<1<<'\n';
      exit(1);
    }
// then read it
    switch(file_type) {
    default:
    case IMAGE_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  image_soff, image_loff, image_foff);
      break;
    case SCAN_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  scan_soff, scan_loff, scan_foff);
      break;
    case NORM_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  norm_soff, norm_loff, norm_foff);
      break;
    case ATTN_FILE:
      read_header(fp, blk0, (unsigned char *)frame_hdr,
		  attn_soff, attn_loff, attn_foff);
      break;
    }
    frame_hdr_frame=frame;
    frame_hdr_plane=plane;
  }
  return(get_ecat_header_string(key_name, frame_hdr, subhdr));
}

/*
	; patient_stuff consists of:
	;	patient_id(16),patient_name(32),patient_sex(1)
	;	patient_age(10),patient_height(10),patient_weight(10)
	;	patient_dexterity(1) = 80 chars total
	; gantry_bed_stuff consists of:
	;	gantry_tilt, gantry_rotation, bed_elevation
	; mcs_stuff consists of:
	;	rot_source_speed, wobble_speed, transm_source_type
	; sampling_stuff consists of:
	;	transaxial_samp_mode, coin_samp_mode, axial_samp_mode
*/
