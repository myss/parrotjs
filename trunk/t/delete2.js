
x = {};
x.a = 10;
print(delete x.a);

print(delete x);

if('__pjs__' in this) {
	var o = {y:33};
	__dontDelete__(o, 'y');
	print(delete o.y);
	
	m = "test";
	__dontDelete__(this, 'm');
	print(delete m);
} else {
	print(false);
	print(false);
}
