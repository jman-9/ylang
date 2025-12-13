#include "catch2/catch_amalgamated.hpp"
#include "module/ModuleManager.h"
#include "compiler/Scanner.h"
#include "compiler/StringInterpolator.h"
#include "compiler/Parser.h"
#include "compiler/SemanticAnalyzer.h"
#include "compiler/BytecodeBuilder.h"
#include "compiler/Compiler.h"
#include "util/Args.h"
#include "vm/Machine.h"
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <filesystem>
using namespace std;
using namespace yvm;
using namespace ycom;


#define DEBUG_OUT


struct Result
{
	int code;
	bool build;
	ErrorTable errTbl;
};

static Result Run(const std::string& src)
{
	Compiler cmplr;
	ErrorTable errTbl;
	Program p;

	errTbl = cmplr.CompileCode(src, p);
	if(!errTbl.empty())
		return { -98765432, false, errTbl };

	yvm::Machine m;
	return { m.Run(p), true, errTbl };
}



TEST_CASE( "Scanner Test", "[scanner]" )
{
	Scanner sc;
	StringInterpolator si;
	string tc;

	{
		tc = "";
		for(int i=0; i<10; i++)
			tc = tc + ' ' + (char)('0' + i);
		for(int i=0; i<26; i++)
			tc = tc + '\t' + (char)('a' + i);
		for(int i=0; i<26; i++)
			tc = tc + '\n' + (char)('A' + i);

		sc.Scan(tc);
		int o = 0;
		for(int i=0; i<10; i++, o++)
		{
			REQUIRE(sc._tokens[o] == EToken::Int);
			REQUIRE(sc._tokens[o].line == 1);
		}
		for(int i=0; i<26; i++, o++)
		{
			REQUIRE(sc._tokens[o] == EToken::Id);
			REQUIRE(sc._tokens[o].line == 1);
		}
		for(int i=0; i<26; i++, o++)
		{
			REQUIRE(sc._tokens[o] == EToken::Id);
			REQUIRE(sc._tokens[o].line == i+2);
		}
	}

	{
		map<EToken, string_view> m1;
		map<string, EToken> m2;
		vector<EToken> toks;
		for(int i=(int)EToken::None; i<= (int)EToken::DotDot; i++)
		{
			auto f = Token::TokenString((EToken)i);
			if(!f.empty()) { m1[ (EToken)i ] = f; m2[ string(f) ] = (EToken)i; }
		}

		tc = "";
		for(char c='!'; c<='~'; c++)
		{
			auto found = m2.find(string() + c);
			if(found != m2.end())
			{
				tc = tc + "\n\t " + c;
				toks.push_back(found->second);
			}
		}

		sc.Clear();
		sc.Scan(tc);
		REQUIRE(sc._errors.empty());
		REQUIRE(sc._tokens.size() == toks.size());
		for(size_t i=0; i<toks.size(); i++)
		{
			REQUIRE(sc._tokens[i].kind == toks[i]);
		}

		tc = "";
		for(auto& [k, v] : m2)
			tc = tc + "\n\t " + k;

		sc.Clear();
		sc.Scan(tc);
		REQUIRE(sc._errors.empty());
		REQUIRE(sc._tokens.size() == m2.size());
		for(size_t i=0; i<sc._tokens.size(); i++)
		{
			auto c1 = m1[sc._tokens[i].kind];
			auto c2 = m2[(string)c1];
			REQUIRE(sc._tokens[i].kind == c2);
		}


		for(char c='!'; c<='~'; c++)
		{
			auto found = m2.find(string() + c);
			if(found != m2.end()) continue;

			tc = string() + c;
			sc.Clear();
			sc.Scan(tc);
			REQUIRE(tc != "");
			if(sc._tokens.empty())
				REQUIRE(!sc._errors.empty());
			else
				REQUIRE(!m1.contains(sc._tokens[0].kind));
		}
	}

	{
		const char* org = R"YT("\a\b\f\n\r\t\v\\\'\"\?"")YT";
		const char cmp[] = { 0x07, 0x08, 0x0C, 0x0A, 0x0D, 0x09, 0x0B, '\\', '\'', '"', '?' };

		tc = org;
		sc.Clear();
		sc.Scan(tc);
		REQUIRE(sc._tokens.size() == 1);

		auto interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(interpolated.res.size() == 1);
		REQUIRE(interpolated.res[0].val.size() == sizeof(cmp));

		for(int i=0; i<sizeof(cmp); i++)
			REQUIRE( interpolated.res[0].val[i] == cmp[i] );
	}

	{
		const char* org = R"YT("\1258\0439\077\xBE\xEF\xBA\xBE\xFAT\xCGE\xABCDEF")YT";
		const char cmp[] = { 0125, '8', 043, '9', 077, (char)0xBE, (char)0xEF, (char)0xBA, (char)0xBE,
							(char)0xFA, 'T', (char)0xC, 'G', 'E', (char)0xAB, 'C', 'D', 'E', 'F' };

		tc = org;
		sc.Clear();
		sc.Scan(tc);
		REQUIRE(sc._tokens.size() == 1);

		auto interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(interpolated.res.size() == 1);
		REQUIRE(interpolated.res[0].val.size() == sizeof(cmp));

		for(int i=0; i<sizeof(cmp); i++)
			REQUIRE( interpolated.res[0].val[i] == cmp[i] );
	}


	{
		tc = R"YT("\GF")YT";
		sc.Clear();
		sc.Scan(tc);
		auto interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(!interpolated.errs.empty());

		tc = R"YT("\98")YT";
		sc.Clear();
		sc.Scan(tc);
		interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(!interpolated.errs.empty());

		tc = R"YT("{{}")YT";
		sc.Clear();
		sc.Scan(tc);
		interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(!interpolated.errs.empty());

		tc = R"YT("{}}")YT";
		sc.Clear();
		sc.Scan(tc);
		interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(!interpolated.errs.empty());

		tc = R"YT("{")YT";
		sc.Clear();
		sc.Scan(tc);
		interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(!interpolated.errs.empty());

		tc = R"YT("}")YT";
		sc.Clear();
		sc.Scan(tc);
		interpolated = si.Interpolate(sc._tokens[0]);
		REQUIRE(!interpolated.errs.empty());
	}
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

	ret = Run( R"YT( a = 9; b = 7; exit(!!!a + b); )YT" );
	REQUIRE( ret.code == 7  );
}

TEST_CASE( "For If Test", "[forif]" )
{
	Result ret;

	ret = Run( R"YT( a = 0; if(1) { a = 1; } else { a = 2; } exit(a); )YT" );
	REQUIRE( ret.code == 1 );

	ret = Run( R"YT( { f = 10; } if(f) { a = 1; } else { a = 2; } exit(a); )YT" );
	REQUIRE( !ret.build );

	ret = Run( R"YT( a = 0; if(0) { a = 1; } else { a = 2; } exit(a); )YT" );
	REQUIRE( ret.code == 2 );

	ret = Run( R"YT( for( ; ; ) break; )YT" );
	REQUIRE( ret.build );

	ret = Run( R"YT( a = 98102052; for(i=0; i<10; i++) { continue; a += i; } exit(a); )YT" );
	REQUIRE( ret.code == 98102052 );

	ret = Run( R"YT( a = 98100000; for(i=0; i<10; i++) { a += 2052; break; } exit(a); )YT" );
	REQUIRE( ret.code == 98102052 );
}

TEST_CASE( "Function Test", "[func]" )
{
	Result ret;

	ret = Run( R"YT( fn test(a, b, c) { d = a + b + c; return d; } exit(test(1,2,3)); )YT" );
	REQUIRE( ret.code == 6 );

	ret = Run( R"YT(
		fn test(a, b, c) { d = a + b + c; return d; }
		p = 1 + 30 * (3 + 2) / (5 + ((test))(1, -2, 3));
		g = []; g.resize(p+1);
		g[p] = 10; g[p]++;
		exit(g[p]); )YT" );
	REQUIRE( ret.code == 11 );
}

TEST_CASE( "Increment Decrement Test", "[incdec]" )
{
	Result ret;

	ret = Run( R"YT( 0++; )YT" );
	REQUIRE( !ret.build );
	ret = Run( R"YT( 1--; )YT" );
	REQUIRE( !ret.build );
	ret = Run( R"YT( (--(((8)))) )YT" );
	REQUIRE( !ret.build );
	ret = Run( R"YT( 9++; )YT" );
	REQUIRE( !ret.build );

	ret = Run( R"YT( "2"++; )YT" );
	REQUIRE( !ret.build );
	ret = Run( R"YT( "5"--; )YT" );
	REQUIRE( !ret.build );
	ret = Run( R"YT( --("0") )YT" );
	REQUIRE( !ret.build );
	ret = Run( R"YT( "2"++; )YT" );
	REQUIRE( !ret.build );

	ret = Run( R"YT( a = 9; exit(a++); )YT" );
	REQUIRE( ret.code == 9 );
	ret = Run( R"YT( a = 9; exit(++a); )YT" );
	REQUIRE( ret.code == 10 );
	ret = Run( R"YT( a = 9; exit(a--); )YT" );
	REQUIRE( ret.code == 9 );
	ret = Run( R"YT( a = 9; exit(--a); )YT" );
	REQUIRE( ret.code == 8 );

	ret = Run( R"YT( a = 2; exit(a++ + a++); )YT" );
	REQUIRE( ret.code == 5 );
	ret = Run( R"YT( a = 2; exit(((a))-- + a++); )YT" );
	REQUIRE( ret.code == 3 );
	ret = Run( R"YT( a = 2; b = a++ + a++; exit(a); )YT" );
	REQUIRE( ret.code == 4 );
	ret = Run( R"YT( a = 2; b = a-- + a++; exit(a); )YT" );
	REQUIRE( ret.code == 2 );

	ret = Run( R"YT( a = 2; exit((a-- - ((a--)))); )YT" );
	REQUIRE( ret.code == 1 );
	ret = Run( R"YT( a = 2; exit(a++ - a--); )YT" );
	REQUIRE( ret.code == -1 );
	ret = Run( R"YT( a = 2; b = a-- - a--; exit(a); )YT" );
	REQUIRE( ret.code == 0 );
	ret = Run( R"YT( a = 2; b = a++ - a--; exit(a); )YT" );
	REQUIRE( ret.code == 2 );

	ret = Run( R"YT( class IncDec { a = 7; }; ic = IncDec(); exit(ic.a++); )YT" );
	REQUIRE( ret.code == 7 );
	ret = Run( R"YT( class IncDec { a = 7; }; ic = IncDec(); exit(++ic.a); )YT" );
	REQUIRE( ret.code == 8 );
	ret = Run( R"YT( class IncDec { a = 7; }; ic = IncDec(); exit(ic.a--); )YT" );
	REQUIRE( ret.code == 7 );
	ret = Run( R"YT( class IncDec { a = 7; }; ic = IncDec(); exit(--ic.a); )YT" );
	REQUIRE( ret.code == 6 );

	ret = Run( R"YT( class IncDec { a = [1, 2]; }; ic = IncDec(); exit((ic.a)[1]++); )YT" );
	REQUIRE( ret.code == 2 );
	ret = Run( R"YT( class IncDec { a = [1, 2]; }; ic = IncDec(); exit(++(ic.a[1])); )YT" );
	REQUIRE( ret.code == 3 );
	ret = Run( R"YT( class IncDec { a = [1, 2]; }; ic = IncDec(); exit((ic.a[1])--); )YT" );
	REQUIRE( ret.code == 2 );
	ret = Run( R"YT( class IncDec { a = [1, 2]; }; ic = IncDec(); exit(--ic.a[1]); )YT" );
	REQUIRE( ret.code == 1 );

	ret = Run( R"YT( class IncDec { a = { "t1":1, "t2":2 }; }; ic = IncDec(); exit(ic.a["t1"]++); )YT" );
	REQUIRE( ret.code == 1 );
	ret = Run( R"YT( class IncDec { a = { "t1":1, "t2":2 }; }; ic = IncDec(); exit(++ic.a["t1"]); )YT" );
	REQUIRE( ret.code == 2 );
	ret = Run( R"YT( class IncDec { a = { "t1":1, "t2":2 }; }; ic = IncDec(); exit(ic.a["t1"]--); )YT" );
	REQUIRE( ret.code == 1 );
	ret = Run( R"YT( class IncDec { a = { "t1":1, "t2":2 }; }; ic = IncDec(); exit(--ic.a["t1"]); )YT" );
	REQUIRE( ret.code == 0 );
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
		a = file.open("test-ab4Nxq.txt", "w+");
		a.write(w);
		a.close();

		a.open("test-ab4Nxq.txt", "r");
		r = a.read(100);
		a.close();
		println("{w} {w.len()}");
		println("{r} {r.len()}");
		if(r != w) exit(1);
	)YT" );
	REQUIRE( ret.code == 0 );

	filesystem::remove("test-ab4Nxq.txt");
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
		jf = file.open("jsontest-ab4Nxq.txt", "w+");
		jf.write(json.dump(a, 2));
		jf.close();

		jf.open("jsontest-ab4Nxq.txt", "r");
		r = jf.read(10000);
		jf.close();

		b = json.parse(r);
		if(json.dump(a, 2) != json.dump(b, 2))
			exit(1);

		println(json.dump(b, 2));
	)YT" );
	REQUIRE( ret.code == 0 );

	filesystem::remove("jsontest-ab4Nxq.txt");
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
		class Ctor { _test = 15; }
		exit(Ctor().(_test));
	)YT" );
	REQUIRE( !ret.build );

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

TEST_CASE( "Includes Test", "[includes]" )
{
	string mod3y =
R"YT(mod3v = "mod3";
class Mod { name = ["mod3", "class"]; }
)YT";

	string mod2y =
R"YT(mod2v = "mod2 global";
class Mod2 { name = "mod2 class"; }
)YT";

	string mody =
R"YT(include mod2;
modv = "mod global " + mod2.mod2v;
class Mod { name = "mod class" + mod2.mod2v; }
println("here mod {modv}");
)YT";

	string appy =
R"YT(include mod;
include mod2;
include ylangd1.d2.mod3;
include ylangd1/d2.mod3;
include math;

app = "app test";
fn appTest(a, b) { println("{a} {b}"); }
class App { name = "app"; }

if(App().name != "app") exit(1);
if(mod.modv != "mod global " + mod2.mod2v) exit(2);
if(mod2.Mod2().name != "mod2 class") exit(3);
appTest(app, mod2.mod2v);

if(math.sqrt(4) != 2.0) exit(4);

if(ylangd1.d2.mod3.mod3v != "mod3") exit(5);
if(ylangd1.d2.mod3.Mod().name[0] != "mod3") exit(6);
if(d2.mod3.Mod().name[1] != "class") exit(7);
if(d2.mod3.mod3v != "mod3") exit(8);
)YT";

	ofstream fout("app.y");
	fout.write(appy.c_str(), appy.size());
	fout.close();

	fout.open("mod.y");
	fout.write(mody.c_str(), mody.size());
	fout.close();

	fout.open("mod2.y");
	fout.write(mod2y.c_str(), mod2y.size());
	fout.close();

	filesystem::create_directories("ylangd1/d2");
	fout.open("ylangd1/d2/mod3.y");
	fout.write(mod3y.c_str(), mod3y.size());
	fout.close();

	SECTION("main test")
	{
		Result ret;

		Compiler com;
		Program prg;
		auto errs = com.CompileFile("app.y", prg);
		REQUIRE(errs.empty());

		yvm::Machine m;
		int code = m.Run(prg);
		REQUIRE(!code);

		ret = Run( R"YT( include aaa+eee; )YT" );
		REQUIRE( !ret.build );
		ret = Run( R"YT( include aaa!eee; )YT" );
		REQUIRE( !ret.build );
		ret = Run( R"YT( include /ddpe/aaa.dpdpd/eee; )YT" );
		REQUIRE( !ret.build );
		ret = Run( R"YT( include /ddpe/../aaa.dpdpd..eee; )YT" );
		REQUIRE( !ret.build );

		ret = Run( R"YT( include ylangd1.d2.mod3; a = ylangd1.c; )YT" );
		REQUIRE( !ret.build );
		ret = Run( R"YT( include ylangd1.d2.mod3; a = ylangd1.d2.gg(); )YT" );
		REQUIRE( !ret.build );
	}

	filesystem::remove("app.y");
	filesystem::remove("mod.y");
	filesystem::remove("mod2.y");
	filesystem::remove_all("ylangd1");
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
	cfg.testsOrTags.push_back("[scanner],");
	cfg.testsOrTags.push_back("[exp],");
	cfg.testsOrTags.push_back("[forif],");
	cfg.testsOrTags.push_back("[func],");
	cfg.testsOrTags.push_back("[incdec],");
	cfg.testsOrTags.push_back("[logop],");
	cfg.testsOrTags.push_back("[primstr],");
	cfg.testsOrTags.push_back("[bltrand],");
	cfg.testsOrTags.push_back("[bltsys],");
	cfg.testsOrTags.push_back("[bltfile],");
	cfg.testsOrTags.push_back("[bltjson],");
	cfg.testsOrTags.push_back("[class],");
	cfg.testsOrTags.push_back("[includes],");

	int numFailed = _session.run();
};
