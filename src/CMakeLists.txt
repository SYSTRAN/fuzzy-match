set(FUZZY_SOURCES
  fuzzy_match.cc
  ngram_matches.cc
  suffix_array_index.cc
  vocab_indexer.cc
  suffix_array.cc
  sentence.cc
  fuzzy_matcher_binarization.cc
  edit_distance.cc
  pattern_coverage.cc
)
if(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
if (COMMAND create_library)
  create_library(${PROJECT_NAME} SHARED
    ${FUZZY_SOURCES})
else()
  add_library(${PROJECT_NAME} SHARED
    ${FUZZY_SOURCES})
endif()

find_package(Boost COMPONENTS serialization iostreams system REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT DEFINED ICU_INCLUDE_DIRS OR NOT DEFINED ICU_LIBRARIES)
  find_package(ICU COMPONENTS uc i18n data REQUIRED)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
  ${ICU_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OPENNMT_TOKENIZER_INCLUDE}
  )

target_link_libraries(${PROJECT_NAME}
  ${OPENNMT_TOKENIZER_LIB}
  ${ICU_LIBRARIES}
  ${Boost_LIBRARIES}
  Threads::Threads
  )
