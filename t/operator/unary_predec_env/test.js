function test(x) {
	print(x);
    print(--x);
    print(x);
}

test(0);
test(1);
test(0.5);
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

print(--Object);
