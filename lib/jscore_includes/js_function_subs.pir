# This file contains some PIR subs that are
# used as PIR subroutines of some JS functions.

.namespace

.sub __object_constructor__
    .param pmc @this
    .param pmc @env_0
    .param pmc @dyn_env
    .param pmc par@b :optional
    .param int has@b :opt_flag
    .param pmc @rest :slurpy

    unless has@b    goto is_undef
    $I0 = isa par@b, 'PjsUndefined'
    if $I0          goto is_undef
    $I0 = isa par@b, 'PjsNull'
    if $I0          goto is_undef
    
    .return '__toObject__'(par@b, @env_0)
    
  is_undef:
    .local pmc object_prototype
    $P0 = pjs_find_lex @env_0, 'Object'
    object_prototype = $P0['prototype']

    $P0 = new 'PjsObject'
    $P0.'setProto'(object_prototype)
    .return ($P0)
.end


.sub __function_constructor__
    .param pmc @this
    .param pmc @env_0
    .param pmc @dyn_env
    .param pmc @rest :slurpy

    .local string body, params
    body    = ''
    
    .local int len
    len = @rest
    if len == 0 goto next
    
    body = pop @rest
    params = join ',', @rest
    
  next:
    .local string js_code, pir_code
    
    js_code = 'function (' . params
    js_code .= ') { '
    js_code .= body
    js_code .= ' }'
    
    .local pmc global_env
    global_env = pjs_get_global_env @dyn_env
    
    pir_code = pjs_compile js_code, 0
    .return eval_js_pir(pir_code, global_env)
.end

.sub __array_constructor__
    .param pmc @this
    .param pmc @env_0
    .param pmc @dyn_env
    .param pmc @args :slurpy
    
    .local pmc arr, parrotarr, arr_proto, arrayFunc
    .local int args_len
    
    arr = new 'PjsArray'
    arrayFunc = pjs_find_lex @dyn_env, 'Array'
    arr_proto = arrayFunc['prototype']
    arr.'setProto'(arr_proto)
    
    # check @args
    args_len = @args
    if args_len > 0 goto has_args
    .return (arr)
    
  has_args:
    if args_len > 1 goto append_args
    $P0 = @args[0]
    $I0 = isa $P0, 'PjsNumber'
    unless $I0 goto append_args
    arr['length'] = $P0
    .return (arr)
    
  append_args:
    $I0 = 0
  loop:
    unless $I0 < args_len goto end
    $P0 = @args[$I0]
    $P1 = new 'PjsNumber'
    $P1 = $I0
    arr[$P1] = $P0
    inc $I0
    goto loop
  end:
    .return (arr)
.end
