#!/bin/bash

# Het genereert een grote js bestand, maar met geen diep-geneste
# expressies. Het blijkt dat het daarom niet veel geheugen gebruikt in
# vgl met genestelde expressies.

if [ "" = "$1" ]
then
	echo 'Usage: generate_large_js.sh n' >&2
	exit 1
fi

echo 'function f(n){ return n; }'
echo 'x = 12345;'

n=$1
i=0
while [ $i -lt $n ]
do
	echo 'f(x);'
	let i=i+1
done

echo 'print("End");'
