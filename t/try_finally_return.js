function f() {
	try {
		print("try");
		return;
	} finally {
		print("finally");
	}
}

f();
