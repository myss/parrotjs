
function A() {}

A.prototype = {};
A.prototype.x = 10;

print(A.prototype.x);

var obj = new A;
print(obj.x);

obj.x = 3;
print(obj.x);
print(A.prototype.x);
