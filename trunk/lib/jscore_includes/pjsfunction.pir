###########################
###### FUNCTION CALL ######
###########################

.namespace

.sub create_function
    .param pmc pir_sub
    .param pmc outer_env
    .param int n_params
    
    .local pmc js_func, n_params_pmc, prototype
    .local pmc object_prototype, function_prototype
    
    js_func = new 'PjsFunction', pir_sub
    js_func.set_outer_env(outer_env)
    
    n_params_pmc = new 'PjsNumber'
    n_params_pmc = n_params
    js_func['length'] = n_params_pmc
    'set_flags'(js_func, 'length', FLAG_READ_ONLY, FLAG_DONT_DELETE, FLAG_DONT_ENUM)
    
    $P0 = pjs_find_lex outer_env, 'Object'
    object_prototype = $P0['prototype']
    
    
    prototype = new 'PjsObject'
    prototype.'setProto'(object_prototype)
    js_func['prototype'] = prototype
    'set_flags'(js_func, 'prototype', FLAG_DONT_DELETE)
    
    $P0 = pjs_find_lex outer_env, 'Function'
    function_prototype = $P0['prototype']
    js_func.'setProto'(function_prototype)
    
    prototype['constructor'] = js_func
    'set_flags'(prototype, 'constructor', FLAG_DONT_ENUM)
    
    .return (js_func)
.end

.sub add_arguments :anon
    .param pmc params
    .param pmc called_func
    .param pmc called_func_env
    
    .local pmc arguments, object_prototype
    
    object_prototype = get_hll_global '__objectPrototype__'
    
    arguments = new 'PjsObject'
    arguments.'setProto'(object_prototype)
    
    arguments['callee'] = called_func
    
    .local int len, i
    len = elements params
    $P0 = new 'PjsNumber'
    $P0 = len
    arguments['length'] = $P0
    
    i = 0
  loop:
    unless i < len goto end_loop
    $P0 = params[i]
    arguments[i] = $P0
    inc i
    goto loop
  end_loop:
    pjs_new_lex_with_flags called_func_env, 'arguments', FLAG_DONT_ENUM
    pjs_store_lex arguments, called_func_env, 'arguments'
.end

# Calls the given function with given parameters.
# Uses the 'this' and 'dyn_env' parameters if applicable.
.sub pjs_call_function
    .param pmc this
    .param pmc function
    .param pmc dyn_env
    .param pmc params :slurpy

    # Check whether it's a PjsFunction.
    # If not, call 'function' as a general function, discarding 'this' and 'dyn_env'
    $I0 = isa function, 'PjsFunction'
    if $I0 goto is_PjsFunction
    $P0 = function(params :flat)
    .return ($P0)
    
  is_PjsFunction:
    .local pmc subroutine, enc_env, new_env

    enc_env = function.get_outer_env()
    new_env = pjs_new_subscope enc_env
    
    # the next can be interesting
    #pjs_new_lex new_env, '__upper__'
    #pjs_store_lex dyn_env, new_env, '__upper__'
    
    add_arguments(params, function, new_env)
    
    subroutine = function.get_pir_sub()
    
    .return subroutine(this, new_env, dyn_env, params :flat)
.end

# Invokes the given method for the given object.
.sub pjs_invoke_method
    .param pmc this
    .param string method_name
    .param pmc dyn_env
    .param pmc params :slurpy
    
    # Check whether it's a PjsFunction.
    # If not, call 'function' as a general function, discarding 'this' and 'dynEnv'
    $I0 = isa this, 'PjsObject'
    if $I0 goto is_PjsObject
    $P0 = this.method_name(params :flat)
    .return ($P0)
    
  is_PjsObject:
    .local pmc function, subroutine, enc_env, new_env

    function = this[method_name]
    $I0 = isa function, 'PjsFunction'
    if $I0 goto is_PjsFunction
    
    # the property is not a function
    $P0 = new 'Exception'
    $S0 = 'TypeError: '
    $S0 .= method_name
    $S0 .= ' is not a function'
    $P0['_message'] = $S0
    throw $P0
    
  is_PjsFunction:
    enc_env = function.get_outer_env()
    new_env = pjs_new_subscope enc_env
    
    # the next can be interesting
    #pjs_new_lex new_env, '__upper__'
    #pjs_store_lex dyn_env, new_env, '__upper__'
    
    add_arguments(params, function, new_env)
    
    subroutine = function.get_pir_sub()
    
    .return subroutine(this, new_env, dyn_env, params :flat)
.end

.sub pjs_invoke_new
    .param pmc function
    .param pmc dynEnv
    .param pmc params :slurpy

    .local pmc func, enc_env, new_env
    
    enc_env = function.get_outer_env()
    new_env = pjs_new_subscope enc_env
    
    # the next can be interesting
    #pjs_new_lex new_env, '__upper__'
    #pjs_store_lex dynEnv, new_env, '__upper__'
    
    func = function.get_pir_sub()
    
    add_arguments(params, function, new_env)

    .local pmc proto, newobject
    proto = function['prototype']
    
    newobject = new 'PjsObject'
    newobject.'setProto'(proto)
    
    .local pmc returnVal
    returnVal = func(newobject, new_env, dynEnv, params :flat)
    $I0 = isa returnVal, 'PjsObject'
    if $I0 goto wasObject
    .return (newobject)
  wasObject:
    .return (returnVal)
.end


# Overriding invoke in a pmclass (PjsFunction in this case) 
# is a pain (if possible) when you need to manipulate arguments and 
# then invoke some other subroutine. This sub makes it possible, but
# [TODO] it gets the PjsFunction from a global variable! (waiting
# for rt #42919 to be resolved)

.sub invoke_from_outside
    .param pmc this
    .param pmc params :slurpy

    .local pmc pjs_func, dyn_env#, this
    
    # TODO dirty hack with global called_func, 
    #        waiting for rt #42919 to be resolved.
    pjs_func = get_hll_global "called_func"
    
    # dummy this and dyn_env
    #this = new 'PjsObject'
    dyn_env = pjs_new_scope_from_object this
    
    $P0 = pjs_call_function(this, pjs_func, dyn_env, params :flat)
    .return ($P0)
.end
