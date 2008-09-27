// We define __pjs__ so we can differentiate pjs with other
// implementations (needed for testing).
__pjs__ = 'PJS';

var undefined;
/**PIR
    .local pmc theUndefined
    theUndefined = new 'PjsUndefined'
    pjs_store_lex theUndefined, @env_0, 'undefined'
END*/

/**PIR
    .local pmc theNull
    theNull = new 'PjsNull'
    pjs_new_lex   @env_0, 'null'
    pjs_store_lex theNull, @env_0, 'null'
END*/

var __objectPrototype__;
/**PIR
    .local pmc objectProto
    objectProto = new 'PjsObject'
    pjs_store_lex objectProto, @env_0, '__objectPrototype__'
    @this.'setProto'(objectProto)
    set_hll_global '__objectPrototype__', objectProto
END*/
__objectPrototype__.__class__ = 'Object';

var __functionPrototype__;
/**PIR
    .local pmc functionProto
    functionProto = new 'PjsFunction'
    pjs_store_lex functionProto, @env_0, '__functionPrototype__'
    functionProto.'setProto'(objectProto)
END*/
__functionPrototype__.__class__ = 'Function';

var __arrayPrototype__;
/**PIR
    .local pmc arrayProto
    arrayProto = new 'PjsArray'
    pjs_store_lex arrayProto, @env_0, '__arrayPrototype__'
    arrayProto.'setProto'(objectProto)
END*/
__arrayPrototype__.__class__ = 'Array';


var Object;
/**PIR
    .local pmc ObjectConstructor
    ObjectConstructor = new 'PjsFunction'
    pjs_store_lex ObjectConstructor, @env_0, 'Object'
    ObjectConstructor.'setProto'(functionProto)
    
    # give it the subroutine for Object constructor
    .const .Sub __object_constructor__ = '__object_constructor__'
    ObjectConstructor.set_pir_sub(__object_constructor__)
    ObjectConstructor.set_outer_env(@env_0)
END*/
Object.length = 1;
Object.prototype = __objectPrototype__;
Object.prototype.constructor = Object;

var Function;
/**PIR
    .local pmc FunctionConstructor
    FunctionConstructor = new 'PjsFunction'
    pjs_store_lex FunctionConstructor, @env_0, 'Function'
    FunctionConstructor.'setProto'(functionProto)
    
    # give it the subroutine for Function constructor
    .const .Sub __function_constructor__ = '__function_constructor__'
    FunctionConstructor.set_pir_sub(__function_constructor__)
    FunctionConstructor.set_outer_env(@env_0)
END*/
Function.length = 1;
Function.prototype = __functionPrototype__;
Function.prototype.constructor = Function;
Function.prototype.call = function(thisVal) {
    if (thisVal == undefined) {
        /**PIR
            par@thisVal = pjs_get_global @env_0
            pjs_store_lex par@thisVal, @env_0, 'thisVal'
        END*/
    } else {
        thisVal = Object(thisVal);
        /**PIR
            par@thisVal = pjs_find_lex @env_0, 'thisVal'
        END*/
    }
    /**PIR
        #TODO do tailcall when fixed
        $P0 = pjs_call_function(par@thisVal, @this, @env_0, @rest :flat)
        .return ($P0)
    END*/
}
Function.prototype.apply = function(thisVal, args) {
    if (thisVal == undefined) {
        /**PIR
            par@thisVal = pjs_get_global @env_0
            pjs_store_lex par@thisVal, @env_0, 'thisVal'
        END*/
    } else {
        thisVal = Object(thisVal);
        /**PIR
            par@thisVal = pjs_find_lex @env_0, 'thisVal'
        END*/
    }
    if (args == undefined)
        args = Array();
        
    var len = args.length;
    
    if ((typeof len) != 'number') {
        // TODO use a real Exception object
        throw 'TypeError: second argument to Function.prototype.apply must be an array';
    }
    /**PIR
        .local pmc args_pmc
        args_pmc = new 'ResizablePMCArray'
    END*/
    for (var i = 0; i < len; i++) {
        var elem = args[i];
        /**PIR
            .local pmc elem
            elem = pjs_find_lex @env_0, 'elem'
            push args_pmc, elem
        END*/
    }
    /**PIR
        #TODO do tailcall when fixed
        $P0 = pjs_call_function(par@thisVal, @this, @env_0, args_pmc :flat)
        .return ($P0)
    END*/
}
Function.prototype.__has_instance__ = function(obj) {
    /**PIR
        $I0 = isa par@obj, 'PjsObject'
        if $I0 goto next
        $P0 = new 'PjsBoolean'
        $P0 = 0
        .return ($P0)        
      next:
    END*/

    var func_proto = this.prototype;
    if (func_proto == null)
        throw "TypeError"; // TODO use an Error object

    do {
        // obj = obj.__proto__
        /**PIR
            par@obj = par@obj.'getProto'()
            pjs_store_lex par@obj, @env_0, 'obj'
        END*/

        if (obj == func_proto)
            return true;

    } while (obj != null);
    
    return false;
}

Object.prototype.hasOwnProperty = function(propName) {
    /**PIR
        $S0 = par@propName
        $I0 = @this.'hasOwnProperty'($S0)
        $P0 = new 'PjsBoolean'
        $P0 = $I0
        .return ($P0)
    END*/
};
Object.prototype.isPrototypeOf = function(obj) {
    if (obj == null || (typeof obj) != 'object')
        return false;
    return this == obj.__proto__;
};

var __setReadonlyAttr__ = function(obj, name, value) {
    /**PIR
        $I0 = par@obj[par@name]         # old flags
        par@obj[par@name] = 0           # set writable
        par@obj[par@name] = par@value   # write
        par@obj[par@name] = $I0         # restore old flags
    END*/
};

var Array;
/**PIR
    .local pmc ArrayConstructor
    ArrayConstructor = new 'PjsFunction'
    pjs_store_lex ArrayConstructor, @env_0, 'Array'
    ArrayConstructor.'setProto'(functionProto)
    
    # give it the subroutine for Array constructor
    .const .Sub __array_constructor__ = '__array_constructor__'
    ArrayConstructor.set_pir_sub(__array_constructor__)
    ArrayConstructor.set_outer_env(@env_0)
END*/
Array.length = 1;
Array.prototype = __arrayPrototype__;
Array.prototype.constructor = Array;

__objectPrototype__.toString = function() {
    /**PIR
        $S0 = '[object '
        $P0 = @this['__class__']
        $S1 = $P0
        concat $S0, $S1
        concat $S0, ']'
        $P0 = new 'PjsString'
        $P0 = $S0
        .return ($P0)
    END*/
};
__objectPrototype__.valueOf = function() {
    return this;
};

__arrayPrototype__.toString = function() {
    var len = this.length;
    if (len == 0)
        return '';
    
    var s = '';
    if (this[0] != undefined)
        s += this[0];
    
    for(var i = 1; i < len; i++) {
        s += ',';
        if (this[i] != undefined)
            s += this[i];
    }
    return s;
};
__arrayPrototype__.join = function(delim) {
    if (delim === undefined)
        delim = ',';

    var len = this.length;
    if (len == 0)
        return '';
    
    var s = '';
    if (this[0] != undefined)
        s += this[0];
    
    for(var i = 1; i < len; i++) {
        s += delim;
        if (this[i] != undefined)
            s += this[i];
    }
    return s;
};
__arrayPrototype__.reverse = function() {
    var i = 0, j = this.length-1;
    while (i < j) {
        var temp = this[i];
        this[i] = this[j];
        this[j] = temp;
        i++;
        j--;
    }
    return this;
};
__arrayPrototype__.push = function() {
    var argLen = arguments.length;
    for (var i = 0; i < argLen; i++)
        this[this.length] = arguments[i];
    return this.length;
};
__setReadonlyAttr__(__arrayPrototype__.push, 'length', 1);

__arrayPrototype__.shift = function() {
    var len = this.length;
    if (len == 0)
        return undefined;
    var shifted = this[0];
    for (var i=1; i < len; i++) {
        if (i in this)
            this[i-1] = this[i];
        else
            delete this[i-1];
    }
    x.length = len - 1;
    return shifted;
};
__arrayPrototype__.unshift = function(elem) {
    var len = this.length;
    this[len] = null;
    for (var i=len-1; i >= 0; i--) {
        if (i in this)
            this[i+1] = this[i];
        else
            delete this[i+1];
    }
    this[0] = elem;
};
__arrayPrototype__.slice = function(start, end) {
    if (end == undefined)
        end = this.length;
    if (start < 0)
        start += this.length;
    if (end < 0)
        end += this.length;
    var result = [];
    for (var i = start; i < end; i++)
        result[result.length] = this[i];
    return result;
};
__arrayPrototype__.splice = function(start, deleteCount) {
    var copy = this.slice(0);
    this.length = 0;
    for (var i = 0; i < start; i++)
        this[i] = copy[i];
    for (var i = 2; i < arguments.length; i++)
        this[this.length] = arguments[i];
    for (var i = start + deleteCount; i < copy.length; i++)
        this[this.length] = copy[i];
    var result = [];
    for (var i = start; i < start + deleteCount && i < copy.length; i++)
        result[result.length] = copy[i];
    return result;
};
__arrayPrototype__.concat = function() {
    var result = this.slice(0);
    for (var i = 0; i < arguments.length; i++) {
        var arr = arguments[i];
        for (var j = 0; j < arr.length; j++)
            result[result.length] = arr[j];
    }
    return result;
};
__setReadonlyAttr__(__arrayPrototype__.concat, 'length', 1);


var __sort_compare__ = function(a, b, cmp) {
    if (x === undefined && y === undefined)
        return +0;
    if (x === undefined)
        return 1;
    if (y === undefined)
        return -1;
    return cmp(x, y);
};
__sort_default_compare__ = function(a, b) {
    return (a < b) ?  -1 : 
                      (a > b) ? 1 : 0;
}
__sort_default_compare__ = function( x, y ) {
    if ( x == void 0 && y == void 0  && typeof x == "undefined" && typeof y == "undefined" ) {
        return +0;
    }
    if ( x == void 0  && typeof x == "undefined" ) {
        return 1;
    }
    if ( y == void 0 && typeof y == "undefined" ) {
        return -1;
    }
    x = String(x);
    y = String(y);
    if ( x < y ) {
        return -1;
    }
    if ( x > y ) {
        return 1;
    }
    return 0;
}
__arrayPrototype__.sort = function(cmp) {
    if (typeof cmp != 'function')
        cmp = __sort_default_compare__;
    
    var len = this.length;
    
    // selection sort
    for (var i = 0; i < len-1; i++) {
        var min = i;
        for (var j = i; j < len; j++) {
            if (this[min] === undefined || 
                    (this[j] !== undefined && cmp(this[j], this[min]) < 0)) {
                min = j;
            }
        }
        var temp  = this[i];
        this[i]   = this[min];
        this[min] = temp;
    }
    return this;
};

//////////////////////////////
//// PROPERTY ATTRIBUTES /////
//////////////////////////////

var __dontEnum__ = function(obj, propname) { 
    /**PIR
        .local int i
        i = par@obj[par@propname]
        i = i | 2
        par@obj[par@propname] = i
    END*/
};
var __dontChange__ = function(obj, propname) {
    /**PIR
        .local int i
        i = par@obj[par@propname]
        i = i | 1
        par@obj[par@propname] = i
    END*/
};
var __dontDelete__ = function(obj, propname) { 
    /**PIR
        .local int i
        i = par@obj[par@propname]
        i = i | 4
        par@obj[par@propname] = i
    END*/
};
var setFlags = function(obj, propname, flags) {
    /**PIR
        .local int i
        i = par@flags
        par@obj[par@propname] = i
    END*/
};
var getFlags = function(obj, propname) {
    /**PIR
        .local int i
        i = par@obj[par@propname]
        $P0 = new 'PjsNumber'
        $P0 = i
        .return ($P0)
    END*/
};

/////////////////////
//// WRAPPERS /////
/////////////////////

var Boolean = function(b) {
    /**PIR
        .local int result_bool
        result_bool = 0
        
        unless has@b goto end
        result_bool = istrue par@b
        
      end:
        
        .local pmc result
        result = new 'PjsBoolean'
        result = result_bool
        @this['__value__'] = result
        
    END*/
    
    __dontEnum__    (this, '__value__');
    __dontDelete__  (this, '__value__');
    return this.__value__;
};
Boolean.prototype.__value__  = false;
__dontDelete__  (Boolean.prototype, '__value__');

Boolean.prototype.toString = function() {
    return '' + this.__value__;
};
Boolean.prototype.valueOf = function() {
    return this.__value__;
};
Boolean.prototype.__class__ = 'Boolean';

var Number = function(b) {
    /**PIR
        .local num result_num
        result_num = 0
        
        unless has@b goto end
        result_num = par@b
        
      end:
        
        .local pmc result
        result = new 'PjsNumber'
        result = result_num
        @this['__value__'] = result
        
    END*/
    
    __dontEnum__    (this, '__value__');
    __dontDelete__  (this, '__value__');
    return this.__value__;
};
Number.prototype.__value__  = 0;
__dontDelete__  (Number.prototype, '__value__');

Number.prototype.toString = function() {
    return '' + this.__value__;
};
Number.prototype.valueOf = function() {
    return this.__value__;
};
Number.prototype.__class__ = 'Number';
// special numbers
var NaN, Infinity;
/**PIR
    .local pmc number
    number = new 'PjsNumber'
    number.'be_nan'()
    pjs_store_lex number, @env_0, 'NaN'
    number = new 'PjsNumber'
    number.'be_posinf'()
    pjs_store_lex number, @env_0, 'Infinity'
END*/
Number.NaN = NaN;
Number.POSITIVE_INFINITY =  Infinity;
Number.NEGATIVE_INFINITY = -Infinity;

// TODO these are not exact
Number.MAX_VALUE = 1.7976931348623157e+308;
Number.MIN_VALUE = 5e-324;
(function() {
    for (var i in Number) {
        __dontChange__(Number, i);
        __dontDelete__(Number, i);
    }
})();


var String = function(b) {
    /**PIR
        .local string result_str
        result_str = ''
    
        unless has@b goto end    
        result_str = par@b

      end:
        .local pmc result
        result = new 'PjsString'
        result = result_str
        @this['__value__'] = result
        
        $I0 = length result_str
        $P0 = new 'PjsNumber'
        $P0 = $I0
        @this['length'] = $P0
        
    END*/
    __dontEnum__    (this, '__value__');
    __dontDelete__  (this, '__value__');
    __dontEnum__    (this, 'length');
    __dontDelete__  (this, 'length');
    return this.__value__;
};
String.prototype.__value__  = '';
String.prototype.length     = 0;
__dontDelete__  (String.prototype, '__value__');
__dontDelete__  (String.prototype, 'length');

String.prototype.toString = function() {
    return this.__value__;
};
String.prototype.valueOf = function() {
    return this.__value__;
};
String.prototype.__class__ = 'String';


String.prototype.charAt = function(i) {
    /**PIR
        $S0 = @this
        $I0 = par@i
        $S1 = substr $S0, $I0, 1
        $P0 = new 'PjsString'
        $P0 = $S1
        .return ($P0)
    END*/
};
String.prototype.substring = function(i, j) {
    //var value = '' + this;
    if (!i || i < 0) i = 0;
    if (j != j) j = 0;
    
    if (j != 0 && !j) {
        /**PIR
            $P1 = pjs_find_lex @env_0, 'i'
            
            $S0 = @this
            $I1 = $P1
            
            $S1 = substr $S0, $I1
            $P0 = new 'PjsString'
            $P0 = $S1
            .return ($P0)
        END*/
    } else {
        if (j < 0) j = 0;
        if (i > j) {
            var temp = i;
            i = j;
            j = temp;
        }
        j = j - i;
        /**PIR
            $P1 = pjs_find_lex @env_0, 'i'
            $P2 = pjs_find_lex @env_0, 'j'
            
            $S0 = @this
            $I1 = $P1
            $I2 = $P2
            
            $S1 = substr $S0, $I1, $I2
            $P0 = new 'PjsString'
            $P0 = $S1
            .return ($P0)
        END*/
    }
};
String.fromCharCode = function(c) {
    /**PIR
        .local string result
        result = ''
        unless has@c goto end
        $I0 = par@c
        result = chr $I0
        
        .local int len, i
        len = @rest
        i = 0
        
      loop:
        unless i < len goto end
        $I0 = @rest[i]
        $S0 = chr $I0
        result .= $S0
        inc i
        goto loop

      end:
        $P0 = new 'PjsString'
        $P0 = result
        .return ($P0)
    END*/
};
String.prototype.charCodeAt = function(pos) {
    /**PIR
        .local int i, len
        i = 0
        
        unless has@pos goto next
        i = par@pos
        if i < 0 goto nan
        
      next:
        $S0 = @this
        len = length $S0
        if i >= len goto nan
        
        $I0 = ord $S0, i
        $P0 = new 'PjsNumber'
        $P0 = $I0
        .return ($P0)
        
      nan:
        $P0 = new 'PjsNumber'
        $P0.'be_nan'()
        .return ($P0)
    END*/
}


this.__class__ = 'global';

///////////////////////////////
//// SOME USEFUL FUNCTIONS/////
///////////////////////////////

var print = function() {
    /**PIR
        .local pmc it, elem
        it = new 'Iterator', @rest
        unless it goto end
        elem = shift it
        print elem
    loop:
        unless it goto end
        elem = shift it
        print ' '
        print elem
        goto loop
    end:
        print "\n"
    END*/
};
var eval = function(str) {
    if (str === undefined)
        return undefined;
    if (typeof str != 'string')
        return str;
    /**PIR
        $S0 = par@str
        $S0 = pjs_compile $S0, 0
        #.return eval_js_pir($S0, @dyn_env)
        $P0 = eval_js_pir($S0, @dyn_env)
        .return ($P0)
    END*/
};
var isNaN = function(val) {
    /**PIR
        $N0 = par@val
        par@val = new 'PjsNumber'
        par@val = $N0
        pjs_store_lex par@val, @env_0, 'val'
    END*/
    return val != val;
};
var isFinite = function(val) {
    /**PIR
        $N0 = par@val
        par@val = new 'PjsNumber'
        par@val = $N0
        pjs_store_lex par@val, @env_0, 'val'
    END*/
    return val != Infinity && val != -Infinity && val == val;
};

var import = function(path) {
    /**PIR
        .local string path
        .local int len, i
        path = par@path
        # for the moment, load_bytecode fails with non-ascii paths
        $I0 = find_charset 'ascii'
        trans_charset path, $I0
        
        
        .local string dir, file_name, curr_dir
        .local pmc os
        os = new 'OS'
        (dir, file_name) = 'split_directory_file'(path)
        curr_dir = os.'cwd'()
        os.'chdir'(dir)
        
        
        len = length path
        i = len - 3   # len - strlen('.js')
        i = index path, '.js', i
        if i < 0 goto byte_code
        
      js_code:
        .local string code
        code = 'read_file'(file_name)
        code = pjs_compile code, 1
        
        .local pmc comp, execute
        comp = compreg 'PIR'
        execute = comp(code)
        execute()
        goto end
        
      byte_code:
        load_bytecode file_name
      end:
      
        os.'chdir'(curr_dir)
    END*/
};

var Math = {};
Math.abs = function(f) {
    /**PIR
        $N0 = par@f
        abs $N0
        .return ($N0)
    END*/
};
Math.acos = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = acos $N0
        .return ($N0)
    END*/
};
Math.asin = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = asin $N0
        .return ($N0)
    END*/
};
Math.atan = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = atan $N0
        .return ($N0)
    END*/
};
Math.atan2 = function(y, x) {
    /**PIR
        $N0 = par@y
        $N1 = par@x
        $N0 = atan $N0, $N1
        .return ($N0)
    END*/
};
Math.ceil = function(f) {
    /**PIR
        $N0 = par@f
        ceil $N0
        .return ($N0)
    END*/
};
Math.cos = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = cos $N0
        .return ($N0)
    END*/
};
Math.exp = function(e) {
    /**PIR
        $N0 = par@e
        $N0 = exp $N0
        .return ($N0)
    END*/
};
Math.floor = function(f) {
    /**PIR
        $N0 = par@f
        floor $N0
        .return ($N0)
    END*/
};
Math.log = function(e) {
    /**PIR
        $N0 = par@e
        $N0 = ln $N0
        .return ($N0)
    END*/
};
Math.max = function() {
    var args = arguments;
    var len  = args.length;
    var max = -Infinity;
    for (var i=0; i < len; i++)
    {
        var el = args[i];
        if (el != el) // NaN test
            return el;
        if (max < args[i])
            max = args[i];
    }
    return max;
};
__setReadonlyAttr__(Math.max, 'length', 2);
Math.min = function() {
    var args = arguments;
    var len  = args.length;
    var min = +Infinity;
    for (var i=0; i < len; i++)
    {
        var el = args[i];
        if (el != el) // NaN test
            return el;
        if (min > args[i])
            min = args[i];
    }
    return min;
};
__setReadonlyAttr__(Math.min, 'length', 2);
Math.pow = function(base, e) {
    /**PIR
        $N0 = par@base
        $N1 = par@e
        $N0 = pow $N0, $N1
        .return ($N0)
    END*/
};
Math.random = function() {
    throw "Math.random() not implemented";
}
Math.round = function(f) {
    var ceiled = Math.ceil(f);
    if (ceiled - f <= 0.5)
        return ceiled;
    else
        return ceiled - 1;
}
Math.sin = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = sin $N0
        .return ($N0)
    END*/
};
Math.sqrt = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = sqrt $N0
        .return ($N0)
    END*/
};
Math.tan = function(f) {
    /**PIR
        $N0 = par@f
        $N0 = tan $N0
        .return ($N0)
    END*/
};
Math.E = Math.exp(1);
Math.LN10 = Math.log(10);
Math.LN2 = Math.log(2);
Math.LOG2E = 1.0 / Math.LN2;
Math.LOG10E = 1.0 / Math.LN10;
Math.PI = Math.acos(-1);
Math.SQRT1_2 = Math.sqrt(0.5);
Math.SQRT2 = Math.sqrt(2);
(function() {
    for (var i in Math) {
        __dontChange__(Math, i);
        __dontDelete__(Math, i);
    }
})();

var __get_proto__ = function(o) {
    /**PIR
        .local pmc proto
        proto = par@o.'getProto'()
        say proto
        .return (proto)
    END*/
};

var __set_proto__ = function(o, proto) {
    /**PIR
        par@o.'setProto'(par@proto)
    END*/
};


// find the scope from an environment
var __activation_object__ = function(env) {
    /**PIR
        $P0 = par@env[0]
        .return ($P0)
    END*/
};
// get an upper scope object
var __up__ = function(n) {
    if (! n) 
        n = 1;
    var ao = __activation_object__(__upper__);
    for (var i = 0; i < n && ao; i++)
        ao = __activation_object__(ao.__upper__);
    return ao;
};

////////////////////////////////////////////////
//// CHANGE PROPERTY ATTRIBUTES OF OBJECTS /////
////////////////////////////////////////////////

__dontChange__  (Object, 'prototype');
__dontDelete__  (Object, 'prototype');
__dontChange__  (Object, 'length');
__dontDelete__  (Object, 'length');

__dontChange__  (Function, 'prototype');
__dontDelete__  (Function, 'prototype');
__dontChange__  (Function, 'length');
__dontDelete__  (Function, 'length');

__dontChange__  (Array, 'prototype');
__dontDelete__  (Array, 'prototype');
__dontChange__  (Array, 'length');
__dontDelete__  (Array, 'length');


__dontChange__  (String, 'prototype');
__dontDelete__  (String, 'prototype');

__dontChange__  (Boolean, 'prototype');
__dontDelete__  (Boolean, 'prototype');

__dontChange__  (Number, 'prototype');
__dontDelete__  (Number, 'prototype');


// don't enumerate anything
(function() {
    for (var i in this) {
        __dontEnum__(this, i);
        
        if (this[i] != null && (typeof this[i]) == 'object') {
            for (j in this[i]) {
                __dontEnum__(this[i], j);
            }
        }
        else if ((typeof this[i]) == 'function') {
        
            for (var j in this[i]) {
                __dontEnum__(this[i], j);
            }
                
            if (this[i].prototype) {
                for (var j in this[i].prototype) {
                    __dontEnum__(this[i].prototype, j);
                }
            }
        }
    }
    __dontChange__  (this, 'null');
    __dontChange__  (this, 'undefined');
    __dontDelete__  (this, 'null');
    __dontDelete__  (this, 'undefined');
})();

