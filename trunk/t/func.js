
function fact1(n) {
	if (n==0)
		return 1;
	else
		return n * fact1(n-1);
}

var f2 = function fact(n) {
    if (n==0)
        return 1;
    else
        return n * fact(n-1);
};

print(fact1(5));
print(f2(7));


if (this.fact) 
	print('this.fact exists');
else 
	print('this.fact does not exist');

if (this.fact1)
    print('this.fact1 exists');
else
    print('this.fact1 does not exist');
