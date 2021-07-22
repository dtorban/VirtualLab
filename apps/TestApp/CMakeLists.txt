project (TestApp)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  src/main.cpp
)

set (HEADERFILES
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  /dependencies/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  /dependencies/include
  ${CMAKE_PROJECT_DIR}/../VLCore
)

set(DEP_LIBS ${DEP_LIBS} VLCore)

set(ALL_LIBS ${ALL_LIBS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") 

# --------------------- Executable -----------------------

addStandardExecutable()

