#!/bin/bash

# Het genereert een grote js bestand met diep geneste expressies.
# Op mijn machine is n > 4996 om
# onvoldoende heap geheugen te krijgen
# bij het parsen van het gegenereerde programma.

if [ "" = "$1" ]
then
	echo 'Usage: generate_large_js.sh depth' >&2
	exit 1
fi

echo 'function f(n){ return n; }'

echo -n 'print('

n=$1
i=0
while [ $i -lt $n ]
do
	echo -n 'f('
	let i=i+1
done

echo -n '12345'

i=0
while [ $i -lt $n ]
do
    echo -n ')'
    let i=i+1
done

echo -n ');'

echo
