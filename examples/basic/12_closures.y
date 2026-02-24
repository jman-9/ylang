println("=== closures example ===");

// Nested function captures outer variables
fn make_adder(n) {
    fn add(x) {
        return n + x;
    }
    return add;
}
add5 = make_adder(5);
add10 = make_adder(10);
println("add5(3) = {add5(3)}, add10(3) = {add10(3)}");

// Recursive closure
fn countdown(start) {
    fn step() {
        if (start <= 0) return start;
        start--;
        return step();
    }
    return step;
}
cnt = countdown(3);
println("countdown from 3: {cnt()}");

// Multiple levels of capture
fn outer(a) {
    fn middle(b) {
        fn inner(c) {
            return a + b + c;
        }
        return inner;
    }
    return middle;
}
println("outer(1)(2)(3) = {outer(1)(2)(3)}");

println("=== end ===");
