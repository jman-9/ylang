class Scanner
{
public:
	Scanner();
	virtual ~Scanner();

	bool Scan();

protected:
};


/*
letter -> [a-zA-Z]
underscore -> _
letter_ -> (letter | underscore)
digit -> [0-9]
identifier -> letter_ (letter_ | digit)*
string -> "..." | '...'
comma -> ,
semicolon -> ;
colon -> :
dot -> .
quotation -> '
double_quotation -> "
left_parenthesis -> (
right_parenthesis -> )
left_brace -> {
right_brace -> }
left_bracket -> [
right_bracket -> ]
plus -> +
minus -> -
star -> *
slash -> /
percent -> %
plusplus -> ++
minusminus -> --
greater -> >
less -> <
greater_equal -> >=
less_equal -> <=
equal -> ==
notequal -> !=
ampersand -> &
pipe -> |
caret -> ^
left_shift -> <<
right_shift -> >>
not -> !
assign -> =
plus_assign -> +=
minus_assign -> -=
mul_assign -> *=
div_assign -> /=
mod_assign -> %=
*/