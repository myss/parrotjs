function test(s) {
    print("'" + s + "'", "->", s - 0);
}

test("");
test("ABC");
test(" ABC");
test(" ABC ");
test("ABC ");
test(" ");
test("   ");
test("Infinity");
test("-Infinity");
test("+Infinity");
test("infinity");
test("NaN");
test("234.8");
test("  234.8");
test("234.8  ");
test("1");
test(" 1");
test(" 1 ");
test("1 ");
test(" 35e99 ");
test(" 33.78E88   ");
test(" 3.4e100000000");
test(" 786875.67465465 ");
test(" 786875.6746546587687575 ");
test(" 786875.674654658768757586875584733734563643 ");
test(" 786875674654658768757586875584733734563.643 ");
test(" 875765.786785f ");
test(" 077 ");
test(" 088 ");
test("  0xFF ");
