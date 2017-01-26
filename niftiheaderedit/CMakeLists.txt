ADD_EXECUTABLE(niftiheaderedit niftiheaderedit.cc)
TARGET_LINK_LIBRARIES(niftiheaderedit psyimg)
INSTALL(TARGETS niftiheaderedit RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
