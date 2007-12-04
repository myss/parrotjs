
var o = new Object();

print(o in o);
print(3 in o);
print('test' in o);
print(2.7 in o);

o.x = 'test';
print('x' in o);
print('test' in o);
var x = 'x';
print(x in o);

print(null in o);
print(undefined in o);

o = {1: 'test', 'y': undefined};
print(1 in o);
print('1' in o);
print(new Number(1) in o);
print('y' in o);
print('z' in o);
