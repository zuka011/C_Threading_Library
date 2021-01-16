#!/bin/bash

TESTS=*Test


# Check for command line arguments
if [[ $# == 1 ]]; then
    
    VERBOSE=$1

    if [[ $VERBOSE != "-v" ]]; then
        
        echo "Not a valid flag. You can add -v for a verbose output."
        exit 0
    fi
fi

# Check if tests have been compiled
if [[ $TESTS == "*Test" ]]
then 
    make All
fi

# Perform tests, log outputs and check for memory leaks
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
    
    valgrind --tool=memcheck --log-file="temp.txt" ./$test $VERBOSE >> output.txt

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

rm *Test

echo "Memory check done. Passed:" $PASSED"/"$TOTAL_TESTS