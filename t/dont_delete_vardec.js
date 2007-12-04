
var x = 13;
print(delete x);

this.z = 16;
print(delete y);

z = 19;
print(delete z);

function a(){}
print(delete a);

var b = function(){};
print(delete b);

c = function(){};
print(delete c);

var s = 13;
print(delete this.s);

for(var i=0; false; ){}
print(delete i);


function test() {
	var r = "test";
	print(delete r);
}
test();
