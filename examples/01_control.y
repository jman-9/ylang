println("=== control example ===");

for (i = 1; i <= 5; i += 1) {
    if (i == 3) {
        println("three");
        continue;
    }
    if (i == 5) {
        println("five");
        break;
    }
    println(i);
}
