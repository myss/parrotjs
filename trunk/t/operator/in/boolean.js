function is_in(x, y) {
    try {
        print(x  in  y);
    } catch (e) {
        print("Err:", x, y);
    }
}

function testWith(bool) {
    is_in(bool,  bool);
    is_in(bool,  false);
    is_in(false,  bool);
    is_in(false,  false);
    
    is_in(bool,  "24");
    is_in("86",  bool);
    
    is_in(bool,  4);
    is_in(4,  bool);
    
    is_in(null,  bool);
    is_in(bool,  null);
    
}

is_in(true,  true);
is_in(true,  false);
is_in(false,  true);
is_in(false,  false);

testWith(true);
testWith(false);

