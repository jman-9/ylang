println("=== bytes example ===");

// Create bytes with <size>, then use index to set values (0-255)
b = <5>;
b[0] = 72;
b[1] = 101;
b[2] = 108;
b[3] = 108;
b[4] = 111;

println("size: {b.size()}");
println("first byte: {b.front()}, last: {b.back()}");

// Copy to another bytes
c = <0>;
c.copy(b);
println("after copy, c.size() = {c.size()}");

// Compare
d = <3>;
d[0] = 1;
d[1] = 2;
d[2] = 3;
e = <3>;
e[0] = 1;
e[1] = 2;
e[2] = 3;
println("d.cmp(e) (same): {d.cmp(e)}");
e[2] = 4;
println("d.cmp(e) (diff): {d.cmp(e)}");

println("=== end ===");
