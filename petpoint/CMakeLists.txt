ADD_EXECUTABLE(petpoint petpoint.cc)
TARGET_LINK_LIBRARIES(petpoint psyimg)
INSTALL(TARGETS petpoint RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
