project (VLExplorer)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  src/main.cpp
)

set (HEADERFILES
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
/panfs/roc/groups/0/keefedf/dtorban/project/deps/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
/panfs/roc/groups/0/keefedf/dtorban/project/deps/include
  ${CMAKE_PROJECT_DIR}/../VLCore
)

#add_external(CppWebServerLib
#	GIT_REPOSITORY https://github.com/dtorban/CppWebServer.git
#	LIB_NAME CppWebServer
#)

#set(DEP_INCLUDES ${DEP_INCLUDES} ${external_dir}/CppWebServerLib/src/src)
#set(DEP_INCLUDES ${DEP_INCLUDES} ${external_dir}/CppWebServerLib/src/cmake/framework/external/WebSockets/build/install/include)

#set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
#  ${external_dir}/CppWebServerLib/src/cmake/framework/external/WebSockets/build/install/lib
#)

set(DEP_LIBS ${DEP_LIBS} VLml VLopt VLCore CppWebServer websockets)

set(ALL_LIBS ${ALL_LIBS} crypto ssl z)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") 

# --------------------- Executable -----------------------

addStandardExecutable()

