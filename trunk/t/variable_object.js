var obj = {};

with(obj) {
	eval("var x = 3");
}
print(obj.x);
print(x);

//////////////

function f() {
	var obj2 = {};
	
	with(obj2) {
		eval("var y = 3");
	}
	print(obj.y);
	print(y);
}
f();

///////////////

try {
	throw "test";
} catch(e) {
	eval("var t = 10");
}
print(t);
