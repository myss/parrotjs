// pjs does not do the wrong thing, I think. The next is a 
// function expression, so it's the same as:
// if(true) { 
//		function() { 
//			print("called"); 
//		}
//	}
// see ECMA v3, section 13.

if(true) {
	function f() {
		print("f called");
	}
}

f();
