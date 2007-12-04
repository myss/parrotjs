// The scope is a .PjsObject, so it will be
// slower. See perf.loop_2.js for a .Hash scope.


function double_(n) {
	return n * 2;
}

i = 0;
while(i++ < 100000) {
	double_(i);
}
