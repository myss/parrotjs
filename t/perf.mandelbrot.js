/*  Ported from:
        http://shootout.alioth.debian.org/, 
        Mandelbrot, java, Stefan Krause.
    
    Does only work for pjs (spidermonkey can't print strings containing "\0" properly),
    but spidermonkey can be used to compare speed.
*/


var limitSquared = 4.0;
var iterations = 50;

function mandelbrot(size) {
    this.size = size;
    this.fac = 2.0 / size;
    var offset = size % 8;
    this.shift = offset == 0 ? 0 : (8-offset);
}

mandelbrot.prototype.compute = function() {
    var result = "P4\n" + this.size + " " + this.size + "\n";
    for (var y = 0; y<this.size; y++) {
        var bits = 0;

        for (var x = 0; x<this.size; x++) {
            var Zr = 0.0;
            var Zi = 0.0;
            var Cr = (x*this.fac - 1.5);
            var Ci = (y*this.fac - 1.0);
        
            var i = iterations;
            var ZrN = 0;
            var ZiN = 0;
            do {
                Zi = 2.0 * Zr * Zi + Ci;
                Zr = ZrN - ZiN + Cr;
                ZiN = Zi * Zi;
                ZrN = Zr * Zr;
            } while (!(ZiN + ZrN > limitSquared) && --i > 0);
        
            bits = bits << 1;
            if (i == 0) bits++;
        
            if (x%8 == 7) {
                result += String.fromCharCode(bits);
                bits = 0;
            }
        }
        if (this.shift!=0) {
            bits = bits << this.shift;
            result += String.fromCharCode(bits);
        }
    }
    print(result);
};

var size = 100;
var m = new mandelbrot(size);
m.compute();