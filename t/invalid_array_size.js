function test(arr_len) {
	try {
		new Array(arr_len);
	} catch(e) {
		print("error with len " + arr_len);
	}
}
function test2(arr_len) {
	var a = new Array;
	try {
        a.length = arr_len;
    } catch(e) {
        print("error2 with len " + arr_len);
    }
}

test(NaN);
test(Infinity);
test(-1);
test(-Infinity);

test2(NaN);
test2(Infinity);
test2(-1);
test2(-Infinity);

print("THE END");
