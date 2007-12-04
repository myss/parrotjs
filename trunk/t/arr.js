var x = new Array();
x[3] = 7;
print(x[3]);
print(x.length);
print(typeof x);

x = new Array(1,"x",3);
print(x.length);
x["10"] = 'hello';
print(x.length);

x[new String('12')] = 'hh';
print(x.length);

//Array.prototype.toString = null;//function() { return "uf"; };

print(x.toString == null);
print(x);
x.length = 3;
print(x);

for(var a in x)
	print(a);


x['abc'] = 'def';
print(x['abc']);

Array.prototype[13] = 'test';
print(x[13]);


print([1,2,3][2]);
