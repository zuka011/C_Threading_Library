#!/bin/bash

TESTS=*Test

if [[ $TESTS == "*Test" ]]
then 
    make All
fi

SUCCESS='ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)'
PASSED=0
TOTAL_TESTS=0

echo "Performing Memory Test on:" $TESTS

printf "Memory check results:\n\n" > memLog.txt
printf "Test outputs:\n\n" > output.txt
for test in $TESTS;
    do echo "Testing:" $test;

    echo > temp.txt
  
    printf "Testing: $test\n" >> memLog.txt
    printf "Testing: $test\n" >> output.txt  
    valgrind --tool=memcheck --log-file="temp.txt" ./$test >> output.txt

    cat temp.txt >> memLog.txt

    RESULT=$(tail -1 memLog.txt)

    if [[ "$RESULT" == *"$SUCCESS"* ]]
    then
        echo "//=====SUCCESS=====//"
        ((PASSED=PASSED+1))
    else 
        echo "//=====FAILURE=====//"
    fi

    ((TOTAL_TESTS=TOTAL_TESTS+1))

    printf "\n" >> memLog.txt
    printf "\n" >> output.txt

done

if [[ -e temp.txt ]] 
then 
    rm temp.txt
fi

echo "Memory check done. Passed:" $PASSED"/"$TOTAL_TESTS