#include "catch2/catch_amalgamated.hpp"
#include "core/module/ModuleManager.h"
#include "core/Scanner.h"
#include "core/StringInterpolator.h"
#include "core/Parser.h"
#include "core/SemanticAnalyzer.h"
#include "core/BytecodeBuilder.h"
#include "core/Args.h"
#include "core/vm/Machine.h"
#include <iostream>
#include <map>
#include <vector>
using namespace std;
using namespace yvm;


#define DEBUG_OUT



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



static std::vector<Error> Build(const std::string& src, Program& retProgram)
{
	vector<Error> errs;

	do {	//todo memory leak
		Scanner s;
		s.Scan(src);
		if(!s._errors.empty())
		{
			errs.insert(errs.end(), s._errors.begin(), s._errors.end());
			break;
		}

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
		if(!p._errors.empty())
		{
			errs.insert(errs.end(), p._errors.begin(), p._errors.end());
			break;
		}

		SemanticAnalyzer sa;
		sa.Analyze(*ast);
		if(!sa._errors.empty())
		{
			errs.insert(errs.end(), sa._errors.begin(), sa._errors.end());
			break;
		}

		BytecodeBuilder bb;
		if(!bb.Build(*ast, retProgram))
		{//TODO trace
			throw 'n';
		}

	#ifdef DEBUG_OUT
		for(int i=0; i<retProgram._mainCode._codeStr.size(); i++)
		{
			cout << format("{:4} {}\n", i, retProgram._mainCode._codeStr[i]);
		}
	#endif

	} while(0);

#ifdef DEBUG_OUT
	for(auto e : errs)
	{
		string errStr = format("{}({}): error E{}: {}", "some file", e.line, (int)e.code, e.msg);
		cout << errStr << endl;
	}
#endif
	return errs;
}


struct Result
{
	int code;
	bool build;
	vector<Error> errs;
};

static Result Run(const std::string& src)
{
	vector<Error> errs;

	Program p;
	errs = Build(src, p);
	if(!errs.empty())
		return { -98765432, false, errs };

	yvm::Machine m;
	return { m.Run(p), true, errs };
}


TEST_CASE( "Primitive String Test", "[primstr]" )
{
	Result ret;

	ret = Run( R"YT( a = 'hello'; if(a.len() != 5) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = 'a b c d t'; if(a.find(' c d') != 3) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = 'pika pika chu'; if(a.substr(5, 4) + a.substr(10) != 'pikachu') exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = 'hello world'; a = a.replace('world', 'ylang'); if(a != 'hello ylang') exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = 'a b c d t'; if(a.split().len() != 5) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = '\r \n \t \v 12345 7\t9 \r \n \t \v '; if(a.trim().len() != 9) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = '  \t  12345 7\t9 \t\n'; if(a.ltrim().len() != 12) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( a = ' \v 12345 7\t9 \t\n\r\v '; if(a.rtrim().len() != 12) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( delim = '::'; list = ['aa' , 'bb', 'cc', 'dd']; if(delim.join(list) != 'aa::bb::cc::dd') exit(1); )YT" );
	REQUIRE( ret.code == 0 );
}

TEST_CASE( "Builtin Random Test", "[bltrand]" )
{
	Result ret;

	ret = Run( R"YT( include rand; rand.randomize_timer(); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( include rand; for(i=0; i<20000; i+=1) { r=rand.get(-2052, 9810); if(!(-2052 <= r && r <= 9810)) exit(1); } )YT" );
	REQUIRE( ret.code == 0 );
}

TEST_CASE( "Builtin Sys Test", "[bltsys]" )
{
	Result ret;

	ret = Run( R"YT( include sys; println(sys.version); if(sys.version.empty()) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( include sys; println(sys.executable); if(sys.executable.empty()) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT( include sys; println(sys.argv); )YT" );
	REQUIRE( ret.code == 0 );
}

TEST_CASE( "Builtin File Test", "[bltfile]" )
{
	Result ret;

	ret = Run( R"YT(
		w = "hahahaha";
		include file;
		a = file.open("test.txt", "w+");
		a.write(w);
		a.close();

		a.open("test.txt", "r");
		r = a.read(100);
		a.close();
		println("{w} {w.len()}");
		println("{r} {r.len()}");
		if(r != w) exit(1);
	)YT" );
	REQUIRE( ret.code == 0 );
}

TEST_CASE( "Builtin Json Test", "[bltjson]" )
{
	Result ret;

	ret = Run( R"YT(
		include json;

		a = json.parse("""
			{
				"glossary": {
					"title": "example glossary",
					"GlossDiv": {
						"title": "S",
						"GlossList": {
							"GlossEntry": {
								"ID": "SGML",
								"SortAs": "SGML",
								"GlossTerm": "Standard Generalized Markup Language",
								"Acronym": "SGML",
								"Abbrev": "ISO 8879:1986",
								"GlossDef": {
									"para": "A meta-markup language, used to create markup languages such as DocBook.",
									"GlossSeeAlso": ["GML", "XML"]
								},
								"GlossSee": "markup"
							}
						}
					}
				}
			}""");
		println("{a}\n");

		include file;
		jf = file.open("jsontest.txt", "w+");
		jf.write(json.dump(a, 2));
		jf.close();

		jf.open("jsontest.txt", "r");
		r = jf.read(10000);
		jf.close();

		b = json.parse(r);
		if(json.dump(a, 2) != json.dump(b, 2))
			exit(1);

		println(json.dump(b, 2));
	)YT" );
	REQUIRE( ret.code == 0 );
}

TEST_CASE( "Expression Test", "[exp]" )
{
	Result ret;

	ret = Run( R"YT( a = b = 9; exit(a); )YT" );
	REQUIRE( ret.code == 9 );

	ret = Run( R"YT( a = (b = 9); exit(a); )YT" );
	REQUIRE( ret.code == 9 );

	ret = Run( R"YT( (a = b) = 9; exit(a); )YT" );
	REQUIRE( !ret.build );

	ret = Run( R"YT( c = 0; a = b + c = 0; )YT" );
	REQUIRE( !ret.build );

	ret = Run( R"YT( b = 0; a = b - (c = 1); exit(a); )YT" );
	REQUIRE( ret.code == -1 );

	ret = Run( R"YT( b = 1; a = b += 1; exit(a); )YT" );
	REQUIRE( ret.code == 2 );

	ret = Run( R"YT( a = c += 1; )YT" );
	REQUIRE( !ret.build );

	ret = Run( R"YT( b = 1; a -= b += 1; )YT" );
	REQUIRE( !ret.build );
}

TEST_CASE( "Logical Operator Test", "[logop]" )
{
	Result ret;
	ret = Run( R"YT( a = 1; b = 1; c = 2; d = a+1>0 && b < 0 || c > 2; if(d) exit(1); )YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT(
		fn Sum(a, b) {
			return a + b;
		}

		a = b = c = 0;
		c = (a = Sum(0, 1)) || (b = Sum(1, 1));
		if(b) exit(1);

		a = b = c = 0;
		c = (a = Sum(0, 0)) && (b = Sum(1, 1));
		if(b) exit(2);

		a = b = c = 0;
		d = (a = Sum(1, 0)) && (b = Sum(0, 0)) || (c = Sum(1, 1));
		if(!d) exit(3);
		if(c != 2) exit(3);

		a = b = c = 0;
		d = (a = Sum(0, 0)) && ((b = Sum(1, 2)) || (c = Sum(1, 1)));
		if(d) exit(4);
		if(b) exit(4);
		if(c) exit(4);

		a = b = c = 0;
		d = (a = Sum(1, 0)) && ((b = Sum(1, 2)) || (c = Sum(1, 1)));
		if(!d) exit(5);
		if(b != 3) exit(5);
		if(c) exit(5);

		a = b = c = 0;
		d = (a = Sum(0, 0)) || (b = Sum(0, 0)) && (c = Sum(1, 0));
		if(d) exit(6);
		if(c) exit(6);

		if(a = 0 || 1) if(!a) exit(7);
	)YT" );
	REQUIRE( ret.code == 0 );
}

TEST_CASE( "Class Test", "[class]" )
{
	Result ret;
	ret = Run( R"YT(
		class Hello {
			front = "hello";
			back = "world";

			fn Say() {
				println("{front}, {back}");
				return;
			}
		}

		hello = Hello();
		hello.Say();
	)YT" );
	REQUIRE( ret.code == 0 );

	ret = Run( R"YT(
		class Ctor { _test = 15; }
		exit(Ctor()._test);
	)YT" );
	REQUIRE( ret.code == 15 );

	ret = Run( R"YT(
		class Ctor { _test = 15; fn Ctor(test) _test = 9; }
		exit(Ctor()._test);
	)YT" );
	REQUIRE( ret.code == 9 );

	ret = Run( R"YT(
		class Ctor { _test = 15; fn Ctor(test) { _test = test; } }
		exit(Ctor(789)._test);
	)YT" );
	REQUIRE( ret.code == 789 );

	ret = Run( R"YT(
		class Ctor { _test = 15; fn Ctor(test) { _test = test; } }
		ctor = Ctor(789);
		exit(ctor._test = 987);
	)YT" );
	REQUIRE( ret.code == 987 );

	ret = Run( R"YT(
		class Ctor { _test = 15; fn Ctor(test) { _test = test; } }
		ctor = Ctor(789);
		exit(ctor._test = 987);
	)YT" );
	REQUIRE( ret.code == 987 );

	ret = Run( R"YT(
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

		if(p1._pos._x != 1.5 || p1._pos._y != 0.75) exit(1);
		if(p2._pos._x != 4.7 || p2._pos._y != -1.85) exit(2);
		if(p3._pos._x != -2.55 || p3._pos._y != 2.8) exit(3);
	)YT" );
	REQUIRE( ret.code == 0 );
}



static const Catch::LeakDetector leakDetector;

int main(int argc, const char** argv)
{
	(void)argc;
	(void)argv;
	(void)&leakDetector;

	yrun::ArgsCollector::Collect(argc, argv);

	static Catch::Session _session;
	Catch::ConfigData& cfg = _session.configData();

	cfg.showSuccessfulTests = true;
	cfg.testsOrTags.push_back("[primstr],");
	cfg.testsOrTags.push_back("[bltrand],");
	cfg.testsOrTags.push_back("[bltsys],");
	cfg.testsOrTags.push_back("[bltfile],");
	cfg.testsOrTags.push_back("[bltjson],");
	cfg.testsOrTags.push_back("[exp],");
	cfg.testsOrTags.push_back("[logop],");
	cfg.testsOrTags.push_back("[class],");

	int numFailed = _session.run();
};
