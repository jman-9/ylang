#include "core/Scanner.h"
#include "core/StringInterpolator.h"
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
println("=== Langton's Ant (inline status line) ===");

W = 40;
H = 20;

fn clear_screen() { print("\033[H\033[J"); }
fn hide_cursor() { print("\033[?25l"); }
fn show_cursor() { print("\033[?25h"); }
fn move_cursor_home() { print("\033[H"); }

fn make_grid(w, h) {
    g = [];
    for (i = 0; i < h; i += 1) {
        row = [];
        for (j = 0; j < w; j += 1) {
            row.append(0);
        }
        g.append(row);
    }
    return g;
}

fn show_grid(g, ax, ay) {
    for (y = 0; y < g.len(); y += 1) {
        line = "";
        for (x = 0; x < g[0].len(); x += 1) {
            if (x == ax && y == ay) {
                line += "\033[33m@\033[0m";
            } else if (g[y][x] == 1) {
                line += "\033[37m#\033[0m";
            } else {
                line += "\033[90m.\033[0m";
            }
        }
        println(line);
    }
}

fn turn_right(d) { return (d + 1) % 4; }
fn turn_left(d)  { return (d + 3) % 4; }

fn dir_name(d) {
    if (d == 0) return "UP   ";
    if (d == 1) return "RIGHT";
    if (d == 2) return "DOWN ";
    return "LEFT ";
}

fn step(g, x, y, d) {
    c = g[y][x];
    if (c == 0) {
        d = turn_right(d);
        g[y][x] = 1;
    } else {
        d = turn_left(d);
        g[y][x] = 0;
    }

    if (d == 0) { y -= 1; }
    if (d == 1) { x += 1; }
    if (d == 2) { y += 1; }
    if (d == 3) { x -= 1; }

    if (x < 0) { x = g[0].len() - 1; }
    if (x >= g[0].len()) { x = 0; }
    if (y < 0) { y = g.len() - 1; }
    if (y >= g.len()) { y = 0; }

    return {'x': x, 'y': y, 'd': d};
}

// ----------------------------------------

grid = make_grid(W, H);
x = W / 2;
y = H / 2;
d = 0;

clear_screen();
hide_cursor();

for (t = 0; t < 500; t += 1) {
    move_cursor_home();
    show_grid(grid, x, y);

    println("\nstep={t}, pos=({x},{y}), dir={dir_name(d)}        ");

    s = step(grid, x, y, d);
    x = s['x'];
    y = s['y'];
    d = s['d'];
}

show_cursor();
println("done");


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
