  var input = 2;
  var result = 0;

  switch ( input ) {
  case 0:
    result += 2;
  case 1:
    result += 4;
    break;
  case 2:
    result += 8;
  case 3:
    result += 16;
  default:
    result += 32;
    break;
  case 4:
    result += 64;
  }
  print('input = ' + input + ' , result = ' + result);
