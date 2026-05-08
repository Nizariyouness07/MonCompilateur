%{
#include "tokeniser.h"
#include <iostream>
using namespace std;
%}
%option noyywrap
%option c++
%option yylineno
stringconst  \"[^\n"]+\"
charconst    \'[^\n']\'
ws      [ \t\n\r]+
alpha   [A-Za-z]
digit   [0-9]
number  {digit}+
floatnumber {digit}+\.{digit}+
id	{alpha}({alpha}|{digit})*
addop	(\+|\-|\|\|)
mulop	(\*|\/|%|\&\&)
relop	(\<|\>|"=="|\<=|\>=|!=)
unknown [^\"A-Za-z0-9 \n\r\t\(\)\<\>\=\!\%\&\|\}\-\;\.]+
%%
{addop}		return ADDOP;
{mulop}		return MULOP;
{relop}		return RELOP;
{floatnumber}	return FLOATNUMBER;
{number}	return NUMBER;
{charconst}	return CHARCONST;
"VAR"		return VARTOK;
"INTEGER"	return INTEGERTOK;
"BOOLEAN"	return BOOLEANTOK;
"DOUBLE"	return DOUBLETOK;
"CHAR"		return CHARTOK;
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
"CASE"		return CASETOK;
"OF"		return OFTOK;
{id}		return ID;
{stringconst}	return STRINGCONST;
"["		return RBRACKET;
"]"		return LBRACKET;
","		return COMMA;
";"		return SEMICOLON;
"."		return DOT;
":="		return ASSIGN;
":"		return COLON;
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
