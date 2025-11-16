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



static std::vector<Error> Build(const std::string& src, Bytecode& retBytecode)
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
		if(!bb.Build(*ast, retBytecode))
		{//TODO trace
			throw 'n';
		}
	} while(0);

	return errs;
}


static pair<int, vector<Error>> Run(const std::string& src)
{
	vector<Error> errs;

	Bytecode c;
	errs = Build(src, c);
	if(!errs.empty())
		return { 1, errs };

	yvm::Machine m;
	return { m.Run(c), errs };
}


TEST_CASE( "Primitive String Test", "[primstr]" )
{
	pair<int, vector<Error>> ret;

	ret = Run( R"YT( a = 'hello'; if(a.len() != 5) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = 'a b c d t'; if(a.find(' c d') != 3) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = 'pika pika chu'; if(a.substr(5, 4) + a.substr(10) != 'pikachu') exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = 'hello world'; a = a.replace('world', 'ylang'); if(a != 'hello ylang') exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = 'a b c d t'; if(a.split().len() != 5) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = '\r \n \t \v 12345 7\t9 \r \n \t \v '; if(a.trim().len() != 9) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = '  \t  12345 7\t9 \t\n'; if(a.ltrim().len() != 12) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( a = ' \v 12345 7\t9 \t\n\r\v '; if(a.rtrim().len() != 12) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( delim = '::'; list = ['aa' , 'bb', 'cc', 'dd']; if(delim.join(list) != 'aa::bb::cc::dd') exit(1); )YT" );
	REQUIRE( ret.first == 0 );
}

TEST_CASE( "Builtin Random Test", "[bltrand]" )
{
	pair<int, vector<Error>> ret;

	ret = Run( R"YT( include rand; rand.randomize_timer(); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( include rand; for(i=0; i<20000; i+=1) { r=rand.get(-2052, 9810); if(!(-2052 <= r && r <= 9810)) exit(1); } )YT" );
	REQUIRE( ret.first == 0 );
}

TEST_CASE( "Builtin Sys Test", "[bltsys]" )
{
	pair<int, vector<Error>> ret;

	ret = Run( R"YT( include sys; println(sys.version); if(sys.version.empty()) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( include sys; println(sys.executable); if(sys.executable.empty()) exit(1); )YT" );
	REQUIRE( ret.first == 0 );

	ret = Run( R"YT( include sys; println(sys.argv); )YT" );
	REQUIRE( ret.first == 0 );
}

TEST_CASE( "Builtin File Test", "[bltfile]" )
{
	pair<int, vector<Error>> ret;

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
	REQUIRE( ret.first == 0 );
}

TEST_CASE( "Builtin Json Test", "[bltjson]" )
{
	pair<int, vector<Error>> ret;

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
	REQUIRE( ret.first == 0 );
}


static const Catch::LeakDetector leakDetector;

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	(void)&leakDetector;

	yrun::ArgsCollector::Collect(argc, argv);

	static Catch::Session _session;
	Catch::ConfigData& cfg = _session.configData();

	cfg.showSuccessfulTests = true;
	//cfg.testsOrTags.push_back("[primstr]");
	//cfg.testsOrTags.push_back("[bltrand]");
	//cfg.testsOrTags.push_back("[bltsys]");
	//cfg.testsOrTags.push_back("[bltfile]");
	cfg.testsOrTags.push_back("[bltjson]");

	int numFailed = _session.run();
};
