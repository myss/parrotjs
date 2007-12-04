
var x = "hello";
print(x);
delete x;
print(this.x);

var o = {};
o.y = 7;
print(o.y);

delete o.y;
print(o.y);

o['z'] = 9;
print(o['z']);
delete o['z'];
print(o['z']);

