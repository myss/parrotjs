
a = 42;
print(a.undefVar);
print(a.toString());

b = "test";
print(b.undefVar);
print(b.toString());

c = true;
print(c.undefVar);
print(c.toString());

d = undefined;
try { print(d.undefVar);   } catch(e) { print("error 1"); }
try { print(d.toString()); } catch(e) { print("error 2"); }

e = null;
try { print(e.undefVar);   } catch(e) { print("error 3"); }
try { print(e.toString()); } catch(e) { print("error 4"); }


