function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

var o = new Object();

is_in(3,  o);
is_in(o,  3);

o.toString = function() { return "toString"; };
is_in(3,  o);
is_in(o,  3);
