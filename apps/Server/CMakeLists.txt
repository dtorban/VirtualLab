project (Server)

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

set(DEP_LIBS ${DEP_LIBS} VLCore )

set(ALL_LIBS ${ALL_LIBS} crypto ssl z)

# --------------------- Executable -----------------------

addStandardExecutable()

