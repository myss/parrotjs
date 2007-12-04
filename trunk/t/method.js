
function X() {}
X.prototype.toString = function() {
	return "test";
};

a = new X;
print(a);

a.toString = Object.prototype.toString;

print(Object.prototype.toString());


print(a.toString());
print(a);
a.toString = function() { return 3; };
print(a);

