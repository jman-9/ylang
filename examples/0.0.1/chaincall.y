println("=== chained calls ===");

fn add1(x)
{
    return x + 1;
}

fn add2(x)
{
    return add1(x) + 1;
}

println(add2(5));
