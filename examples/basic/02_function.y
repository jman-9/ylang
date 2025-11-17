println("=== function example ===");

fn square(x) {
    return x * x;
}

fn fact(n) {
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}

println("square of 4 : " + square(4));
println("factorial of 5 : {fact(5)}");
