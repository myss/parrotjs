function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

var o = new Object();

is_in("test",  o);
is_in(o,  "test");

o.toString = function() { return "toString"; };
is_in("test",  o);
is_in(o,  "test");
