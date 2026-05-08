%{
#include "tokeniser.h"
#include <iostream>
using namespace std;
%}
%option noyywrap
%option c++
%option yylineno
stringconst  \"[^\n"]+\"
ws      [ \t\n\r]+
alpha   [A-Za-z]
digit   [0-9]
number  {digit}+
id	{alpha}({alpha}|{digit})*
addop	(\+|\-|\|\|)
mulop	(\*|\/|%|\&\&)
relop	(\<|\>|"=="|\<=|\>=|!=)
unknown [^\"A-Za-z0-9 \n\r\t\(\)\<\>\=\!\%\&\|\}\-\;\.]+
%%
{addop}		return ADDOP;
{mulop}		return MULOP;
{relop}		return RELOP;
{number}	return NUMBER;
"IF"		return IFTOK;
"THEN"		return THENTOK;
"ELSE"		return ELSETOK;
"WHILE"		return WHILETOK;
"DO"		return DOTOK;
"FOR"		return FORTOK;
"TO"		return TOTOK;
"BEGIN"		return BEGINTOK;
"END"		return ENDTOK;
"DISPLAY"	return DISPLAYTOK;
{id}		return ID;
{stringconst}	return STRINGCONST;
"["		return RBRACKET;
"]"		return LBRACKET;
","		return COMMA;
";"		return SEMICOLON;
"."		return DOT;
":="		return ASSIGN;
"("		return RPARENT;
")"		return LPARENT;
"!"		return NOT;
<<EOF>>		return FEOF;
{ws}    {/* skip blanks and tabs */};
"(*"    { /* Skip comments between '(*' and '*)' */
		int c;
		while((c = yyinput()) != 0){
			if(c == '*'){
				if((c = yyinput()) == ')')
					break;
				else
					unput(c);
			}
		}
	};
{unknown}	return UNKNOWN;
%%
