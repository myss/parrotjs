// The scope is a .Hash, so it will be
// faster. See perf.loop.js for a .PjsObject scope.

(function wrapper() {

function double_(n) {
	return n * 2;
}

var i = 0;
while(i++ < 100000) {
	double_(i);
}

})();
