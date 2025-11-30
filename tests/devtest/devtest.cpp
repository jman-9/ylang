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
include rand;

class Character {
    _name = "";
    _hp = 100.0;
    _attack = 10.0;
    _defense = 3.0;
    _crit = 0.1;
    _alive = 1;

    fn Character(name, hp, attack, defense, crit) {
        _name = name;
        _hp = hp;
        _attack = attack;
        _defense = defense;
        _crit = crit;
        _alive = 1;
    }

    fn isAlive() {
        return _alive;
    }

    fn takeDamage(dmg) {
        _hp = _hp - dmg;
        if (_hp <= 0) {
            _hp = 0;
            _alive = 0;
        }
    }

    fn attackTarget(target) {
        if (!_alive) {
            println("{_name} is unable to attack.");
            return;
        }

        base = _attack;
        real = base;

        if (rand.get(0.0, 1.0) < _crit) {
            real = real * 2.0;
            println("{_name} lands a CRITICAL HIT!");
        }

        dmg = real - target._defense;
        if (dmg < 0) dmg = 0;

        println("{_name} attacks {target._name} for {dmg}");
        target.takeDamage(dmg);
    }

    fn status() {
        return "{_name} [HP={_hp}]";
    }
}

class Party {
    _members = [];

    fn add(c) {
        _members.append(c);
    }

    fn aliveCount() {
        count = 0;
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive()) count++;
        }
        return count;
    }

    fn getRandomAlive() {
        alive_list = [];
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive())
                alive_list.append(_members[i]);
        }

        if (alive_list.len() == 0)
            return null;

        idx = rand.get(0, alive_list.len() - 1);
        return alive_list[idx];
    }

    fn debug() {
        for (i = 0; i < _members.len(); i++) {
            println(_members[i].status());
        }
    }
}

class Battle {
    _partyA = 0;
    _partyB = 0;

    fn Battle(a, b) {
        _partyA = a;
        _partyB = b;
    }

    fn step() {
        if (_partyA.aliveCount() == 0 || _partyB.aliveCount() == 0) {
            return 0;
        }

        a = _partyA.getRandomAlive();
        b = _partyB.getRandomAlive();
        if (a && b) {
            a.attackTarget(b);
        }

        aa = _partyA.getRandomAlive();
        bb = _partyB.getRandomAlive();
        if (aa && bb) {
            bb.attackTarget(aa);
        }

        return 1;
    }

    fn run() {
        for (turn = 1; _partyA.aliveCount() > 0 && _partyB.aliveCount() > 0; turn++) {
            println("------ Turn {turn} ------");
            step();

            println("Party A:");
            _partyA.debug();

            println("Party B:");
            _partyB.debug();

            println("");
        }

        if (_partyA.aliveCount() > 0)
            println("Party A wins!");
        else
            println("Party B wins!");
    }
}

fn main() {
    rand.randomize_timer();

    a = Party();
    a.add(Character("Alice", 120.0, 12.0, 4.0, 0.2));
    a.add(Character("Bob",   100.0, 10.0, 3.0, 0.1));
    a.add(Character("Clara",  80.0, 15.0, 2.0, 0.3));

    b = Party();
    b.add(Character("Orc",    130.0, 11.0, 3.0, 0.15));
    b.add(Character("Goblin",  70.0,  8.0, 1.0, 0.05));
    b.add(Character("Troll",  160.0, 14.0, 4.0, 0.12));

    battle = Battle(a, b);
    battle.run();
}
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
