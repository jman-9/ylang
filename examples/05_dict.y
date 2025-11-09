println("=== dict example ===");

d = {'x': 10, 'y': 20, 'z': 30};
println(d.len());

println(d.keys());
println(d.values());
println(d.items());

println(d['y']);
d.pop("y");
println(d);

d['w'] = 50;
println(d);
