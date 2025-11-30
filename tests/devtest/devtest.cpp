#include "core/Scanner.h"
#include "core/StringInterpolator.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/vm/Machine.h"
#include "core/Args.h"
#include <iostream>
#include <format>
using namespace std;

#if 0
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
#endif

const char* testcode2 =
R"TEST(
include sys;
include math;

class Vector2D {
    _x = 0; _y = 0;
    fn Vector2D(x, y) { _x = x; _y = y; }
    fn add(other) { return Vector2D(_x + other._x, _y + other._y); }
    fn scale(s) { return Vector2D(_x * s, _y * s); }
    fn length() { return math.sqrt(_x*_x + _y*_y); }
    fn toString() { return "Vector2D({_x}, {_y})"; }
}

class Particle {
    _pos = 0; _vel = 0;
    fn Particle(px, py, vx, vy) { _pos = Vector2D(px, py); _vel = Vector2D(vx, vy); }

    fn update(dt) { _pos = _pos.add(_vel.scale(dt)); }

    fn debug() { println("[Particle] pos={_pos.toString()}, vel={_vel.toString()}"); }
}

class ParticleSystem {
    _particles = [];

    fn add(p) { _particles.append(p); return 123456789; }
    fn updateAll(dt) { for(i=0; i<_particles.len(); i++) _particles[i].update(dt); }
    fn debug() {
        println("=== ParticleSystem Debug ===");
        for(i=0; i<_particles.len(); i++) _particles[i].debug();
    }
}

println("ylang class test, version = {sys.version}");

p1 = Particle(0, 0, 1, 0.5);
p2 = Particle(5, -2, -0.2, 0.1);
p3 = Particle(-3, 4, 0.3, -0.8);

ps = ParticleSystem();

ps.add(p1);
ps.add(p2);
ps.add(p3);

println("--- Before Update ---");
ps.debug();

println("--- Update dt=1.0 ---");
ps.updateAll(1.0);
ps.debug();

println("--- Update dt=0.5 ---");
ps.updateAll(0.5);
ps.debug();
)TEST";

int main(int argc, const char** argv)
{
	yrun::ArgsCollector::Collect(argc, argv);

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
		StringInterpolator si;
		vector<Token> processed;
		for(auto& t : s._tokens)
		{
			if(t != EToken::Str)
			{
				processed.push_back(t);
				continue;
			}

			auto interpolated = si.Interpolate(t);
			if(interpolated.empty())
			{//todo error
				throw 'n';
			}
			processed.insert(processed.end(), interpolated.begin(), interpolated.end());
		}

		Parser p(processed);
		auto ast = p.Parse();
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
		Program prg;
		if(!bb.Build(*ast, prg)) throw 'n';

		for(int i=0; i<prg._mainCode._codeStr.size(); i++)
		{
			cout << format("{:4} {}\n", i, prg._mainCode._codeStr[i]);
		}

		yvm::Machine m;
		m.Run(prg);
	}

	return 0;
}
