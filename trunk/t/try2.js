
function F() {
	try {
		print("Before throwing");
		throw "[I am the exception]";
		print("After throwing");
	} finally {
		print("finally in F");
	}
	print("end of F");
}

try {
try {
	print("before F");
	F();
	print("after F");
} catch(e) {
	print(e);
	try {
		throw "[Ex 2]";
	} finally {
		print("Finally");
	}
	print("I should not be printed");
}
} catch (e) {
	print("Caught:", e);
} finally {
	print("OK");
}
print("THE END");
