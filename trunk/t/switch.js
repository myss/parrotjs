print('start');

var x = 10;

switch(x) {
	case 3: print(3);
	case 10: print(10);
	case 8: print(8);
	case 10: print('second', 10);
	case "abc": print("abc");
	default: print('default');
}

print('end');

//////////////////


var x = 20;
switch(x) {
	case 10:
	case 20:
	case 30: print(30); break;
	case NaN: print(NaN); break;
	case NaN: print(NaN);
	case 5: print(5); break;
}
switch(x) {
    case 10:
    case 20: {}
    case 30: print(30); 
    case NaN: print(NaN); 
    case NaN: print(NaN);
    case 5: print(5); break;
    default: print("default"); break;
}

label: {
    print("start label");
    switch(x) {
        case 10:
        case 20: {}
        case 30: print(30); 
        case NaN: print(NaN); 
        case NaN: print(NaN);
        case 5: print(5); break label;
        default: print("default"); break;
    }
    print("end label");
}