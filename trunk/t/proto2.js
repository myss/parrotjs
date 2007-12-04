function C() {}

C.prototype = {};

C.prototype.f = function(p) {
	print('this =', this);
    print('this.x =', this.x);
};

var o = new C();
o.x = 12;
o.f();

// the ordering of i's may change from
// implementation to implementation
for(var i in o) {
	if (i == 'f')
		print('has f');
}

print("o.x =", o.x);

