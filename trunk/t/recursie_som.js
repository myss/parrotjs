
function som(n) {
    if (n == 0) {
        return 0;
    } else {
        return n + som(n-1);
    }
}

s = som(790);
print(s);