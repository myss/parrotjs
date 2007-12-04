
var obj = {};

obj.x = 3;
print("obj.x =", obj.x);
print("obj['x'] =", obj['x']);

obj['y'] = 10;
print("obj.y =", obj.y);
print("obj['y'] =", obj['y']);

var z = 'z';
obj[z] = 30;
print("z =", z);
print("obj.z =", obj.z);
print("obj[z] =", obj[z]);
