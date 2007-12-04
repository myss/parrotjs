
try {
	for(i in "test") ;
} catch(e) {
	print("error 1");
}

try {
    for(i in 3.5) ;
} catch(e) {
    print("error 2");
}

try {
    for(i in false) ;
} catch(e) {
    print("error 3");
}

// spidermonkey seems to unbox null and undefined
// but it hasn't to (see 'ToObject' in ECMA-262 3rd Edition)
if ('__pjs__' in this) {
	// pjs
	try {
	    for(i in null) ;
	} catch(e) {
	    print("error 4");
	}

	try {
	    for(i in undefined) ;
	} catch(e) {
	    print("error 5");
	}
} 
else {
	// spidermonkey
	print("error 4");
	print("error 5");
}


