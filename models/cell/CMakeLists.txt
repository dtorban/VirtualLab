project (CellModel)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  src/main.cpp
)

set (HEADERFILES
)

add_external(CellSim
	GIT_REPOSITORY https://github.umn.edu/ivlab-cs/CellSimulator.git
    GIT_TAG lib
    LIB_NAME CellSim
)
include_directories(${external_dir}/CellSim/src/src)

add_external(Eigen3
GIT_REPOSITORY https://github.com/eigenteam/eigen-git-mirror.git
)

include_directories(
${external_dir}/Eigen3/build/install/include/eigen3
)

if(MSVC)
	set(DEP_LIBS gsl ${DEP_LIBS} libgsl.a libgslcblas.a)
else()
	set(DEP_LIBS gsl ${DEP_LIBS} libgsl.a libgslcblas.a)
endif()

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
  ${external_dir}/CellSim/src/libs/gsl/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
  ${external_dir}/CellSim/src/libs/gsl/include
  ${CMAKE_PROJECT_DIR}/../VLCore
)

set(DEP_LIBS ${DEP_LIBS} VLCore CppWebServer websockets)

set(ALL_LIBS ${ALL_LIBS} crypto ssl z)

include(${external_dir}/CPP11/CPP11.cmake)
useCPP11()

# --------------------- Executable -----------------------

addStandardExecutable()

