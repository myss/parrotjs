
var obj = {
	x : 10,
	f : function() { print(this.x); }
};
obj.f();

with(obj) f();
