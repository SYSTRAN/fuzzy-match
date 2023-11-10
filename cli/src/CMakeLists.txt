add_executable(${PROJECT_NAME}-cli
  FuzzyMatch-cli.cc
  )

find_package(Boost COMPONENTS program_options REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${Boost_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}-cli
  ${PROJECT_NAME}
  Boost::program_options
  gperftools::tcmalloc_minimal
  )
