set(FUZZY_SOURCES
  fuzzy_match.cc
  ngram_matches.cc
  suffix_array_index.cc
  vocab_indexer.cc
  suffix_array.cc
  sentence.cc
  fuzzy_matcher_binarization.cc
  edit_distance.cc
)

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

target_include_directories(${PROJECT_NAME} PUBLIC
  "${ICU_ROOT}/include"
  ${Boost_INCLUDE_DIRS}
  ${OPENNMT_TOKENIZER_INCLUDE}
  )

target_link_libraries(${PROJECT_NAME}
  ${OPENNMT_TOKENIZER_LIB}
  icuuc icui18n
  ${Boost_LIBRARIES}
  Threads::Threads
  )
