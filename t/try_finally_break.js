label: {
    print("begin label");
    try {
        print("try");
        break label;
    } finally {
        print("finally");
    }
    print("end label");
}
print("after label");