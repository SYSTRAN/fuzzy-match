project(FuzzyMatch)

cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-pedantic -ansi)

if(TARGET OpenNMTTokenizer)
  set(OPENNMT_TOKENIZER_LIB OpenNMTTokenizer)
  set(OPENNMT_TOKENIZER_INCLUDE "")  # Resolved via the target.
else()
  find_library(OPENNMT_TOKENIZER_LIB OpenNMTTokenizer REQUIRED HINTS ${OPENNMT_TOKENIZER_ROOT}/lib)
  find_path(OPENNMT_TOKENIZER_INCLUDE onmt/Tokenizer.h REQUIRED HINTS ${OPENNMT_TOKENIZER_ROOT}/include)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)

if (NOT LIB_ONLY)
  add_subdirectory(cli)
endif()

if (NOT CLI_ONLY AND NOT LIB_ONLY)
  add_subdirectory(test)
endif()
