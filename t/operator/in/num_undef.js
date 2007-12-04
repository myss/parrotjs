function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

is_in(7,  undefined);
is_in(undefined,  7);
