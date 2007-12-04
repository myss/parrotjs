function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

is_in(undefined,  null);
is_in(null,  undefined);
