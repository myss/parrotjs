
x = 3;
y = ++x + 3;
print(x); // 4
print(y); // 7

a = ++x * y--;
print("x =", x); // 5
print("y =", y); // 6
print("a =", a); // 35

print(1 + 10/2*3 - (5 % 3)); // 14

if (x < 10) print("x < 10"); else print("NOT(x<10) with x =", x);
if (y < 2)  print("y < 2");  
if (y >= 6) print("y >= 6"); 
if (y < 2 || y >= 6) print("y < 2 || y >= 6");

if (x < 10 && (y < 2 || y >= 6)) {
	print("x < 10 && (y < 2 || y >= 6)");
}
