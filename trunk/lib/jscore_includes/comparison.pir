.namespace

# TODO ugly! Find a better way!
.sub __pjs_eq__
    .param pmc a
    .param pmc b
    .local int class_a, class_b
    .local int isobj_a, isobj_b
    
    isobj_a = isa a, 'PjsObject'
    isobj_b = isa b, 'PjsObject'
    
    $I0 = isobj_a && isobj_b
    if $I0 goto cmp_identity

    class_a = typeof a
    class_b = typeof b
    
    # why gives 'if ==' an error?
    if class_a == class_b goto X_X
    #eq_addr class_a, class_b, X_X
    
    # return true if both are undef or null
    .local int undef_a, undef_b
    $I0 = isa a, 'PjsUndefined'
    $I1 = isa a, 'PjsNull'
    undef_a = $I0 || $I1
    $I0 = isa b, 'PjsUndefined'
    $I1 = isa b, 'PjsNull'
    undef_b = $I0 || $I1
    $I0 = undef_a && undef_b
    if $I0 goto true
    
    .local int num_a, num_b, str_a, str_b
    num_a = isa a, 'PjsNumber'
    str_b = isa b, 'PjsString'
    $I0 = num_a && str_b
    if $I0 goto num_str
    
    str_a = isa a, 'PjsString'
    num_b = isa b, 'PjsNumber'
    $I0 = str_a && num_b
    if $I0 goto str_num
    
    $I0 = isa a, 'PjsBoolean'
    if $I0 goto bool_X
    
    $I0 = isa b, 'PjsBoolean'
    if $I0 goto X_bool
    
    $I0 = str_a || num_a
    $I0 = $I0 && isobj_b
    if $I0 goto X_obj
    
    $I0 = str_b || num_b
    $I0 = $I0 && isobj_a
    if $I0 goto obj_X
    
    .return (0)
    
  cmp_identity:
    $I0 = issame a, b
    .return ($I0)

  X_X:
    $I0 = iseq a, b
    .return ($I0)
    
  true:
    .return (1)
    
  str_num:
    $N0 = a
    a = new 'PjsNumber'
    a = $N0
    .return __pjs_eq__(a, b)
  num_str:
    $N0 = b
    b = new 'PjsNumber'
    b = $N0
    .return __pjs_eq__(a, b)
    
  bool_X:
    $N0 = a
    a = new 'PjsNumber'
    a = $N0
    .return __pjs_eq__(a, b)
  X_bool:
    $N0 = b
    b = new 'PjsNumber'
    b = $N0
    .return __pjs_eq__(a, b)
    
  obj_X:
    a = pjs_to_primitive a
    .return __pjs_eq__(a, b)
  X_obj:
    b = pjs_to_primitive b
    .return __pjs_eq__(a, b)
.end

#=======================================


# Next subroutines were needed because !(NaN < NaN) and !(NaN == NaN) and !(NaN > NaN).
# But at the PMC level I can only implement comparison with the 'cmp' vtable function, 
# which enforces me ordering.

.sub __pjs_lte__ :multi(PjsString, PjsString)
    .param pmc a
    .param pmc b
    $I0 = a <= b
    .return ($I0)
.end

.sub __pjs_lte__ :multi(_, _)
    .param pmc a
    .param pmc b
    $N0 = a
    $N1 = b
    
#     print "a = "
#     print $N0
#     print "\n"
#     print "b = "
#     print $N1
#     print "\n"
    
    $I0 = $N0 <= $N1
    
#     print "result = "
#     print $I0
#     print "\n"
    
    .return ($I0)
.end

.sub __pjs_gte__ :multi(PjsString, PjsString)
    .param pmc a
    .param pmc b
    $I0 = a >= b
    .return ($I0)
.end

.sub __pjs_gte__ :multi(_, _)
    .param pmc a
    .param pmc b
    $N0 = a
    $N1 = b
    $I0 = $N0 >= $N1
    .return ($I0)
.end

#=======================================

.sub __pjs_in__
    .param pmc a
    .param pmc b
    $I0 = isa b, 'PjsObject'
    unless $I0 goto err
    $S0 = a
    $I0 = exists b[$S0]
    .return ($I0)

  err:
    $P0 = new 'Exception'
    $P0['_message'] = "TypeError (TODO): invalid in operand"
    throw $P0
.end