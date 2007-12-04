function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

function testWith(bool) {
    is_in(undefined,  bool);
    is_in(bool,  undefined);
}

testWith(true);
testWith(false);