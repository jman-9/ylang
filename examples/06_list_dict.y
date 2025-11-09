println("=== list-dict mixed example ===");

users = {
    'alice': [10, 20, 30],
    'bob': [5, 15, 25]
};

println(users.keys());

users['alice'].append(40);
println(users['alice']);

v = users['bob'].pop_back();
println(v);
