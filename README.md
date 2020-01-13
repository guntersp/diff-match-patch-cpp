diff-match-patch-cpp
====================

Another C++ variant of https://github.com/google/diff-match-patch.
based on
- STL Port was done by Sergey Nozhenko (snhere@gmail.com) and posted on
		https://code.google.com/p/google-diff-match-patch/issues/detail?id=25
- and the weaks for std::string by leutloff@sundancer.oche.de (Christian Leutloff)

This version was done following the current C# version as close as possible algorithm wise. However, this version has some special features:
- completely header-only as the old versions but the single large file has been splitted into separate files
- usage of string views
- tries to prevent most of the string creation operations
- string and container classes are completely exchangable by traits and wrappers
- support for more than 256 encoded lines in case of 1byte strings (by using separate traits for line encoding with 16 bit characters)
- support for string pool, thus completely eliminating the need for dynamic string creation
- everything is constexpr compatible


Compile Example

	MSVC:
		cl /O2 /Idiff-match-patch-cpp /std:c++17 example.cpp
	
	LLVM on windows:
		clang-cl /O2 /Idiff-match-patch-cpp /std:c++17 example.cpp
	
	LLVM on macOs/linux/unix:
		clang++ -O2 -I diff-match-patch-cpp -std=c++17 example.cpp

Compile and run the test cases:

	MSVC:
		cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_string.cpp
		cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_wstring.cpp
		cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_non_allocating.cpp
		cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_constexpr.cpp

	LLVM on windows:
		clang-cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_string.cpp
		clang-cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_wstring.cpp
		clang-cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_non_allocating.cpp
		clang-cl /O2 /Idiff-match-patch-cpp /Igoogletest\include /Igoogletest /std:c++17 tests\dmp_test_constexpr.cpp

	LLVM on macOs/linux/unix:
		clang++ -O2 -I diff-match-patch-cpp -I googletest/include -I googletest -std=c++17 tests/dmp_test_string.cpp
		clang++ -O2 -I diff-match-patch-cpp -I googletest/include -I googletest -std=c++17 tests/dmp_test_wstring.cpp
		clang++ -O2 -I diff-match-patch-cpp -I googletest/include -I googletest -std=c++17 tests/dmp_test_non_allocating.cpp
		clang++ -O2 -I diff-match-patch-cpp -I googletest/include -I googletest -std=c++17 tests/dmp_test_constexpr.cpp

Compile and run the speedtest - requires the files speedtest1.txt and speedtest2.txt for comparison:

	MSVC:
		cl /O2 /Idiff-match-patch-cpp /std:c++17 tests\dmp_speedtest_wstring.cpp

	LLVM on windows:
		clang-cl /O2 /Idiff-match-patch-cpp /std:c++17 tests\dmp_speedtest_wstring.cpp

	LLVM on macOs/linux/unix:
		clang++ -O2 -I diff-match-patch-cpp -std=c++17 tests/dmp_speedtest_wstring.cpp
