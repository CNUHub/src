ADD_EXECUTABLE(analyzeheaderedit analyzeheaderedit.cc)
TARGET_LINK_LIBRARIES(analyzeheaderedit psyimg)
INSTALL(TARGETS analyzeheaderedit RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
