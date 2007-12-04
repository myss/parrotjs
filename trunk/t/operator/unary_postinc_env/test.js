function test(x) {
	print("--------");
	print(x);
    print(x++);
    print(x);
}

test(0);
test(1);
test(0.7);
test(-8.4);
test(989898);
test(undefined);
test(null);
test(NaN);
test(Infinity);
test(-Infinity);
test("test");
test("8");
test(true);
test(false);
test({});
test(new Object);

print(Object++);
