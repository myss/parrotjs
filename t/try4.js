function a(cont, n) {
	print(n);
	if (n > 5)
		throw "return";
	try {
		throw "exc";
	} finally {
		print("finally 1");
		cont(cont, n+1);
	}
}


function b() {
	try {
		a(a, 0);
	} finally {
		print("end");
	}
}

try {
	b();
} catch(e) {
	print("caught: " + e);
}
