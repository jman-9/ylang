#include "ylang.h"
#include "core/Scanner.h"
#include <format>
using namespace std;

const char* testcode =
R"TEST(

func
if

++ -- += & % ! != hahaha	// i'm comment
_3hjssdj 0x192
037 890
'sdokfosdkfoskdfokfsd' "sdofksdokfsdfofksdfk'erokdfgoixcjvojksdj;'sdflkcxvik4m12hbjh\"\\ckjr\"\n\r\\\\sodpfpsdlfpl\""

else

/*
block
comment
*/

"""
spsdlfpsldfpl
vcitimdmnxc

for

,
.ddp fdfssdfpcc
     spdflpsdlfpsldf
"""

for

)TEST";


int main()
{
	Scanner s;
	s.Scan(testcode);

	for(auto t : s._tokens)
	{
		string ts = format("line:{},kind:{},val:{}", t.line, (int)t.kind, t.val);
		cout << ts << endl;
	}

	if(!s._errors.empty())
	{
		for(auto e : s._errors)
		{
			string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
			cout << errStr << endl;
		}
	}

	return 0;
}
