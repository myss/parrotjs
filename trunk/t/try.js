
try {
	throw "A test";
} catch(e) {
	var x = 10;
	print('Caught 1:', e);
}

print(x);

try {
	// js does not throw with this, it gives infinity
	// var i = 3 / 0;
	
	// it will throw a reference error
	iDoNotExist;
} catch (e) {
	print('Caught 2:', e);
}
