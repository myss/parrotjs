function test(a, b) {
	if (a == b)
		print(a + " == " + b);
	if (a != b)
		print(a + " != " + b);
}
test(true, true);
test(false, false);
test(true, false);
test(false, true);
