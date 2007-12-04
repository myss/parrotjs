var import_all;

if ('__pjs__' in this) {
    var imp = eval('import'); // import is a keyword in spidermonkey

    print('in file: importing');
    
    import_all = function(file) {
        print('importing: ' + file + '.js');
        imp(file + '.js');
        print('importing: ' + file + '.pir');
        imp(file + '.pir');
        print('importing: ' + file + '.pbc');
        imp(file + '.pbc');
    }
    
    import_all('importing/x');
    
} 
else {

    print('in file: importing');
    print("importing: importing/x.js");
    print("in file: x");
    print("importing: dir/y.js");
    print("in file: y");
    print("importing: ../z.js");
    print("in file: z");
    print("importing: ../z.pir");
    print("in file: z");
    print("importing: ../z.pbc");
    print("importing: dir/y.pir");
    print("in file: y");
    print("importing: ../z.js");
    print("in file: z");
    print("importing: ../z.pir");
    print("importing: ../z.pbc");
    print("importing: dir/y.pbc");
    print("importing: importing/x.pir");
    print("in file: x");
    print("importing: dir/y.js");
    print("in file: y");
    print("importing: ../z.js");
    print("in file: z");
    print("importing: ../z.pir");
    print("importing: ../z.pbc");
    print("importing: dir/y.pir");
    print("importing: dir/y.pbc");
    print("importing: importing/x.pbc");
    
}
