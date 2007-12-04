.HLL "Pjs", "pjs_group"
.loadlib 'pjs_group_ops'

.const int FLAG_READ_ONLY   = 1
.const int FLAG_DONT_ENUM   = 2
.const int FLAG_DONT_DELETE = 4
.const int FLAG_INTERNAL    = 8

.sub set_flags
    .param pmc      js_obj
    .param string   prop_name
    .param pmc      flags :slurpy
    
    .local int result_flags
    result_flags = js_obj[prop_name]
    
    .local int i, len, next_flag
    len = flags
    i = 0
  loop:
    unless i < len goto end
    next_flag = flags[i]
    result_flags |= next_flag
    inc i
    goto loop
  end:
  
    js_obj[prop_name] = result_flags
.end


.include 'languages/pjs/lib/jscore_includes/conversions.pir'
.include 'languages/pjs/lib/jscore_includes/comparison.pir'
.include 'languages/pjs/lib/jscore_includes/pjsfunction.pir'
.include 'languages/pjs/lib/jscore_includes/js_function_subs.pir'

.namespace

# Parrot_runops_fromc_args can't directly call subs
# that do a tail-call (bug?) . As a workaround, it
# will call this.
.sub do_call
    .param pmc subroutine
    .param pmc rest :slurpy
    $P0 = subroutine(rest :flat)
    .return ($P0)
.end

.sub initialize_pjs :load :init :anon
    .local pmc global_env
    global_env = get_hll_global 'global_env'
    unless null global_env goto end
    
    .local pmc global_obj
    global_obj = new 'PjsObject'
    global_obj['this'] = global_obj

    global_env = pjs_new_scope_from_object global_obj
    set_hll_global 'global_env', global_env
    
    .local pmc undef
    undef = new 'PjsUndefined'
    set_hll_global 'undefined', undef
    
    global_obj['arguments'] = undef
    set_flags(global_obj, 'arguments', FLAG_DONT_ENUM)
    
    load_bytecode 'languages/pjs/lib/stdlib.pbc'
  end:
    .return (global_env)
.end

.sub __pjs_instanceof__
    .param pmc env
    .param pmc obj
    .param pmc constructor
    
    $I0 = isa constructor, 'PjsObject'
    unless $I0 goto err
    
    $P0 = constructor['__has_instance__']
    $I0 = isa $P0, 'PjsUndefined'
    if $I0 goto err
    $I0 = isa $P0, 'PjsNull'
    if $I0 goto err
    
    .return pjs_invoke_method(constructor, '__has_instance__', env, obj)
  
  err:
    # TODO use a real Error object
    $P0 = new 'Exception'
    $P0[0] = 'TypeError'
    throw $P0
.end

# Evaluate pir code compiled from a 
# javascript source.
.sub eval_js_pir
    .param string pir_code
    .param pmc env
    
    .local pmc comp, main_getter, main, this
    comp = compreg 'PIR'
    main_getter = comp(pir_code)
    main = main_getter()
    
    this = pjs_find_lex env, 'this'
    .return main(this, env, env)
.end

.sub read_file
    .param string path
    .local pmc file
    file = open path, '<'
    if null file goto not_found
    unless file goto not_found
    $S0 = file.'slurp'('')
    close file
    .return ($S0)
  not_found:
    $S0 = 'File not found: '
    $S0 = $S0 . path
    $P0 = new 'Exception'
    $P0[0] = $S0
    throw $P0
.end

# split_directory_file('x/y/z.ext') => ('x/y/z/', 'z.ext')
# assumes that path is not a path for a directory
.sub split_directory_file
    .param string path
    
    .local pmc splitted
    .local int len
    splitted = split '/', path
    len = elements splitted
    
    if len < 2 goto no_dir
    
    .local int i
    .local string dir, file_name
    dec len
    file_name = splitted[len]
    i = 0
    dir = ''
  loop:
    $S0 = splitted[i]
    dir .= $S0
    dir .= '/'
    inc i
    if i < len goto loop
    .return (dir, file_name)
    
  no_dir:
    .return ('./', path)
.end
