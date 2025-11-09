println("=== break / continue ===");

for(i = 1; i <= 10; i += 1)
{
    if(i == 3)
        continue;

    if(i == 7)
        break;

    println(i);
}
