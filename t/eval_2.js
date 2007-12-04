
function f(s) {
	return eval(s);
}

f("print(s)");

var x = f("function() { if(typeof(s)=='number') return ++s; else return s=0; }");
print(x());
print(x());
print(x());
