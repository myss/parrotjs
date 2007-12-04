try {
    try {
        print ("ts1");
        throw "e";
    }
    finally {
        print ("fs3");
        try {
            print ("ts4");
            throw "e";
            print ("te4");
        }
        catch (ex) {
            print ("cs4");
            print ("ce4");
        }
        print ("fe5");
    }
}
catch (e) {
    print ("error");
}
