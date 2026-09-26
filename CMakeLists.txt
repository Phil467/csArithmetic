cmake_minimum_required(VERSION 3.16)
project(csArithmetic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
set(CMAKE_IMPORT_LIBRARY_PREFIX "lib")
set(CMAKE_IMPORT_LIBRARY_SUFFIX ".a")

add_library(csArithmetic SHARED
  src/csARITHMETIC.cpp
  src/csAddition.cpp
  src/csArithUtils.cpp
  src/csDivision.cpp
  src/csModulus.cpp
  src/csMultiplication.cpp
  src/csQNUMBER.cpp
  src/csRNUMBER.cpp
  src/csSQRT.cpp
  src/csSubstraction.cpp
)

target_include_directories(csArithmetic PUBLIC include)
target_compile_definitions(csArithmetic PRIVATE BUILDING_CSARITHMETIC_DLL)

set_target_properties(csArithmetic PROPERTIES
  OUTPUT_NAME "csArithmetic"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
