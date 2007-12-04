var x = 10;

var obj = {
	"a": 10,
	b: "hello",
	c: x,
	d: obj,
	e: {
		p: 3,
		q: x,
		r: obj,
		s: obj
	}
};

print("obj.a =", obj.a);
print("obj.b =", obj.b);
print("obj.c =", obj.c);
print("obj.d =", obj.d);
print("obj.e =", obj.e);
print("obj.e.p =", obj.e.p);
print("obj.e.q =", obj.e.q);
print("obj.e.r =", obj.e.r);
print("obj.e.s =", obj.e.s);

