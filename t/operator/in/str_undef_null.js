function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

is_in("hello",  undefined);
is_in(undefined,  "hello");

is_in("hello",  null);
is_in(null,  "hello");
