ADD_EXECUTABLE(convertto1d convertto1d.cc)
TARGET_LINK_LIBRARIES(convertto1d psyimg)
INSTALL(TARGETS convertto1d RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
