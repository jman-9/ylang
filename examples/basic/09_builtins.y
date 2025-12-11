// Built-in module showcase: math, rand, sys

include math;
include rand;
include sys;

fn main() {
    println("=== Built-in Module Showcase ===");
    println('This example demonstrates how to use sys, rand, and math modules.');
	println();

    println("[1] System Information:");
	println("Version: " + sys.version);
    println("Executable: " + sys.executable);
    println("Arguments:  {sys.argv}");
	println();

    println("[2] Random Number Example:");
    rand.randomize_timer();
    r = rand.get(1, 10);
    println("Random number between 1 and 10: {r}");
	println();

    println("[3] Math Functions:");
    pi_d2 = 1.57079632679;
    println("sin(pi/2) = {math.sin(pi_d2)}");
    println("sqrt(2)   = {math.sqrt(2.)}");
	println();
    println("=== End of Example ===");
}
