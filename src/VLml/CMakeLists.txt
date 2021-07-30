project (VLml)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  VirtualLab/pca/PCAModel.cpp
  VirtualLab/pca/PCA.cpp
)

set (HEADERFILES
  VirtualLab/pca/PCAModel.h
  VirtualLab/pca/PCA.h
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
)

set(DEP_LIBS ${DEP_LIBS} VLCore)

if (EXISTS /usr/include/mlpack/prereqs.hpp)
  set(DEP_LIBS ${DEP_LIBS} mlpack armadillo)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp") 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") 
  add_definitions(-DUSE_MLPACK)
endif()

# --------------------- Executable -----------------------

addStandardLibrary()