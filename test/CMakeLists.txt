add_executable(${PROJECT_NAME}-test test.cc)

find_package(GTest REQUIRED)

find_package(Boost COMPONENTS filesystem system serialization REQUIRED)

include_directories(
  ${source_dir}/include
  ${GTest_INCLUDE_DIRS}
  )

target_include_directories(${PROJECT_NAME}-test PUBLIC
  ../include
  ${Boost_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}-test
  ${PROJECT_NAME}
  GTest::GTest GTest::Main
  Boost::filesystem
  Boost::serialization
  Boost::system
  )

set(${PROJECT_NAME}-test-args "${CMAKE_CURRENT_LIST_DIR}/data/")
gtest_add_tests(${PROJECT_NAME}-test
  "${${PROJECT_NAME}-test-args}"
  AUTO
  )
