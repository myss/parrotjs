if ('__pjs__' in this) {
	// it is the pjs engine

	this.x = 10;
	print(this.x);

	setFlags(this, "x", 1);
	this.x = 20;
	print(this.x);

	if (getFlags(this, "x") == 1) {
		print("this['x']: ReadOnly");
		setFlags(this, "x", 0);
	}
	this.x = 30;
	print(this.x);
}
else {
	// it is not the pjs engine
	print(10);
	print(10);
	print("this['x']: ReadOnly");
	print(30);
}
