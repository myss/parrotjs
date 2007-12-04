/* tail call optimization needed */

function add(a, b) {
	//print("a =", a, ", b =", b);
	//print("OK");
	if (b <= 0) {
		return a;
	} else {
		return add(a+1, b-1);
	}
}

var n = 600; //998; //600;//997;
print(add(1234, n));
