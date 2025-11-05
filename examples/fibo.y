fn fib(n)
{
    if(n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

println("=== fibonacci ===");
for(i = 0; i < 10; i += 1)
{
    println("fib(" + i + ") = " + fib(i));
}
