
var f = 9.5;
var f2 = 9.501;

var str = "test";
var str2 = "xyz";

var sf = "9.5";

print(9.5 || 9.5);
print(9.5 || 9.501);

print("test" || "test");
print("test" || "abc");

print("9.5" || 9.5);
print(9.5 || 9.5);
print("9.5" || 8.6);
print("8.6" || 9.5);

print(new String("test") || "test");
print("test" || new String("test"));
print(new String("test") || "abc");
print("abc" || new String("test"));

print(new String("3.5") || 3.5);
print(3.5 || new String("3.5"));
print(new String("3.5") || 7.0);
print(3.5 || new String("7.0"));

print(undefined || null);
print(null || undefined);
print(null || null);
print(undefined || undefined);

var x = new Object();
var y = new Object();

print(x || x);
print(x || y);
print(Object.prototype || Object.prototype);

print("test" || undefined);
print("test" || null);

