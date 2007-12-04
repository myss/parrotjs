
function p(x) {
	print(x);
	return x;
}

if (p(2) == 2 || p(5) == 5) {
	print("should only print 2");
}

if (! (p(7) == 0 && p(9) == 9)) {
    print("should only print 7");
}

