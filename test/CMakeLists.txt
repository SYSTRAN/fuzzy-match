add_executable(${PROJECT_NAME}-test test.cc)

# find_package (Eigen3 3.3 QUIET)
# if(TARGET Eigen3::Eigen)
#   add_definitions(-DUSE_EIGEN)
# endif()

find_package(GTest REQUIRED)

find_package(Boost COMPONENTS filesystem REQUIRED)

include_directories(
  ${source_dir}/include
  ${GTEST_INCLUDE_DIRS}
  )

target_include_directories(${PROJECT_NAME}-test PUBLIC
  ../include
  ${Boost_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}-test
  ${PROJECT_NAME}
  GTest::GTest GTest::Main
  ${Boost_FILESYSTEM_LIBRARY}
  ${GTEST_LIBRARIES}
  )

set(${PROJECT_NAME}-test-args "${CMAKE_CURRENT_LIST_DIR}/data/")
gtest_add_tests(${PROJECT_NAME}-test
  "${${PROJECT_NAME}-test-args}"
  AUTO
  )
