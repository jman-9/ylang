println("=== function reference example ===");

// Assign function to variable and call through it
fn add(a, b) { return a + b; }
f = add;
println("f(3, 5) = {f(3, 5)}");

// Pass function as argument
fn apply(op, x, y) {
    return op(x, y);
}
println("apply(add, 10, 20) = {apply(add, 10, 20)}");

// Return function from function
fn make_mult(n) {
    fn mul(x) { return n * x; }
    return mul;
}
double = make_mult(2);
triple = make_mult(3);
println("double(7) = {double(7)}, triple(7) = {triple(7)}");

// Method reference (call class method via variable)
class Box {
    _v = 0;
    fn Box(v) { _v = v; }
    fn get() { return _v; }
}
box = Box(42);
g = box.get;
println("g() = {g()}");

println("=== end ===");
