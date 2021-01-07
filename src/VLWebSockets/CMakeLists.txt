project (VLWebSockets)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  VirtualLab/ws/WebSocketAPI.cpp
)

set (HEADERFILES
  VirtualLab/ws/WebSocketAPI.h
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
  ${CMAKE_PROJECT_DIR}/../VLCore
)

set(DEP_LIBS ${DEP_LIBS} VLCore CppWebServer websockets)

set(ALL_LIBS ${ALL_LIBS} crypto ssl z)

# --------------------- Executable -----------------------

addStandardLibrary()

