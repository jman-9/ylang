#include "ylang.h"
#include "core/Scanner.h"
#include <format>
using namespace std;

const char* testcode =
R"TEST(


++ -- += & % ! != hahaha
_3hjssdj 0x192
037 890
'sdokfosdkfoskdfokfsd' "sdofksdokfsdfofksdfk'erokdfgoixcjvojksdj;'sdflkcxvik4m12hbjh\"\ckjr\"\n\r\\\\sodpfpsdlfpl\""

"""
spsdlfpsldfpl
vcitimdmnxc

,
.ddp fdfssdfpcc
     spdflpsdlfpsldf
"""
)TEST";


int main()
{
	Scanner s;
	s.Scan(testcode);
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
