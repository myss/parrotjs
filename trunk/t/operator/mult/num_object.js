
var o = new Object();

print(3 * o);
print(o * 3);

o.toString = function() { return "toString"; };
print(3 * o);
print(o * 3);
