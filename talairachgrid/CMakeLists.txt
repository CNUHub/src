ADD_EXECUTABLE(talairachgrid talairachgrid.cc)
TARGET_LINK_LIBRARIES(talairachgrid psyimg)
INSTALL(TARGETS talairachgrid RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
