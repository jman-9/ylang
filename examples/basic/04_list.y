println("=== list example ===");

a = [1, 2, 3];
println(a.size());

a.push_back(4);
a.insert(1, 99);
println(a);

a.pop(2);
println(a);

a.pop_front();
a.pop_back();
println(a);
