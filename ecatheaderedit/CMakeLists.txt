ADD_EXECUTABLE(ecatheaderedit ecatheaderedit.cc)
TARGET_LINK_LIBRARIES(ecatheaderedit psyimg)
INSTALL(TARGETS ecatheaderedit RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
