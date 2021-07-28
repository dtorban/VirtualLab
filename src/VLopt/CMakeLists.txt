project (VLopt)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
VirtualLab/opt/InteractiveModel.cpp
  VirtualLab/opt/OptimizedModel.cpp
  VirtualLab/opt/LeastSquares.cpp
)

set (HEADERFILES
  VirtualLab/opt/InteractiveModel.h
  VirtualLab/opt/OptimizedModel.h
  VirtualLab/opt/LeastSquares.h
)

add_external(Eigen3
	GIT_REPOSITORY https://github.com/eigenteam/eigen-git-mirror.git
  HEADER_ONLY
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
)
message(${external_dir}/Eigen3/src)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
  ${external_dir}/Eigen3/src
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") 

set(DEP_LIBS ${DEP_LIBS} VLCore)

# --------------------- Executable -----------------------

addStandardLibrary()