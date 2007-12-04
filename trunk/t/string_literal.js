s = "'abc\p\x77\"\r\t\n\'\"\u0078\\";
print(s);

s = '"abc\p\x77\"\r\t\n\'\"\u0078\\';
print(s);

s = "华语/華語";
print(s);

    
s = "中文 = 3;"          + '\n' +
    "中文++;"            + '\n' +
    "print(中文 + 中文);" ;

print(s);

if ('__pjs__' in this) {
    eval(s);
} else {
    // spidermonkey does not accept non-ascii
    // as identifiers!
    print(8);
}

s = 
"function ÇĞIİÖŞÜçğıiöşüéœɯʒɾʃ(n) { " + '\n' +
"    print(n);                      " + '\n' +
"    if (n > 0)                     " + '\n' +
"        ÇĞIİÖŞÜçğıiöşüéœɯʒɾʃ(n-1); " + '\n' +
"}                                  " + '\n' +

"ÇĞIİÖŞÜçğıiöşüéœɯʒɾʃ(2);           " ;

if ('__pjs__' in this) {
    eval(s);
} else {
    // spidermonkey does not accept non-ascii
    // as identifiers!
    print(2);
    print(1);
    print(0);
}
