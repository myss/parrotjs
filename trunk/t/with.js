
var x = 1;

var obj1  = {x: 10};
var obj2  = {x: 100};
var empty = {};

print(x);
with(obj1) {
	print(x);
}

with(obj1) {
	with(obj2) {
		print(x);
		with (empty) {
			print(x);
		}
	}
	print(x);
}

with(empty) 
	with(empty) 
		print(x);

with(empty) 
	with(obj1) 
		print(x);
