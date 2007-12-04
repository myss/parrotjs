
this.x = 10;

with(new Object)
	delete x;
	
print(this.x);
