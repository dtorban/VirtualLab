project (VLCore)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  VirtualLab/Ensemble.cpp
)

set (HEADERFILES
  VirtualLab/Ensemble.h
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
)

set(DEP_LIBS ${DEP_LIBS} mysqlcppconn)

# --------------------- Executable -----------------------

addStandardLibrary()

