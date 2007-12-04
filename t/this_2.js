var x = 'global';

function f() {
	print("f: " + this.x);
}

var obj = {
	x : 'local',
	g : function() {
		f();
		(function() {
			print("lambda: " + this.x);
		})();
	}
};

obj.g();
