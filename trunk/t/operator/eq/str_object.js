
var o = new Object();

print("test" == o);
print(o == "test");

o.toString = function() { return "toString"; };
print("test" == o);
print(o == "test");
