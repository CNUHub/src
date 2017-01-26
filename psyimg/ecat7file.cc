#include "psyhdr.h"
#include "psyecat7.h"
/*#include <strstream.h>*/
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>


const char* psyecat7names[] = {
  "UNKNOWN_FILE_TYPE",
  "ECAT7_SINOGRAM",
  "ECAT7_IMAGE_16",
  "ECAT7_ATTENUATION_CORRECTION",
  "ECAT7_NORMALIZATION",
  "ECAT7_POLAR_MAP",
  "ECAT7_VOLUME_8",
  "ECAT7_VOLUME_16",
  "ECAT7_PROJECTION_8",
  "ECAT7_PROJECTION_16",
  "ECAT7_IMAGE_8",
  "ECAT7_3D_SINOGRAM_16",
  "ECAT7_3D_SINOGRAM_8",
  "ECAT7_3D_SINOGRAM_NORMALIZATION",
  "ECAT7_3D_SINOGRAM_FIT"
};

/* ecat7 header structures */

int ecat7_main_soff[] = { 46,48,50,126,128,138,140,142,148,150,152,
  328,330,352,354,356,358,428,430,432,444,498,500,502,504,506,508,510,
  -1 };
int ecat7_main_loff[] = { 62,228,454, -1 };
int ecat7_main_foff[] = { 74,110,114,118,122,130,134,144,216,220,224,
  360, 364,368,372,376,380,384,388,392,396,400,404,408,412,416,420,
  424,446,450,458,462, -1 };

data_word ecat7_main_header[]= {
  {0,psychar,"magic_number",14},
  {14,psychar,"original_file_name",32},
  {46,psyshort,"sw_version",1},
  {48,psyshort,"system_type",1},
  {50,psyshort,"file_type",1},
  {52,psychar,"serial_number",10},
  {62,psyint,"scan_start_time",1},
  {66,psychar,"isotype_name",8},
  {74,psyfloat,"isotope_halflife",1},
  {78,psychar,"radiopharmaceutical",32},
  {110,psyfloat,"gantry_tilt",1},
  {114,psyfloat,"gantry_rotation",1},
  {118,psyfloat,"bed_elevation",1},
  {122,psyfloat,"intrinsic_tilt",1},
  {126,psyshort,"wobble_speed",1},
  {128,psyshort,"transm_source_type",1},
  {130,psyfloat,"distance_scanned",1},
  {134,psyfloat,"transaxial_fov",1},
  {138,psyshort,"angular_compression",1},
  {140,psyshort,"coin_samp_mode",1},
  {142,psyshort,"axial_samp_mode",1},
  {144,psyfloat,"ecat_calibration_factor",1},
  {148,psyshort,"calibration_units",1},
  {150,psyshort,"calibration_units_label",1},
  {152,psyshort,"compression_code",1},
  {154,psychar,"study_type",12},
  {166,psychar,"patient_id",16},
  {182,psychar,"patient_name",32},
  {214,psychar,"patient_sex",1},
  {215,psychar,"patient_dexterity",1},
  {216,psyfloat,"patient_age",1},
  {220,psyfloat,"patient_height",1},
  {224,psyfloat,"patient_weight",1},
  {228,psydate,"patient_birth_date",1},
  {232,psychar,"physician_name",32},
  {264,psychar,"operator_name",32},
  {296,psychar,"study_description",32},
  {328,psyshort,"acquisition_type",1},
  {330,psyshort,"patient_orientation",1},
  {332,psychar,"facility_name",20},
  {352,psyshort,"num_planes",1},
  {354,psyshort,"num_frames",1},
  {356,psyshort,"num_gates",1},
  {358,psyshort,"num_bed_pos",1},
  {360,psyfloat,"init_bed_position",1},
  {364,psyfloat,"bed_position",15},
  {424,psyfloat,"plane_separation",1},
  {428,psyshort,"lwr_sctr_thres",1},
  {430,psyshort,"lwr_true_thres",1},
  {432,psyshort,"upr_true_thres",1},
  {434,psychar,"user_process_code",10},
  {444,psyshort,"acquisition_mode",1},
  {446,psyfloat,"bin_size",1},
  {450,psyfloat,"branching_fraction",1},
  {454,psyint,"dose_start_time",1},
  {458,psyfloat,"dosage",1},
  {462,psyfloat,"well_counter_corr_factor",1},
  {466,psychar,"date_units",32},
  {498,psyshort,"septa_state",1},
  {500,psyshort,"fill",6},
  {-1,psychar,"",0}
};

int ecat7_attenuation_soff[] = { 0,2,4,6,8,10,12,70,108,110,
      112,114,116,118,120,122,124,126,128,130,132,134,136,138,
  140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,
  170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,
  200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,
  230,232,234,236,238,240,242,244,246,248,250,252,254,256,258,
  160,262,264,266,268,270,272,274,276,278,280,282,284,286,288,
  290,292,294,296,298,300,302,304,306,308,310,312,314,316,318,
  320,322,324,326,328,330,332,334,336,338,340,342,344,346,348,
  350,352,354,356,358,360,362,364,366,368,370,372,374,376,378,
  380,382,384,386,388,390,392,394,396,398,400,402,404,406,408,
  410,
      412,414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510,
  -1 };
int ecat7_attenuation_loff[] = { -1 };
int ecat7_attenuation_foff[] = { 14,18,22,26,30,34,38,42,46,50,
  54,58,62,66, 72,76,80,84,88,92,96,100, 104,
  -1 };
data_word ecat7_attenuation_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"num_dimensions",1},
  {4,psyshort,"attenuation_type",1},
  {6,psyshort,"num_r_elements",1},
  {8,psyshort,"num_angles",1},
  {10,psyshort,"num_z_elements",1},
  {12,psyshort,"ring_difference",1},
  {14,psyfloat,"x_resolution",1},
  {18,psyfloat,"y_resolution",1},
  {22,psyfloat,"z_resolution",1},
  {26,psyfloat,"w_resolution",1},
  {30,psyfloat,"scale_factor",1},
  {34,psyfloat,"x_offset",1},
  {38,psyfloat,"y_offset",1},
  {42,psyfloat,"x-radius",1},
  {46,psyfloat,"y_radius",1},
  {50,psyfloat,"tilt_angle",1},
  {54,psyfloat,"attenuation_coeff",1},
  {58,psyfloat,"attenuation_min",1},
  {62,psyfloat,"attenuation_max",1},
  {66,psyfloat,"skull_thickness",1},
  {70,psyshort,"num_additional_atten_coeff",1},
  {72,psyfloat,"additional_atten_coeff",8},
  {104,psyfloat,"edge_finding_threshold",1},
  {108,psyshort,"storage_order",1},
  {110,psyshort,"span",1},
  {112,psyshort,"z_elements",64},
  {240,psyshort,"fill1",86},
  {412,psyshort,"fill2",50},
  {-1,psychar,"",0}
};

int ecat7_image_soff[] = { 0,2,4,6,8,30,32,54,112,206,208,218,220,
  234,236,238,
                      240,242,244,246,248,250,252,254,256,258,
  160,262,264,266,268,270,272,274,276,278,280,282,284,286,288,
  290,292,294,296,298,300,302,304,306,308,310,312,314,316,318,
  320,322,324,326,328,330,332,334,336,338,340,342,344,346,348,
  350,352,354,356,358,360,362,364,366,368,370,372,374,376,378,
  380,382,384,386,388,390,392,394,396,398,400,402,404,406,408,
  410,412,
          414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510,
  -1 };
int ecat7_image_loff[] = { 46,50,84,88,92,96,
  -1 };
int ecat7_image_foff[] = { 10,14,18,22,26,34,38,42,56,60,64,68,72,76,80,
  100,104,108,114,118,162,166,170,174,178,182,186,190,194,198,202,210,214,222,
  226,230,
  -1 };
data_word ecat7_image_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"num_dimensions",1},
  {4,psyshort,"x_dimension",1},
  {6,psyshort,"y_dimension",1},
  {8,psyshort,"z_dimension",1},
  {10,psyfloat,"x_offset",1},
  {14,psyfloat,"y_offset",1},
  {18,psyfloat,"z_offset",1},
  {22,psyfloat,"recon_zoom",1},
  {26,psyfloat,"scale_factor",1},
  {30,psyshort,"image_min",1},
  {32,psyshort,"image_max",1},
  {34,psyfloat,"x_pixel_size",1},
  {38,psyfloat,"y_pixel_size",1},
  {42,psyfloat,"z_pixel_size",1},
  {46,psyint,"frame_duration",1},
  {50,psyint,"frame_start_time",1},
  {54,psyshort,"filter_code",1},
  {56,psyfloat,"x_resolution",1},
  {60,psyfloat,"y_resolution",1},
  {64,psyfloat,"z_resolution",1},
  {68,psyfloat,"num_r_elements",1},
  {72,psyfloat,"num_angles",1},
  {76,psyfloat,"z_rotation_angle",1},
  {80,psyfloat,"decay_corr_fctr",1},
  {84,psyint,"processing_code",1},
  {88,psyint,"gate_duration",1},
  {92,psyint,"r_wave_offset",1},
  {96,psyint,"num_accepted_beats",1},
  {100,psyfloat,"filter_cutoff_freouency",1},
  {104,psyfloat,"filter_resolution",1},
  {108,psyfloat,"filter_ramp_slope",1},
  {112,psyshort,"filter_order",1},
  {114,psyfloat,"filter_scatter_fraction",1},
  {118,psyfloat,"filter_scatter_slope",1},
  {122,psychar,"annotation",40},
  {162,psyfloat,"mt_1_1",1},
  {166,psyfloat,"mt_1_2",1},
  {170,psyfloat,"mt_1_3",1},
  {174,psyfloat,"mt_2_1",1},
  {178,psyfloat,"mt_2_2",1},
  {182,psyfloat,"mt_2_3",1},
  {186,psyfloat,"mt_3_1",1},
  {190,psyfloat,"mt_3_2",1},
  {194,psyfloat,"mt_3_3",1},
  {198,psyfloat,"rfilter_cutoff",1},
  {202,psyfloat,"rfilter_resolution",1},
  {206,psyshort,"rfilter_code",1},
  {208,psyshort,"rfilter_order",1},
  {210,psyfloat,"zfilter_cutoff",1},
  {214,psyfloat,"zfilter_resolution",1},
  {218,psyshort,"zfilter_code",1},
  {220,psyshort,"zfilter_order",1},
  {222,psyfloat,"mt_1_4",1},
  {226,psyfloat,"mt_2_4",1},
  {230,psyfloat,"mt_3_4",1},
  {234,psyshort,"scatter_type",1},
  {236,psyshort,"recon_type",1},
  {238,psyshort,"recon_views",1},
  {240,psyshort,"fill1",87},
  {414,psyshort,"fill2",49},
  {-1,psychar,"",0}
};
int ecat7_polar_map_soff[] = { 0,2,4,
  6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,
  50,52,54,56,58,60,62,64,66,68,
  198,
  200,202,204,206,208,210,212,214,216,218,
  220,222,224,226,228,230,232,234,236,238,
  240,242,244,246,248,250,252,254,256,258,
  260,
  262, 264,266,268, 270,272,274, 276,278,280,298,300,
  404,406,408,
  410,412,414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,
                                      458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510
  -1 };
int ecat7_polar_map_loff[] = { 290,294,342,346,350,
  -1 };
int ecat7_polar_map_foff[] = {
  70,74,78,82,86,90,94,98,102,106,110,114,118,122,126,130,134,138,
  142,146,150,154,158,162,166,170,174,178,182,186,190,194,
  282,286,
  -1 };
data_word ecat7_polar_map_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"polar_map_type",1},
  {4,psyshort,"num_rings",1},
  {6,psyshort,"sectors_per_ring",32},
  {70,psyfloat,"ring_position",32},
  {198,psyshort,"ring_angle",32},
  {262,psyshort,"start_angle",1},
  {264,psyshort,"long_axis_left",3},
  {270,psyshort,"long_axis_right",3},
  {276,psyshort,"position_data",1},
  {278,psyshort,"image_min",1},
  {280,psyshort,"image_max",1},
  {282,psyfloat,"scale_factor",1},
  {286,psyfloat,"pixel_size",1},
  {290,psyint,"frame_duration",1},
  {294,psyint,"frame_start_time",1},
  {298,psyshort,"processing_code",1},
  {300,psyshort,"quant_units",1},
  {302,psychar,"annotation",40},
  {342,psyint,"gate_duration",1},
  {346,psyint,"r_wave_offset",1},
  {350,psyint,"num_accepted_beats",1},
  {354,psychar,"polar_map_protocol",20},
  {374,psychar,"database_name",30},
  {404,psyshort,"fill1",27},
  {458,psyshort,"fill2",27},
  {-1,psychar,"",0}
};
int ecat7_3d_scan_soff[] = { 0,2,4,6,
  10,12,14,16,18,20,22,24,26,28,
  30,32,34,36,38,40,42,44,46,48,
  50,52,54,56,58,60,62,64,66,68,
  70,72,74,76,78,80,82,84,86,88,
  90,92,94,96,98,100,102,104,106,108,
  110,112,114,116,118,120,122,124,126,128,
  130,132,134,136,
  138,140,142,160,188,190,
      232,234,236,238,240,242,244,246,248,250,252,254,256,258,
  260,262,264,266,268,270,272,274,276,278,280,282,284,286,288,
  290,292,294,296,298,300,302,304,306,308,310,312,314,316,318,
  320,322,324,326,328,330,332,334,336,338,340,342,344,346,348,
  350,352,354,356,358,360,362,364,366,368,370,372,374,376,378,
  380,382,384,386,388,390,392,394,396,398,400,402,404,406,408,
  410,
      412,414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510,
  -1 };
int ecat7_3d_scan_loff[] = { 172,176,180,192,196,200,204,216,220,224,
  -1 };
int ecat7_3d_scan_foff[] = { 144,148,152,156,184,208,212,228
  -1 };
data_word ecat7_3d_scan_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"num_dimensions",1},
  {4,psyshort,"num_r_elements",1},
  {6,psyshort,"num_angles",1},
  {8,psyshort,"corrections_applied",1},
  {10,psyshort,"first_num_z_elements",1},
  {10,psyshort,"num_z_elements",64},
  {138,psyshort,"ring_difference",1},
  {140,psyshort,"storage_order",1},
  {142,psyshort,"axial_compression",1},
  {144,psyfloat,"x_resolution",1},
  {148,psyfloat,"v_resolution",1},
  {152,psyfloat,"z_resolution",1},
  {156,psyfloat,"w_resolution",1},
  {160,psyshort,"fill1",6},
  {172,psyint,"gate_duration",1},
  {176,psyint,"r_wave_offset",1},
  {180,psyint,"num_accepted_beats",1},
  {184,psyfloat,"scale_factor",1},
  {188,psyshort,"scan_min",1},
  {190,psyshort,"scan_max",1},
  {192,psyint,"prompts",1},
  {196,psyint,"delayed",1},
  {200,psyint,"multiples",1},
  {204,psyint,"net_trues",1},
  {208,psyfloat,"tot_avg_cor",1},
  {212,psyfloat,"tot_avg_uncor",1},
  {216,psyint,"total_coin_rate",1},
  {220,psyint,"frame_start_time",1},
  {224,psyint,"frame_duration",1},
  {228,psyfloat,"deadtime_correction_factor",1},
  {232,psyshort,"fill2",90},
  {412,psyshort,"fill3",50},
// this next line must be wrong because it would be greater then 512
//  {512,psyfloat,"uncor_singles",128},
  {-1,psychar,"",0}
};
int ecat7_3d_norm_soff[] = { 0,2,4,6,8,10,12,14,16,22,312,314,
  316,318,
  320,322,324,326,328,330,332,334,336,338,340,342,344,346,348,
  350,352,354,356,358,360,362,364,366,368,370,372,374,376,378,
  380,382,384,386,388,390,392,394,396,398,400,402,404,406,408,
  410,
      412,414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510,
  -1 };
int ecat7_3d_norm_loff[] = {
  -1 };
int ecat7_3d_norm_foff[] = { 18,
       24, 28, 32, 26,
   40, 44, 48, 52, 56, 60, 64, 68, 72, 76,
   80, 84, 88, 92, 96,100,104,108,112,116,
  120,124,128,132,136,140,144,148,
  152,156,160,164,168,172,176,180,184,188,
  192,196,200,204,208,212,216,220,224,226,
  232,236,240,244,248,252,256,260,264,268,
  272,276,
  280,284,288,292,296,300,304,308,
  -1 };
data_word ecat7_3d_norm_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"num_r_elements",1},
  {4,psyshort,"num_transaxial_crystals",1},
  {6,psyshort,"num_crystal_rings",1},
  {8,psyshort,"crystals_per_ring",1},
  {10,psyshort,"num_geo_corr_planes",1},
  {12,psyshort,"uld",1},
  {14,psyshort,"lld",1},
  {16,psyshort,"scatter_energy",1},
  {18,psyfloat,"norm_quality_factor",1},
  {22,psyshort,"norm_quality_factor_code",1},
  {24,psyfloat,"ring_dtcor1",32},
  {152,psyfloat,"ring_dtcor2",32},
  {280,psyfloat,"crystal_dtcor",8},
  {312,psyshort,"span",1},
  {314,psyshort,"max_ring_diff",1},
  {316,psyshort,"fill1",48},
  {412,psyshort,"fill2",50},
  {-1,psychar,"",0}
};
int ecat7_imported_6_5_scan_soff[] = { 0,2,4,6,8,10,12,
  30,32,34,36,38,40,58,60,
  230,232,234,236,238,240,242,244,
  246,248,250,252,254,256,258,
  260,262,264,266,268,270,272,274,276,278,280,282,284,286,288,
  290,292,294,296,298,300,302,304,306,308,310,312,314,316,318,
  320,322,324,326,328,330,332,334,336,338,340,342,344,346,348,
  350,352,354,356,358,360,362,364,366,368,370,372,374,376,378,
  380,382,384,386,388,390,392,394,396,398,400,402,404,406,408,
  410,
      412,414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510,
  -1 };
int ecat7_imported_6_5_scan_loff[] = { 42,46,50,62,66,70,74,214,218,222,
  -1 };
int ecat7_3d_imported_6_5_scan_foff[] = { 14,18,22,26,54,
  78,82,86,90,94,98,102,106,110,114,118,122,126,130,134,138,
  142,146,150,154,158,162,166,170,174,178,182,186,190,194,198,202,
  206,210,226,
  -1 };
data_word ecat7_imported_6_5_scan_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"num_dimensions",1},
  {4,psyshort,"num_r_elements",1},
  {6,psyshort,"num_angles",1},
  {8,psyshort,"corrections_applied",1},
  {10,psyshort,"num_z_elements",1},
  {12,psyshort,"ring_difference",1},
  {14,psyfloat,"x_resolution",1},
  {18,psyfloat,"y_resolution",1},
  {22,psyfloat,"z_resolution",1},
  {26,psyfloat,"w_resolution",1},
  {30,psyshort,"fill1",6},
  {42,psyint,"gate_duration",1},
  {46,psyint,"r_wave_offset",1},
  {50,psyint,"num_accepted_beats",1},
  {54,psyfloat,"scale_factor",1},
  {58,psyshort,"scan_min",1},
  {60,psyshort,"scan_max",1},
  {62,psyint,"prompts",1},
  {66,psyint,"delayed",1},
  {70,psyint,"multiples",1},
  {74,psyint,"net_trues",1},
  {78,psyfloat,"cor_singles",16},
  {142,psyfloat,"uncor_singles",16},
  {206,psyfloat,"tot_avg_cor",1},
  {210,psyfloat,"tot_avg_uncor",1},
  {214,psyint,"total_coin_rate",1},
  {218,psydate,"frame_start_time",1},
  {222,psyint,"frame_duration",1},
  {226,psyfloat,"deadtime_correction_factor",1},
  {230,psyshort,"physical_planes",8},
  {246,psyshort,"fill2",83},
  {412,psyshort,"fill3",50},
  {-1,psychar,"",0}
};
int ecat7_imported_6_5_norm_soff[] = { 0,2,4,6,8,10,32,34,36,
                                       38, 40, 42, 44, 46, 48,
   50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78,
   80, 82, 84, 86, 88, 90, 92, 94, 96, 98,100,102,104,106,108,
  110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,
  140,142,144,146,148,150,152,154,156,158,160,162,164,
                                                      166,168,
  170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,
  200,202,204,206,208,210,212,214,216,218,220,222,224,226,228,
  230,232,234,236,238,240,242,244,246,248,250,252,254,256,258,
  260,262,264,266,268,270,272,274,276,278,280,282,284,286,288,
  290,292,294,296,298,300,302,304,306,308,310,312,314,316,318,
  320,322,324,326,328,330,332,334,336,338,340,342,344,346,348,
  350,352,354,356,358,360,362,364,366,368,370,372,374,376,378,
  380,382,384,386,388,390,392,394,396,398,400,402,404,406,408,
  410,
      412,414,416,418,420,422,424,426,428,430,432,434,436,438,
  440,442,444,446,448,450,452,454,456,458,460,462,464,466,468,
  470,472,474,476,478,480,482,484,486,488,490,492,494,496,498,
  500,502,504,506,508,510,
  -1 };
int ecat7_imported_6_5_norm_loff[] = {
  -1 };
int ecat7_3d_imported_6_5_norm_foff[] = { 12,16,20,24,28,
  -1 };
data_word ecat7_imported_6_5_norm_subheader[]= {
  {0,psyshort,"data_type",1},
  {2,psyshort,"num_dimensions",1},
  {4,psyshort,"num_r_elements",1},
  {6,psyshort,"num_angles",1},
  {8,psyshort,"num_z_elements",1},
  {10,psyshort,"ring_difference",1},
  {12,psyfloat,"scale_factor",1},
  {16,psyfloat,"norm_min",1},
  {20,psyfloat,"norm_max",1},
  {24,psyfloat,"fov_source_width",1},
  {28,psyfloat,"norm_quality_factor",1},
  {32,psyshort,"norm_quality_factor_code",1},
  {34,psyshort,"storage_order",1},
  {36,psyshort,"span",1},
  {38,psyshort,"z_elements",64},
  {166,psyshort,"fill1",123},
  {412,psyshort,"fill2",50},
  {-1,psychar,"",0}
};

void write_swapped_bytes(fstream *fd, unsigned char inbytes[], int size,
			 int *soff, int *loff, int *foff)
{
  unsigned char block[size];
  int *ptr;
  int i;
  for(i=0; i<size; i++)block[i]=inbytes[i];
#ifndef USES_LITTLE_ENDIAN
  for (ptr = (int *) soff; *ptr > 0; ptr++)
    swaper(&block[*ptr]);
  for (ptr = (int *) loff; *ptr > 0; ptr++)
    lswaper(&block[*ptr]);
#endif
  for (ptr = (int *) foff; *ptr > 0; ptr++) {
#ifdef USES_LITTLE_ENDIAN
    // swap little endian to big
    lswaper(&block[*ptr]);
#endif
    // fix floating point to vax format while converting to little_endian
    ifswaper(&block[*ptr]);
  }
  fd->write((char *)block, size);
}

string ecat7filetypetoname(int filetype) { return psyecat7names[filetype]; }

psytype getecat7closesttype(psytype type) {
  return getecatclosesttype(type);
}

int isecat7file(string name, fstream *imgfd, char *main_hdr,
		psyswaptype *swaptype)
{
  fstream *local_imgfd;
  char *local_main_hdr;
  psyswaptype localswaptype = psynoswap;
  int return_value=0;
  if(main_hdr == NULL) local_main_hdr=new char[ECAT7MAINHDRSIZE];
  else local_main_hdr=main_hdr;
// open file with standard c++ constructs
  local_imgfd = imgfd;
  if(local_imgfd == NULL) local_imgfd = new fstream();
  if(! local_imgfd->is_open()) {
    int status=0;
    psyopenfile(name, "r", local_imgfd, &status);
    if(status != 0) return 0;
  }
// read main header
  local_imgfd->seekg(0);
  local_imgfd->read(local_main_hdr, ECAT7MAINHDRSIZE);

  if(! (local_imgfd->fail() | local_imgfd->bad()) ){
// check for magic number
    string magic_number=get_ecat_header_string("magic_number", local_main_hdr,
					       ecat7_main_header);
    if(magic_number.length() >= 6) {
      if(magic_number.substr(0,6).compare("MATRIX") == 0) {
	short file_type=(short) get_ecat_header_value("file_type",
						      local_main_hdr,
						      ecat7_main_header);
	if(file_type > 255) {
	  localswaptype = psyreversewordbytes;
	  swaper((unsigned char*) &file_type);
	}
	switch (file_type) {
	case ECAT7_IMAGE_16:
	case ECAT7_IMAGE_8:
	case ECAT7_ATTENUATION_CORRECTION:
	case ECAT7_NORMALIZATION:
	case ECAT7_POLAR_MAP:
	case ECAT7_PROJECTION_8:
	case ECAT7_PROJECTION_16:
	case ECAT7_3D_SINOGRAM_16:
	case ECAT7_3D_SINOGRAM_8:
	case ECAT7_3D_SINOGRAM_NORMALIZATION:
	case ECAT7_3D_SINOGRAM_FIT:
	case ECAT7_VOLUME_8:
	case ECAT7_VOLUME_16:
	case ECAT7_SINOGRAM:
	  return_value=file_type;
	  break;
	case UNKNOWN_FILE_TYPE:
	default:
	  return_value=0;
	}
      }
    }
  }
// clean up and return true or false
  if(imgfd == NULL) {
    local_imgfd->close();
    delete local_imgfd;
  }
  if(main_hdr == NULL)delete[] local_main_hdr;
  if(swaptype != NULL) *swaptype = localswaptype;
  return(return_value);
}

ecat7file::ecat7file(string fname) : rawfile(fname, "r")
{
// initialize like ecatfile() here also although
// not a problem like write constructor
  file_type=ECAT7_IMAGE_16;
  sub_hdr_info=ecat7_image_subheader;
  initecat7file();
}


ecat7file::ecat7file(string fname, psyimg *psyimgptr,
                     psytype outtype, int ecat7filetype,
		     char *in_main_hdr, char *in_sub_hdr) : rawfile(fname, "w")
{
  if(fname.empty()) {
    output_tree(&cerr);
    cerr<<":ecat7file::ecat7file - empty file name\n";
    exit(1);
  }

  //  fp=NULL;
  data_word *sub_header_info=ecat7_image_subheader;
  int *sub_soff, *sub_loff, *sub_foff;

  initpsyimglnk(psyimgptr, outtype);

  const char *xdim_hdr_name = NULL;
  const char *ydim_hdr_name = NULL;
  const char *zdim_hdr_name = NULL;
  const char *xres_hdr_name = NULL;
  const char *yres_hdr_name = NULL;
  const char *word_res_hdr_name = NULL;


  file_type = ecat7filetype;
  switch(file_type) {
  case ECAT7_VOLUME_8:
  case ECAT7_VOLUME_16:
  case ECAT7_IMAGE_16:
  case ECAT7_IMAGE_8:
  case ECAT7_PROJECTION_8:
  case ECAT7_PROJECTION_16:
    sub_header_info = ecat7_image_subheader;
    sub_soff = ecat7_image_soff;
    sub_loff = ecat7_image_loff;
    sub_foff = ecat7_image_foff;

    xdim_hdr_name = "x_dimension";
    ydim_hdr_name = "y_dimension";
    zdim_hdr_name = "z_dimension";
    xres_hdr_name = "x_pixel_size";
    yres_hdr_name = "y_pixel_size";
    word_res_hdr_name = "scale_factor";

    break;
  case ECAT7_ATTENUATION_CORRECTION:
    sub_header_info = ecat7_attenuation_subheader;
    sub_soff = ecat7_attenuation_soff;
    sub_loff = ecat7_attenuation_loff;
    sub_foff = ecat7_attenuation_foff;

    xdim_hdr_name = "num_r_elements";
    ydim_hdr_name = "num_angles";
    zdim_hdr_name = "num_z_elements";
    xres_hdr_name = "x_resolution";
    yres_hdr_name = "y_resolution";
    word_res_hdr_name = "scale_factor";

    break;
  case ECAT7_NORMALIZATION:
  case ECAT7_3D_SINOGRAM_NORMALIZATION:
    sub_header_info = ecat7_3d_norm_subheader;
    sub_soff = ecat7_3d_norm_soff;
    sub_loff = ecat7_3d_norm_loff;
    sub_foff = ecat7_3d_norm_foff;

    xdim_hdr_name = "num_crystal_rings";
    ydim_hdr_name = "num_r_elements";

    break;
  case ECAT7_POLAR_MAP:
    sub_header_info = ecat7_polar_map_subheader;
    sub_soff = ecat7_polar_map_soff;
    sub_loff = ecat7_polar_map_loff;
    sub_foff = ecat7_polar_map_foff;

    xdim_hdr_name = "num_rings";
    xres_hdr_name = "pixel_size";
    word_res_hdr_name = "scale_factor";

    break;
  case ECAT7_SINOGRAM:
  case ECAT7_3D_SINOGRAM_16:
  case ECAT7_3D_SINOGRAM_8:
  case ECAT7_3D_SINOGRAM_FIT:
    sub_header_info = ecat7_3d_scan_subheader;
    sub_soff = ecat7_3d_scan_soff;
    sub_loff = ecat7_3d_scan_loff;
    sub_foff = ecat7_3d_scan_foff;

    xdim_hdr_name = "num_r_elements";
    ydim_hdr_name = "num_angles";
    zdim_hdr_name = "first_num_z_elements";
    xres_hdr_name = "x_resolution";
    yres_hdr_name = "v_resolution";
    word_res_hdr_name = "scale_factor";

    break;
  case UNKNOWN_FILE_TYPE:
  default:
    output_tree(&cerr);
    cerr<<":ecat7file - unknown output ecat7 file type="<<file_type<<"\n";
    exit(1);
    break;
  }

  outtype=gettype();

// initialize
  int i;
  if(in_main_hdr != NULL) {
    for(i=0; i<ECAT7MAINHDRSIZE; i++) main_hdr[i] = in_main_hdr[i];
  }
  else {
    for(i=0; i<ECAT7MAINHDRSIZE; i++) main_hdr[i] = 0;

// fill in as much of the main header as known
    put_ecat_header_string("MATRIX", "magic_number", main_hdr,
			  ecat7_main_header);
    put_ecat_header_value((float)file_type, "file_type", main_hdr,
			  ecat7_main_header);
// jtl 08/25/2015 -- why did this work? data_type is in sub header not main header
//    put_ecat_header_value((float)psytype2ecattype(outtype),
//			  "data_type", main_hdr, ecat7_main_header);
// jtl 08/25/2015 -- ecat7 has only one value for date and time
//    int day, month, year;
//    getdate(&month, &day, &year);

//    put_ecat_header_value((float)day, "scan_start_day", main_hdr,
//			  ecat7_main_header);
//    put_ecat_header_value((float)month, "scan_start_month", main_hdr,
//			  ecat7_main_header);
//   put_ecat_header_value((float)year, "scan_start_year", main_hdr,
//			  ecat7_main_header);
//    int hour, minute, second;
//    gettime(&hour, &minute, &second);
//    put_ecat_header_value((float)hour, "scan_start_hour",main_hdr,
//			  ecat7_main_header);
//    put_ecat_header_value((float)minute, "scan_start_minute",main_hdr,
//			  ecat7_main_header);
//    put_ecat_header_value((float)second, "scan_start_second",main_hdr,
//			  ecat7_main_header);

//    time_t basetime=time((time_t *)NULL);

    put_ecat_header_value((float) getcurrenttimeinsecs(), "scan_start_time",main_hdr,
			  ecat7_main_header);

    put_ecat_header_string(getpatientid().c_str(),
			   "patient_id",main_hdr, ecat7_main_header);
    put_ecat_header_string(getdescription().c_str(),
			   "study_description",main_hdr, ecat7_main_header);
    put_ecat_header_string("notset", "original_file_name",main_hdr,
			   ecat7_main_header);
    put_ecat_header_string("va_mpls", "facility_name",main_hdr,ecat7_main_header);
    put_ecat_header_string("pardo", "physician_name",main_hdr, ecat7_main_header);
    put_ecat_header_string("jtlee", "operator_name",main_hdr, ecat7_main_header);
    put_ecat_header_string("none", "radiopharmaceutical",main_hdr,
			   ecat7_main_header);
    psydims size=getsize();
    put_ecat_header_value((float)size.z, "num_planes", main_hdr,
			  ecat7_main_header);
    if((int)get_ecat_header_value("num_planes", main_hdr, ecat7_main_header) != size.z) {
      output_tree(&cerr);
      cerr<<":ecat7file - error storing z(plane) size="<<size.z<<" in ecat main header\n";
      exit(1);
    }
    put_ecat_header_value((float)size.i, "num_frames", main_hdr,
			  ecat7_main_header);
    if((int)get_ecat_header_value("num_frames", main_hdr, ecat7_main_header) != size.i) {
      output_tree(&cerr);
      cerr<<":ecat7file - error storing i(frame) size="<<size.i<<" in ecat main header\n";
      exit(1);
    }
    psyres res=getres();
    put_ecat_header_value((float)res.z*100,"plane_separation", main_hdr,
			  ecat7_main_header);
  }

// write out main header
//  write_swapped_bytes(&imgfd, (unsigned char *)main_hdr, ECAT7MAINHDRSIZE,
//		      main_soff, main_loff, main_foff);
  imgfd.write(main_hdr, ECAT7MAINHDRSIZE);

// sub header stuff
  if(in_sub_hdr != NULL) {
    for(i=0; i<ECAT7SUBHDRSIZE; i++) sub_hdr[i] = in_sub_hdr[i];
  }
  else {

    for(i=0; i<ECAT7SUBHDRSIZE; i++) sub_hdr[i] = 0;
    put_ecat_header_value((float)psytype2ecattype(outtype),
			  "data_type", sub_hdr, sub_header_info);
    if(xdim_hdr_name != NULL) {
      put_ecat_header_value((float)size.x, xdim_hdr_name, sub_hdr,
                            sub_header_info);
      if((int)get_ecat_header_value(xdim_hdr_name, sub_hdr, sub_header_info) != size.x) {
        output_tree(&cerr);
        cerr<<":ecat7file - error storing x size="<<size.x;
	cerr<<" in ecat7 subheader \""<<xdim_hdr_name<<"\"\n";
        exit(1);
      }
    }
    if(ydim_hdr_name != NULL) {
      put_ecat_header_value((float)size.y, ydim_hdr_name, sub_hdr,
                            sub_header_info);
      if((int)get_ecat_header_value(ydim_hdr_name, sub_hdr, sub_header_info) != size.y) {
        output_tree(&cerr);
        cerr<<":ecat7file - error storing y size="<<size.y;
	cerr<<" in ecat7 subheader \""<<ydim_hdr_name<<"\"\n";
        exit(1);
      }
    }
    if(zdim_hdr_name != NULL) {
      put_ecat_header_value((float)size.z, zdim_hdr_name, sub_hdr,
                            sub_header_info);
      if((int)get_ecat_header_value(zdim_hdr_name, sub_hdr, sub_header_info) != size.z) {
        output_tree(&cerr);
        cerr<<":ecat7file - error storing z size="<<size.z;
	cerr<<" in ecat7 subheader \""<<zdim_hdr_name<<"\n";
        exit(1);
      }
    }
    if(xres_hdr_name != NULL) {
      put_ecat_header_value((float)res.x, xres_hdr_name, sub_hdr,
                            sub_header_info);
      if(fabs(get_ecat_header_value(xres_hdr_name, sub_hdr, sub_header_info)-res.x) > 1e-10) {
        output_tree(&cerr);
        cerr<<":ecat7file - error storing x res="<<res.x<<" stored="<<get_ecat_header_value(xres_hdr_name, sub_hdr, sub_header_info)<<"\n";
	cerr<<" in ecat7 subheader \""<<xres_hdr_name<<"\"\n";
        exit(1);
      }
    }
    if(yres_hdr_name != NULL) {
      put_ecat_header_value((float)res.y, yres_hdr_name, sub_hdr,
                            sub_header_info);
      if(fabs(get_ecat_header_value(yres_hdr_name, sub_hdr, sub_header_info)-res.y) > 1e-10) {
        output_tree(&cerr);
        cerr<<":ecat7file - error storing y res="<<res.y<<" stored="<<get_ecat_header_value(yres_hdr_name, sub_hdr, sub_header_info)<<"\n";
	cerr<<" in ecat7 subheader \""<<yres_hdr_name<<"\"\n";
        exit(1);
      }
    }
    if(word_res_hdr_name != NULL) {
      double wres = getwordres();
      put_ecat_header_value((float)wres, word_res_hdr_name, sub_hdr,
                            sub_header_info);
      if(fabs(get_ecat_header_value(word_res_hdr_name, sub_hdr, sub_header_info)-wres) > 1e-10) {
        output_tree(&cerr);
        cerr<<":ecat7file - error storing w res="<<wres;
	cerr<<" in ecat7 subheader \""<<word_res_hdr_name<<"\"\n";
        exit(1);
      }
    }

  }
// write sub header
//  write_swapped_bytes(&imgfd, (unsigned char *)sub_hdr, ECAT7SUBHDRSIZE,
//		      sub_soff, sub_loff, sub_foff);
  imgfd.write(sub_hdr, ECAT7SUBHDRSIZE);
// write data
  writedata();
}


void ecat7file::initecat7file()
{
  int xdim, ydim, zdim, idim, xorig, yorig, zorig, iorig, skippixels;
  double xres, yres, zres, ires, wres;
  char *ptr;

  xorig=yorig=zorig=iorig=0;

  if(imgfilename.empty()) {
    output_tree(&cerr);
    cerr<<":ecat7file::initecat7file - empty file name\n";
    exit(1);
  }
// isecat7file opens ecat file for reading, reads main header
// and checks if valid
  file_type = isecat7file(imgfilename, &imgfd, main_hdr, &swaptype);
  if(file_type == 0) {
    output_tree(&cerr);
    cerr<<":ecat7file::initecat7file - error invalid ECAT7 file: "<<imgfilename<<'\n';
    exit(1);
  }
  if(swaptype == psyreversewordbytes)
    swap_data((char *) main_hdr,
	      ecat7_main_soff, ecat7_main_loff, ecat7_main_foff);
  xdim = 0;
  ydim = 0;
  zdim = (int) get_ecat_header_value("num_planes", main_hdr, ecat7_main_header);
  idim = (int) get_ecat_header_value("num_frames", main_hdr, ecat7_main_header);
  skippixels = ECAT7MAINHDRSIZE + ECAT7SUBHDRSIZE;

  xres = 1.0;
  yres = 1.0;
  zres = get_ecat_header_value("plane_separation", main_hdr, ecat7_main_header) *
    1e-2;
  ires = 1.0;
  wres = 1.0;

  int itmp;
  //  int err = 0;

  imgfd.seekg(ECAT7MAINHDRSIZE);
  imgfd.read(sub_hdr, ECAT7SUBHDRSIZE);

  if(imgfd.fail() | imgfd.bad()) {
    output_tree(&cerr);
    cerr<<":ecat7file::initecat7file - error reading subheader: "<<imgfilename<<'\n';
    exit(1);
  }

// set sub header data word array according to file type
  switch (file_type) {
  default:
  case UNKNOWN_FILE_TYPE:
    break;
  case ECAT7_VOLUME_8:
  case ECAT7_VOLUME_16:
  case ECAT7_IMAGE_16:
  case ECAT7_IMAGE_8:
  case ECAT7_PROJECTION_8:
  case ECAT7_PROJECTION_16:
    sub_hdr_info = ecat7_image_subheader;
    if(swaptype == psyreversewordbytes)
      swap_data((char *) sub_hdr,
		ecat7_image_soff, ecat7_image_loff, ecat7_image_foff);

    xdim = (int) get_ecat_header_value("x_dimension", sub_hdr, sub_hdr_info);
    ydim = (int) get_ecat_header_value("y_dimension", sub_hdr, sub_hdr_info);
    itmp = (int) get_ecat_header_value("z_dimension", sub_hdr, 
				       sub_hdr_info);
    zdim = (zdim > itmp)? zdim:itmp;
		    
    xres = get_ecat_header_value("x_pixel_size", sub_hdr, sub_hdr_info) *
      1e-2;
    yres = get_ecat_header_value("y_pixel_size", sub_hdr, sub_hdr_info) *
      1e-2;
    wres = get_ecat_header_value("scale_factor", sub_hdr, sub_hdr_info);
    break;
  case ECAT7_ATTENUATION_CORRECTION:
    sub_hdr_info = ecat7_attenuation_subheader;
    if(swaptype == psyreversewordbytes)
      swap_data((char *) sub_hdr,
		ecat7_attenuation_soff, ecat7_attenuation_loff,
		ecat7_attenuation_foff);
    xdim = (int) get_ecat_header_value("num_r_elements", sub_hdr, sub_hdr_info);
    ydim = (int) get_ecat_header_value("num_angles", sub_hdr, sub_hdr_info);
    itmp = (int) get_ecat_header_value("num_z_elements", sub_hdr,
				       sub_hdr_info);
    zdim = (zdim > itmp)? zdim:itmp;
    xres = get_ecat_header_value("x_resolution", sub_hdr, sub_hdr_info) * 1e-2;
    yres = get_ecat_header_value("y_resolution", sub_hdr, sub_hdr_info) * 1e-2;
    wres = get_ecat_header_value("scale_factor", sub_hdr, sub_hdr_info);
    break;
  case ECAT7_NORMALIZATION:
  case ECAT7_3D_SINOGRAM_NORMALIZATION:
    sub_hdr_info = ecat7_3d_norm_subheader;
    if(swaptype == psyreversewordbytes)
      swap_data((char *) sub_hdr,
		ecat7_3d_norm_soff, ecat7_3d_norm_loff,
		ecat7_3d_norm_foff);
    ydim = (int) get_ecat_header_value("num_r_elements", sub_hdr, sub_hdr_info);
    xdim = (int) get_ecat_header_value("num_crystal_rings", sub_hdr,
				       sub_hdr_info);
    xdim *= (int) get_ecat_header_value("crystals_per_ring",
					sub_hdr, sub_hdr_info);
    break;
  case ECAT7_POLAR_MAP:
    sub_hdr_info = ecat7_polar_map_subheader;
    if(swaptype == psyreversewordbytes)
      swap_data((char *) sub_hdr,
		ecat7_polar_map_soff, ecat7_polar_map_loff,
		ecat7_polar_map_foff);
    // i don't know sizes
    xdim = (int) get_ecat_header_value("num_rings", sub_hdr, sub_hdr_info);
//      xdim *= getIntValue("sectors_per_ring", sub_hdr_info, sub_hdr);
    ydim = 1;
    xres = (int) get_ecat_header_value("pixel_size", sub_hdr, sub_hdr_info) *
      1e-2;
    yres = xres;
    wres = get_ecat_header_value("scale_factor", sub_hdr, sub_hdr_info);
    break;
  case ECAT7_SINOGRAM:
  case ECAT7_3D_SINOGRAM_16:
  case ECAT7_3D_SINOGRAM_8:
  case ECAT7_3D_SINOGRAM_FIT:
    sub_hdr_info = ecat7_3d_scan_subheader;
    if(swaptype == psyreversewordbytes)
      swap_data((char *) sub_hdr,
		ecat7_3d_scan_soff, ecat7_3d_scan_loff,	ecat7_3d_scan_foff);
    xdim = (int) get_ecat_header_value("num_r_elements", sub_hdr, sub_hdr_info);
    ydim = (int) get_ecat_header_value("num_angles", sub_hdr, sub_hdr_info);
    itmp = (int) get_ecat_header_value("first_num_z_elements",
				       sub_hdr, sub_hdr_info);
    zdim = (zdim > itmp)? zdim:itmp;
    xres = get_ecat_header_value("x_resolution", sub_hdr, sub_hdr_info) * 1e-2;
    yres = get_ecat_header_value("v_resolution", sub_hdr, sub_hdr_info);
    wres = get_ecat_header_value("scale_factor", sub_hdr, sub_hdr_info);
    int storage_order = (int) get_ecat_header_value("storage_order",
						    sub_hdr, sub_hdr_info);
    if(storage_order != 0) {
      // swap z and angle?
    }
    break;
  }
  int ecattype = (int) get_ecat_header_value("data_type",
					     sub_hdr, sub_hdr_info);
  psytype pixeltype = ecattype2psytype(ecattype);
// initialize psyimglnk information
  initpsyimglnk(NULL, xdim, ydim, zdim, idim, pixeltype,
		xorig, yorig, zorig, iorig, skippixels,
		xres, yres, zres, ires, wres);
  setpatientid(get_ecat_patientid(main_hdr,
				  ecat7_main_header));
  time_t time=(time_t) get_ecat_header_value("scan_start_time",
					     main_hdr, ecat7_main_header);
  ptr = new char[16];
  tm *tm_time = localtime(&time);
  int cnt=strftime(ptr, 16, "%D", tm_time);
  setdate(ptr);
  cnt=strftime(ptr, 16, "%T", tm_time);
  settime(ptr);
  delete[] ptr;
  setdescription(get_ecat_header_string("study_description",
					main_hdr,
					ecat7_main_header));
}

ecat7file::~ecat7file()
{
//  if(fp != NULL) fclose(fp);
}

char *ecat7file::get_mainheader(char *out_main_hdr) {
  if(main_hdr != NULL) {
    if(out_main_hdr == NULL) out_main_hdr = new char[ECAT7MAINHDRSIZE];
    for(int i=0; i<ECAT7MAINHDRSIZE; i++) out_main_hdr[i] = main_hdr[i];
    return out_main_hdr;
  }
  else return NULL;
}

char *ecat7file::get_subheader(char *out_sub_hdr) {
  if(sub_hdr != NULL) {
    if(out_sub_hdr == NULL) out_sub_hdr = new char[ECAT7SUBHDRSIZE];
    for(int i=0; i<ECAT7SUBHDRSIZE; i++) out_sub_hdr[i] = sub_hdr[i];
    return out_sub_hdr;
  }
  else return NULL;
}

void ecat7file::showhdr(ostream *out)
{
  showmainhdr(out);
  showsubhdr(out);
}

void ecat7file::showmainhdr(ostream *out)
{
  *out<<"main header:\n";
  show_ecat_header(main_hdr, ecat7_main_header, out);
}

void ecat7file::showsubhdr(ostream *out)
{
  *out<<ecat7filetypetoname(get_ecat7filetype())<<" subheader:\n";
  show_ecat_header(sub_hdr, sub_hdr_info, out);
}
