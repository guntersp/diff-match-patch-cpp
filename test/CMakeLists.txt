find_package(Catch2 REQUIRED)

include(CTest)
include(Catch)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2 diff-match-patch-cpp)
target_link_libraries(catch_main PRIVATE project_options)

add_executable(tests 
  dmp_test_non_allocating.cpp
  dmp_test_string.cpp
  dmp_test_wstring.cpp
  dmp_speedtest_wstring.cpp
  )
target_link_libraries(tests PRIVATE project_warnings project_options catch_main)

target_sources(tests
  PRIVATE
    dmp_test_interface.h
    dmp_test_non_allocating_interface.h
    dmp_tests.h
    Speedtest1.txt.h
    Speedtest2.txt.h
  )

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  tests
  TEST_PREFIX
  "unittests."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)

# Add a file containing a set of constexpr tests
add_executable(constexpr_tests dmp_test_constexpr.cpp)
target_link_libraries(constexpr_tests PRIVATE project_options project_warnings catch_main)

catch_discover_tests(
  constexpr_tests
  TEST_PREFIX
  "constexpr."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "constexpr."
  OUTPUT_SUFFIX
  .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
add_executable(relaxed_constexpr_tests dmp_test_constexpr.cpp)
target_link_libraries(relaxed_constexpr_tests PRIVATE project_options project_warnings catch_main)
target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

catch_discover_tests(
  relaxed_constexpr_tests
  TEST_PREFIX
  "relaxed_constexpr."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "relaxed_constexpr."
  OUTPUT_SUFFIX
  .xml)
