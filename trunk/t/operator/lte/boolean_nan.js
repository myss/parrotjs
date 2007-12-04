function testWith(bool) {
    print(undefined <= bool);
    print(bool <= undefined);
}

print(true <= true);
print(true <= false);
print(false <= true);
print(false <= false);

testWith(true);
testWith(false);

