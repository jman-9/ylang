#include "core/Scanner.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/vm/Machine.h"
#include <iostream>
#include <format>
using namespace std;

const char* lextestcode =
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

const char* exptestcode =
R"TEST(

!!!a + b;
c + b;

)TEST";

const char* iftestcode =
R"TEST(

if(1) { a = 1 } else { b = 1 }

c = 1

{
  if(0) { a = 1 } else { b = 1 }

  f = 10
}

)TEST";

const char* fortestcode =
R"TEST(

for( ; ; ) { continue }

for(i=1; i<10; i+=1) { break }

)TEST";

const char* fntestcode =
R"TEST(

fn test(a, b, c) {
  d = a + b + c
  return d
}

test(1, 2, 3)

)TEST";

const char* postfixtestcode =
R"TEST(

fn test(a, b, c) {
  d = a + b + c;
  return d;
}

p = 1 + 30 * (3 + 2) / (5 - ((test))(1, 2, 3));

g[p] = 10;

)TEST";

const char* testcode =
R"TEST(
/*
tt = 10;

t = 20 + tt - 50 * ((61 - 3)) + 4;

{
  ttt = 20;
  tt += ttt;
}

//tt = sum(1,1,1);
*/
fn sum(a, b, c) {
  d = a + b + c;
  return d;
}
/*
//sum(1, 1);
t += sum(1, 2, 3);
tt += t * 10 / 2;
*/

a = 10;
for(i=0; i<10; i+=1)
{
  a += sum(1,1,1);
  if(a > 0)
  {
    break;
  }
  else
  {
    break;
  }
}

/*
if(a > 10)
{
  fn sum2(a1, b, c) {
    d = a1 + b + c;
    return d;
  }

  p = 1 + 30 * (3 + 2) / (5 - sum2(1, 2, 3));
}
else if(a < 5)
{
  fn sum2(a1, b, c) {
    d = a1 + b + c;
    return d;
  }
  p = 3 * 7 + sum2(4, 5, 6);
}
else
{
  if(a == 6)
    p = 6 * 9;
  else if(a == 7)
    p = 7 * 9;
  else
  {
    if(a == 8) p = 8 * 9; else p = 9 * 9;
  }
}
*/
)TEST";

const char* testcode2 =
R"TEST(
a = { 'a' : 'orange', 'b' : 'apple' };
if(a['a'] == 'orange')
{
     println('I like orange');
}
else
{
     println('I hate orange');
}

)TEST";

int main()
{
	Scanner s;
	//s.Scan(testcode);
	s.Scan(testcode2);

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
	else
	{
		Parser p(s._tokens);
		TreeNode* ast = p.Parse();
		if(!ast)
		{
			for(auto e : p._errors)
			{
				string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
				cout << errStr << endl;
			}
			return 1;
		}

		SemanticAnalyzer sa;
		if(!sa.Analyze(*ast))
		{
			for(auto e : sa._errors)
			{
				string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
				cout << errStr << endl;
			}
			return 1;
		}

		BytecodeBuilder bb;
		Bytecode c;
		if(!bb.Build(*ast, c)) throw 'n';

		for(int i=0; i<bb._bytecodeStr.size(); i++)
		{
			cout << format("{:4} {}\n", i, bb._bytecodeStr[i]);
		}

		yvm::Machine m;
		m.Run(c);
	}

	return 0;
}
