.HLL 'Pjs', 'pjs_group'
.loadlib 'pjs_group_ops'

.sub read_file
    .param string path
    
    .local pmc file
    if path == '-' goto from_stdin
    file = open path, '<'
    if null file goto not_found
    unless file goto not_found
    goto next
  from_stdin:
    file = getstdin
  next:
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

.sub main :main :anon
    .param pmc args
    
    .local int argc
    argc = args
    if argc < 2 goto repl
    
    $P0 = args[1]
    $S0 = $P0
    
    if $S0 == '--compile' goto compile_standalone
    if $S0 == '-c'        goto compile_standalone
    if $S0 == '--compile-eval'  goto compile_eval
    if $S0 == '-C'              goto compile_eval
    if $S0 == '--help'    goto help
    if $S0 == '-h'        goto help
    
  run:
    load_bytecode 'languages/pjs/lib/jscore.pbc'

    # run each file
    
    .local int i
    .local string path
    i = 1
  loop:
    unless i < argc goto end_loop
    path = args[i]
    inc i
    if path == '-f' goto loop # skip -f (spidermonkey compatibility)
    run_file(path)
    goto loop
  end_loop:
    .return()
    
  compile_error:
    .get_results($P0, $S0)
    say $S0
    .return()
    
  compile_eval:
    if argc < 3 goto help
    $P0 = args[2]
    $S0 = $P0
    'compile'($S0, 0)
    .return ()

  compile_standalone:
    if argc < 3 goto help
    $P0 = args[2]
    $S0 = $P0
    'compile'($S0, 1)
    .return ()
    
  help:
    $P0 = args[0]
    $S0 = $P0
    print "\nUsage:    parrot "
    print $S0
    print " <file.js>               Execute file.js\n"
    
    print "          parrot "
    print $S0
    print " <option> <file.js>      See further\n\n\n"
    
    print "Options:\n\n"
    
    print "    --compile, -c         Compile file.js as a standalone PIR\n"
    print "                             (it is then executable, loadable)\n\n"
    
    print "    --compile-eval, -C    Compile file.js for evaluation\n"
    print "                             (only useful for debugging purposes)\n\n"
    
    print "    --help, -h            Show this help message\n\n"
    .return()
  
  repl:
    repl()
.end

.sub run_file
    .param string path
    
    $S0 = 'read_file'(path)
    $S0 = pjs_compile $S0, 1
    
    .local pmc os
    .local string dir, curr_dir, file_name
    os = new 'OS'
    curr_dir = os.'cwd'()
    (dir, file_name) = 'split_directory_file'(path)
    os.'chdir'(dir)
    
    .local pmc comp, execute
    comp = compreg 'PIR'
    execute = comp($S0)
    #pop_eh
    execute()
    
    os.'chdir'(curr_dir)
    .return()
.end

.sub compile :anon
    .param string path
    .param int is_standalone
    
    .local string s
    s = 'read_file'(path)
    $S0 = pjs_compile s, is_standalone
    say $S0
.end

.sub repl
    load_bytecode 'languages/pjs/lib/jscore.pbc'
    
    .local pmc global_env, global_obj
    global_env = get_hll_global 'global_env'
    global_obj = global_env[0]

    .local pmc pio
    pio = getstdin
    $I0 = pio.'set_readline_interactive'(1)
    if $I0 >= 0 goto loop
    printerr "set_readline_interactive failed: "
    goto ex
  loop:
    $P1 = pio.'readline'('pjs> ')
    if null $P1 goto ex
    $S0 = $P1
    push_eh eval_error
    $S0 = pjs_compile $S0, 0
    $P0 = eval_js_pir($S0, global_env)
    print_jsvalue($P0)
    pop_eh
    goto loop
  eval_error:
    .get_results($P0, $S0)
    say $S0
    goto loop
  ex:
.end

.sub print_jsvalue
    .param pmc p
    
    if null p goto end
    $I0 = isa p, 'PjsUndefined'
    if $I0 goto end
    say p
  end:
.end
