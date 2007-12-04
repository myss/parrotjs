
function t(a, b) {
	print(a, "===", b, "=", a===b);
}

t(10, 5);
t(-10, 5);
t(10, -5);
t(-10, -5);

t(10, 4);
t(-10, 4);
t(10, -4);
t(-10, -4);

t(10.3, 4.7);
t(-10.3, 4.7);
t(10.3, -4.7);
t(-10.3, -4.7);

t(NaN, NaN);
t(Infinity, NaN);
t(Infinity, Infinity);
t(-Infinity, -Infinity);
t(3.7, NaN);
t(NaN, 3.7);
t(2.3, Infinity);
t(2.3, -Infinity);
t(-7.8, Infinity);
t(-7.8, -Infinity);

t(10, 0);
t(0, 2);
t(0, NaN);
t(NaN, 0);
t(Infinity, 0);
t(0, Infinity);
t(-Infinity, 0);
t(0, -Infinity);
