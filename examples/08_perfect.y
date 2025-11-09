println("=== perfect numbers (under 1000) ===");

for(i=2; i<=1000; i+=1)
{
	sum = 0;
	k = i / 2;

	for(j=1; j<=k; j+=1)
	{
		r = i % j;
		if(r == 0)
		{
			sum += j;
		}
	}

	if(i == sum)
	{
		print(i + " ");
	}
}

println();
