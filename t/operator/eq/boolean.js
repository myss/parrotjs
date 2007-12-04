function testWith(bool) {
    print(bool == bool);
    print(bool == false);
    print(false == bool);
    print(false == false);
    
    print(bool == "24");
    print("86" == bool);
    
    print(bool == 4);
    print(4 == bool);
    
    print(null == bool);
    print(bool == null);
    
}

print(true == true);
print(true == false);
print(false == true);
print(false == false);

testWith(true);
testWith(false);

