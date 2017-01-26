ADD_EXECUTABLE(connectedregion connectedregion.cc)
TARGET_LINK_LIBRARIES(connectedregion psyimg)
INSTALL(TARGETS connectedregion RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
