#!/bin/bash

PJS="../../parrot pjs.pir"


if [ "$1" = "--operator" ]
then
    files=t/operator/*/*.js
    base=testing/operators
else
    files=t/*.js
    base=testing/main
fi

mkdir -p $base
touch $base/NEXT_TEST

CURR_TEST=$( cat $base/NEXT_TEST )
(( CURR_TEST = CURR_TEST + 0 ))  # ensure numeric
mkdir -p $base/results/$CURR_TEST


# Make tests, and write test output for each js file.
# Write alse the test summary to the file RESULTS.

(( n=0, nFailed=0, nPassed=0 ))
echo >$base/results/$CURR_TEST/RESULTS
for file in $files
do
    echo $file | grep 'perf' >/dev/null
    if [ $? = '1' ]
    then
    
        OUTPUT_FILE=$base/results/$CURR_TEST/$file.out
        mkdir -p $( dirname $OUTPUT_FILE )
    
        PJS_OUTPUT=$( $PJS $file 2>&1 | tee $OUTPUT_FILE )
        JS_OUTPUT=$( js   $file 2>&1 )
        
        if [ "$PJS_OUTPUT" = "$JS_OUTPUT" ]
        then
            echo "Passed: $file" | tee -a $base/results/$CURR_TEST/RESULTS
            (( nPassed = nPassed+1 ))
        else
            echo "FAILED: $file" | tee -a $base/results/$CURR_TEST/RESULTS
            (( nFailed = nFailed+1 ))
        fi
        (( n = n+1 ))
    
    fi
done
echo "--" | tee -a $base/results/$CURR_TEST/RESULTS
echo "Total : $n" | tee -a $base/results/$CURR_TEST/RESULTS
echo "Passed: $nPassed" | tee -a $base/results/$CURR_TEST/RESULTS
echo "Failed: $nFailed" | tee -a $base/results/$CURR_TEST/RESULTS
echo


# Generate MD5 checksum for the test results
CURR_MD5=$( find $base/results/$CURR_TEST/ -name '*.out' | xargs cat | md5sum | cut -f 1 -d ' ' )
echo $CURR_MD5 >$base/results/$CURR_TEST/MD5


# Check if this test did have an output different from the previous.
# If so, increment the NEXT_TEST value.
(( PREV_TEST = CURR_TEST - 1 ))
if (( PREV_TEST >= 0 ))
then
    PREV_MD5=$( cat $base/results/$PREV_TEST/MD5 )
    if [ "$PREV_MD5" = "$CURR_MD5" ]
    then
        echo "No difference with the previous test!"
        rm -r $base/results/$CURR_TEST/
        exit 0
    fi
    
    echo "===== DIFF ====="
    diff $base/results/$PREV_TEST/RESULTS $base/results/$CURR_TEST/RESULTS
    echo "================"
fi

(( NEXT_TEST = CURR_TEST + 1 ))
echo $NEXT_TEST >$base/NEXT_TEST


# Write some more info about the current state of the code.
echo "[Time]" >$base/results/$CURR_TEST/LOG
date >>$base/results/$CURR_TEST/LOG

echo
echo
echo "[Latest svn revision]" >>$base/results/$CURR_TEST/LOG
svn info >>$base/results/$CURR_TEST/LOG

echo
echo
echo "[Parrot configuration]" >>$base/results/$CURR_TEST/LOG
cat ../../myconfig >>$base/results/$CURR_TEST/LOG
