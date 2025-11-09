fn fact(n)
{
    if(n <= 1)
        return 1;
    return n * fact(n - 1);
}

println("=== factorials ===");
for(i = 1; i <= 10; i += 1)
{
    println(i + "! = " + fact(i));
}
