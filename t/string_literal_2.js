s = '\x77';
print(s);

s = '\xab';
print(s);

s = '\u0077';
print(s);

s = '\uabcd';
print(s);

print('\uaaaa' == '\uaaaa');
print('\uaaaa' == '\uaaab');
