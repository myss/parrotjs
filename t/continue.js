var n;

print("First loop:");

n = 1;
label:
for(var i=1; i<=7; i++) {

	// no infinite loop if the next continue fails
	n++; 
	if (n == 11) {
		print("First loop caused an infinite loop! i =", i);
		break;
	}
	
	if (i >= 3)
		continue label;
	print(i);
}

print("\nSecond loop:");

n = 1;
for(var i=1; i<=7; i++) {
    // no infinite loop if the next continue fails
    n++;
    if (n == 11) {
        print("Second loop caused an infinite loop! i =", i);
        break;
    }

    if (i >= 3)
        continue;
    print(i);
}

