ADD_EXECUTABLE(ecatheader ecatheader.cc)
TARGET_LINK_LIBRARIES(ecatheader psyimg)
INSTALL(TARGETS ecatheader RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
