function test(testFunc) {
    var obj;
    
    obj = {x: 0};
    testFunc(obj);
    
    obj = {x: 1};
    testFunc(obj);
    
    obj = {x: 0.5};
    testFunc(obj);
    
    obj = {x: -8.4};
    testFunc(obj);
    
    obj = {x: 989898};
    testFunc(obj);
    
    obj = {x: undefined};
    testFunc(obj);
    
    obj = {x: null};
    testFunc(obj);
    
    obj = {x: NaN};
    testFunc(obj);
    
    obj = {x: Infinity};
    testFunc(obj);
    
    obj = {x: -Infinity};
    testFunc(obj);
    
    obj = {x: "testFunc"};
    testFunc(obj);
    
    obj = {x: "8"};
    testFunc(obj);
    
    obj = {x: true};
    testFunc(obj);
    
    obj = {x: false};
    testFunc(obj);
    
    obj = {x: {}};
    testFunc(obj);
    
    obj = {x: new Object};
    testFunc(obj);
}

function testPreInc(obj) {
    print(obj.x);
    print(++obj.x);
    print(obj.x);
}

function testPostInc(obj) {
    print(obj.x);
    print(obj.x++);
    print(obj.x);
}

function testPreDec(obj) {
    print(obj.x);
    print(--obj.x);
    print(obj.x);
}

function testPostDec(obj) {
    print(obj.x);
    print(obj.x--);
    print(obj.x);
}

test(testPreInc);
test(testPreDec);
test(testPostInc);
test(testPostDec);
