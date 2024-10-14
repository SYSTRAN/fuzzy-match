set(FUZZY_SOURCES
  fuzzy_match.cc
  filter_matches.cc
  ngram_matches.cc
  bm25_matches.cc
  vocab_indexer.cc
  suffix_array.cc
  bm25.cc
  sentence.cc
  fuzzy_matcher_binarization.cc
  edit_distance.cc
  pattern_coverage.cc
  filter.cc
  index.cc
  no_filter.cc
  no_matches.cc
  submodular.cc
)
if(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package (Eigen3 3.3 QUIET)
if(TARGET Eigen3::Eigen)
  message("-- Found Eigen3, building all sources.")
  add_definitions(-DUSE_EIGEN)
else()
  message("-- Eigen3 not found, excluding bm25_matches.cc and bm25.cc.")
  list(REMOVE_ITEM FUZZY_SOURCES bm25_matches.cc bm25.cc)
  add_definitions(-DNO_EIGEN)
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
  # ${ICU_LIBRARIES}
  ${Boost_LIBRARIES}
  Threads::Threads
  )

if(TARGET Eigen3::Eigen)
  target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)
endif()