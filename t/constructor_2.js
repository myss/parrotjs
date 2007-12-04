
// If a function F returns an object, new F()
// returns that object too. Else, new F() returns
// a new object.

function A() {
	this.x = 1;
	return {x:10};
}
function B() {
	this.x = 2;
	return 7;
}
function C() {
	this.x = 3;
}

print(new A().x);
print(new B().x);
print(new C().x);
