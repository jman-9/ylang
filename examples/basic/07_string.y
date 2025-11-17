println("=== string example ===");

a = "hello world";
println(a);
println(a.len());
println(a.find("world"));
println(a.substr(0, 5));

b = a.replace("world", "ylang");
println(b);

println("a b   c d".split());
println("apple,banana,,grape".split(","));
