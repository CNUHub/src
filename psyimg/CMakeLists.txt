set(PSYIMG_SRCS analyze.cc ecat7file.cc mathfunc.cc psyimgdblz.cc sdtfile.cc cnustats.cc ecatfile.cc nifti.cc psyiofunctions.cc talairach_func.cc conversions.cc filter.cc proc1img.cc psyshapes.cc template.cc dicomfile2.cc imgmath.cc proc2img.cc psytools.cc timefunc.cc dicomfile.cc logfile.cc psyimg.cc rawfile.cc transforms.cc read_ecat2_utils.c  write_ecat2_utils.c)
add_library(psyimg STATIC ${PSYIMG_SRCS})
INSTALL(TARGETS psyimg RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
