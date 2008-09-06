.namespace

.sub is_primitive
    .param pmc x
    .local int type_num
    type_num = typeof x
    if type_num == .PjsNumber    goto primitive
    if type_num == .PjsString    goto primitive
    if type_num == .PjsBoolean   goto primitive
    if type_num == .PjsUndefined goto primitive
    if type_num == .PjsNull      goto primitive
    .return (0)
  primitive:
    .return (1)
.end

#=============================================

.sub __toObject__ :multi(_, _)
    .param pmc p
    .param pmc env
    .return (p)
.end

.sub __toObject__ :multi(PjsNumber, _)
    .param pmc primitiveNumber
    .param pmc env
    .local pmc NumberFunction, nProto, o
    NumberFunction = pjs_find_lex env, 'Number'
    nProto = NumberFunction['prototype']
    o = new 'PjsObject'
    o.'setProto'(nProto)
    o['__value__'] = primitiveNumber
    .return (o)
.end

.sub __toObject__ :multi(PjsString, _)
    .param pmc primitiveString
    .param pmc env
    .local pmc StringFunction, sProto, o
    StringFunction = pjs_find_lex env, 'String'
    sProto = StringFunction['prototype']
    o = new 'PjsObject'
    o.'setProto'(sProto)
    o['__value__'] = primitiveString
    
    $S0 = primitiveString
    $I0 = length $S0
    $P0 = new 'PjsNumber'
    $P0 = $I0
    o['length'] = $P0
    .return (o)
.end

.sub __toObject__ :multi(PjsBoolean, _)
    .param pmc primitiveBoolean
    .param pmc env
    .local pmc BooleanFunction, bProto, o
    BooleanFunction = pjs_find_lex env, 'Boolean'
    bProto = BooleanFunction['prototype']
    o = new 'PjsObject'
    o.'setProto'(bProto)
    o['__value__'] = primitiveBoolean
    .return (o)
.end

.sub __toObject__ :multi(PjsNull, _)
    .param pmc p
    .param pmc env
    # TODO wrap in a TypeError
    $P0 = new 'Exception'
    $P0['message'] = "TypeError: can't convert null to object"
    throw $P0
.end

.sub __toObject__ :multi(PjsUndefined, _)
    .param pmc p
    .param pmc env
    # TODO wrap in a TypeError
    $P0 = new 'Exception'
    $P0['message'] = "TypeError: can't convert undefined to object"
    throw $P0
.end

