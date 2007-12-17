#!/bin/bash

make import_test_files >/dev/null 2>/dev/null

echo "Does testing without logging."

PJS="../../parrot pjs.pir"

args=$@
if [ "$args" = "" ]
then
	args=t/*.js
fi

n=0
nFailed=0
nPassed=0

for file in $args
do
	echo $file | grep 'perf' >/dev/null
	if [ $? = '1' ]
	then
	
	S0=`$PJS $file 2>/dev/null`
	S1=`js $file 2>/dev/null`
	if [ "$S0" = "$S1" ]
	then
		echo "Passed: $file"
		(( nPassed = nPassed+1 ))
	else
		echo "FAILED: $file"
		(( nFailed = nFailed+1 ))
	fi
	(( n = n+1 ))
	
	fi
done

echo "--"
echo "Total : $n"
echo "Passed: $nPassed"
echo "Failed: $nFailed"
