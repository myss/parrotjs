var x = 10;

var obj = {
	a: 10,
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

for (var i in obj) {
	print("obj[",i,"] =", obj[i]);
}


for (var i in obj.e) {
    print("obj.e[",i,"] =", obj.e[i]);
}
