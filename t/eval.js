
eval("print('simple eval')");

eval("x = 10");
print(x);

eval("this.a = 7");
print("a =", a);

var b = eval("3");
print("b =", b);

var obj = {x:10};
eval("with(obj) { x = 19; var y = 6; z = 3 }");
print("obj.x =", obj.x);
print("obj.y =", obj.y);
print("z =", z);

eval('eval("function f() {print(10); return(15); }")');
print(f());

g = eval('function r(){ print("89"); }');
//g();

r();


var X = 'global';
function func() {
	var X = 'local';
	eval('print(X)');
}

func();
