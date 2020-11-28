#!/bin/bash

cd ../../

exit_count=0
exit_gtest=0
exit_gtest_mp=0
exit_gtest_mp3=0
exit_des_ser=0
exit_normal=0
exit_tcp=0
exit_udp=0
#remove gcda files build folder
find build.debug/ -name '*.gcda' | xargs rm -f

mkdir -p UnitTests/OPStest-C++/Unit_test_results
date > UnitTests/OPStest-C++/Unit_test_results/UnitTests-result.txt
build.debug/UnitTests/OPStest-C++/gtest/gtest-main
exit_gtest=$?
build.debug/UnitTests/OPStest-C++/gtest/gtest-mempool
exit_gtest_mp=$?
build.debug/UnitTests/OPStest-C++/gtest/gtest-mempool3
exit_gtest_mp3=$?

build.debug/UnitTests/OPStest-C++/UnitTests/test-serialize-and-deserialize
exit_des_ser=$?

build.debug/UnitTests/OPStest-C++/UnitTests/test-subscribe &
pid_normal=$!

build.debug/UnitTests/OPStest-C++/UnitTests/test-subscribeTCP &
pid_tcp=$!

build.debug/UnitTests/OPStest-C++/UnitTests/test-subscribeUDP &
pid_udp=$!

sleep 1
build.debug/UnitTests/OPStest-C++/src/publishBin

wait $pid_normal
exit_normal=$?

wait $pid_tcp
exit_tcp=$?

wait $pid_udp
exit_udp=$?

mkdir -p UnitTests/OPStest-C++/Unit_test_results/Coverage
gcovr -r . --html --html-details -o UnitTests/OPStest-C++/Unit_test_results/Coverage/coverage_result_OPS.html

cppcheck --enable=all Cpp/source Cpp/include 2> UnitTests/OPStest-C++/Unit_test_results/SA_ERRORS.txt > /dev/null
exit_count=$(( $exit_gtest + $exit_gtest_mp + $exit_gtest_mp3 + $exit_des_ser + $exit_normal + $exit_tcp  + $exit_udp ))

if [ $exit_count == 0 ]
then
	printf "\n\n   ALL TEST PASSED\n\n\n"
else
	printf "\n\n   $exit_count TEST FAILED\n\n\n"
fi
exit $exit_count
