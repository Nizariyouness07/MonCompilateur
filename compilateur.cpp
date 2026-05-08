#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>
#include <map>
#include <vector>


using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND, WTFM};
enum TYPE {UNSIGNED_INT, BOOLEAN, DOUBLE, CHAR};

TOKEN current;

FlexLexer* lexer = new yyFlexLexer;

set<string> DeclaredVariables;
map<string, TYPE> VariableType;
unsigned long TagNumber=0;

bool IsDeclared(const char *id){
	return DeclaredVariables.find(id)!=DeclaredVariables.end();
}

void Error(string s){
	cerr << "Ligne n°"<<lexer->lineno()<<", lu : '"<<lexer->YYText()<<"'("<<current<<"), mais ";
	cerr<< s << endl;
	exit(-1);
}

TYPE Identifier(void){
    string name = lexer->YYText();
    TYPE type = UNSIGNED_INT;
    if(VariableType.find(name)!=VariableType.end())
        type=VariableType[name];
    if(type==DOUBLE){
        cout << "\tsubq $8, %rsp" << endl;
        cout << "\tmovq " << name << ", %rax" << endl;
        cout << "\tmovq %rax, (%rsp)" << endl;
    } else if(type==CHAR){
        cout << "\tmovzbq " << name << ", %rax" << endl;
        cout << "\tpush %rax" << endl;
    } else {
        cout << "\tpush " << name << endl;
    }
    current=(TOKEN) lexer->yylex();
    return type;
}


TYPE Number(void){
    cout <<"\tpush $"<<atoi(lexer->YYText())<<endl;
    current=(TOKEN) lexer->yylex();
    return UNSIGNED_INT;
}

TYPE FloatNumber(void){
    double f=atof(lexer->YYText());
    long long unsigned int *i=(long long unsigned int *)&f;
    cout << "\tsubq $8, %rsp" << endl;
    cout << "\tmovq $"<<*i<<", %rax\t# empile le flottant "<<f<<endl;
    cout << "\tmovq %rax, (%rsp)" << endl;
    current=(TOKEN) lexer->yylex();
    return DOUBLE;
}


TYPE CharConst(void){
    char c=lexer->YYText()[1]; // 'a' -> a
    cout <<"\tpush $"<<(int)c<<"\t# empile le caractère '"<<c<<"'"<<endl;
    current=(TOKEN) lexer->yylex();
    return CHAR;
}

TYPE Expression(void);
TYPE Factor(void){
    TYPE type;
    if(current==RPARENT){
        current=(TOKEN) lexer->yylex();
        type=Expression();
        if(current!=LPARENT)
            Error("')' était attendu");
        else
            current=(TOKEN) lexer->yylex();
    }
    else if(current==NUMBER)
        type=Number();
    else if(current==FLOATNUMBER)
        type=FloatNumber();
    else if(current==CHARCONST)
        type=CharConst();
    else if(current==ID)
        type=Identifier();
    else
        Error("'(' ou chiffre ou lettre attendue");
    return type;
}
OPMUL MultiplicativeOperator(void);

TYPE Term(void){
	OPMUL mulop;
	TYPE type, type2;
	type=Factor();
	while(current==MULOP){
		mulop=MultiplicativeOperator();
		type2=Factor();
		if(type!=type2)
			Error("Types incompatibles dans Term");
		if(type==DOUBLE){
			cout << "\tmovq (%rsp), %rax" << endl;
			cout << "\taddq $8, %rsp" << endl;
			cout << "\tmovq %rax, -8(%rsp)" << endl;
			cout << "\tmovsd -8(%rsp), %xmm1" << endl;
			cout << "\tmovq (%rsp), %rax" << endl;
			cout << "\taddq $8, %rsp" << endl;
			cout << "\tmovq %rax, -8(%rsp)" << endl;
			cout << "\tmovsd -8(%rsp), %xmm0" << endl;
			switch(mulop){
				case MUL:
					cout << "\tmulsd %xmm1, %xmm0" << endl;
					break;
				case DIV:
					cout << "\tdivsd %xmm1, %xmm0" << endl;
					break;
				default:
					Error("opérateur multiplicatif inconnu pour DOUBLE");
			}
			cout << "\tsubq $8, %rsp" << endl;
			cout << "\tmovsd %xmm0, (%rsp)" << endl;
		} else {
			cout << "\tpop %rbx"<<endl;
			cout << "\tpop %rax"<<endl;
			switch(mulop){
				case AND:
					cout << "\tmulq\t%rbx"<<endl;
					cout << "\tpush %rax\t# AND"<<endl;
					break;
				case MUL:
					cout << "\tmulq\t%rbx"<<endl;
					cout << "\tpush %rax\t# MUL"<<endl;
					break;
				case DIV:
					cout << "\tmovq $0, %rdx"<<endl;
					cout << "\tdiv %rbx"<<endl;
					cout << "\tpush %rax\t# DIV"<<endl;
					break;
				case MOD:
					cout << "\tmovq $0, %rdx"<<endl;
					cout << "\tdiv %rbx"<<endl;
					cout << "\tpush %rdx\t# MOD"<<endl;
					break;
				default:
					Error("opérateur multiplicatif attendu");
			}
		}
	}
	return type;
}






OPMUL MultiplicativeOperator(void){
	OPMUL opmul;
	if(strcmp(lexer->YYText(),"*")==0) opmul=MUL;
	else if(strcmp(lexer->YYText(),"/")==0) opmul=DIV;
	else if(strcmp(lexer->YYText(),"%")==0) opmul=MOD;
	else if(strcmp(lexer->YYText(),"&&")==0) opmul=AND;
	else opmul=WTFM;
	current=(TOKEN) lexer->yylex();
	return opmul;
}





OPADD AdditiveOperator(void){
	OPADD opadd;
	if(strcmp(lexer->YYText(),"+")==0) opadd=ADD;
	else if(strcmp(lexer->YYText(),"-")==0) opadd=SUB;
	else if(strcmp(lexer->YYText(),"||")==0) opadd=OR;
	else opadd=WTFA;
	current=(TOKEN) lexer->yylex();
	return opadd;
}

TYPE SimpleExpression(void){
    OPADD adop;
    TYPE type, type2;
    type=Term();
    while(current==ADDOP){
        adop=AdditiveOperator();
        type2=Term();
        if(type!=type2)
            Error("Types incompatibles dans SimpleExpression");
        if(type==DOUBLE){
            cout << "\tmovq (%rsp), %rax" << endl;
            cout << "\taddq $8, %rsp" << endl;
            cout << "\tmovq %rax, -8(%rsp)" << endl;
            cout << "\tmovsd -8(%rsp), %xmm1" << endl;
            cout << "\tmovq (%rsp), %rax" << endl;
            cout << "\taddq $8, %rsp" << endl;
            cout << "\tmovq %rax, -8(%rsp)" << endl;
            cout << "\tmovsd -8(%rsp), %xmm0" << endl;
            switch(adop){
                case ADD:
                    cout << "\taddsd %xmm1, %xmm0" << endl;
                    break;
                case SUB:
                    cout << "\tsubsd %xmm1, %xmm0" << endl;
                    break;
                default:
                    Error("opérateur additif inconnu pour DOUBLE");
            }
            cout << "\tsubq $8, %rsp" << endl;
            cout << "\tmovsd %xmm0, (%rsp)" << endl;
        } else {
            cout << "\tpop %rbx" << endl;
            cout << "\tpop %rax" << endl;
            switch(adop){
                case OR:
                    cout << "\taddq\t%rbx, %rax\t# OR" << endl;
                    break;
                case ADD:
                    cout << "\taddq\t%rbx, %rax\t# ADD" << endl;
                    break;
                case SUB:
                    cout << "\tsubq\t%rbx, %rax\t# SUB" << endl;
                    break;
                default:
                    Error("opérateur additif inconnu");
            }
            cout << "\tpush %rax" << endl;
        }
    }
    return type;
}




void DeclarationPart(void){
	if(current!=VARTOK)
		Error("VAR attendu");
	cout << "\t.data" << endl;
	cout << "\t.align 8" << endl;
        cout << "FormatString1:\t.string \"%llu\\n\"" << endl;
cout << "FormatString2:\t.string \"%f\\n\"" << endl;
cout << "FormatString3:\t.string \"%c\\n\"" << endl;       
 	current=(TOKEN) lexer->yylex();
	// VarDeclaration := Ident {"," Ident} ":" Type
	do {
		vector<string> vars;
		if(current!=ID)
			Error("Identificateur attendu");
		vars.push_back(lexer->YYText());
		DeclaredVariables.insert(lexer->YYText());
		current=(TOKEN) lexer->yylex();
		while(current==COMMA){
			current=(TOKEN) lexer->yylex();
			if(current!=ID)
				Error("Identificateur attendu");
			vars.push_back(lexer->YYText());
			DeclaredVariables.insert(lexer->YYText());
			current=(TOKEN) lexer->yylex();
		}
		if(current!=COLON)
			Error("':' attendu");
		current=(TOKEN) lexer->yylex();
		TYPE type;
                if(current==INTEGERTOK)
    type=UNSIGNED_INT;
else if(current==BOOLEANTOK)
    type=BOOLEAN;
else if(current==DOUBLETOK)
    type=DOUBLE;
else if(current==CHARTOK)
    type=CHAR;
else
    Error("Type attendu (INTEGER, BOOLEAN, DOUBLE ou CHAR)");
for(string v : vars){
    if(type==DOUBLE)
        cout << v << ":\t.double 0.0" << endl;
    else if(type==CHAR)
        cout << v << ":\t.byte 0" << endl;
    else
        cout << v << ":\t.quad 0" << endl;
    VariableType[v]=type;
}		


            current=(TOKEN) lexer->yylex();
	} while(current==SEMICOLON && (current=(TOKEN)lexer->yylex(), true));
	if(current!=DOT)
		Error("'.' attendu");
	current=(TOKEN) lexer->yylex();
}



OPREL RelationalOperator(void){
	OPREL oprel;
	if(strcmp(lexer->YYText(),"==")==0) oprel=EQU;
	else if(strcmp(lexer->YYText(),"!=")==0) oprel=DIFF;
	else if(strcmp(lexer->YYText(),"<")==0) oprel=INF;
	else if(strcmp(lexer->YYText(),">")==0) oprel=SUP;
	else if(strcmp(lexer->YYText(),"<=")==0) oprel=INFE;
	else if(strcmp(lexer->YYText(),">=")==0) oprel=SUPE;
	else oprel=WTFR;
	current=(TOKEN) lexer->yylex();
	return oprel;
}

TYPE Expression(void){
	OPREL oprel;
	TYPE type, type2;
	type=SimpleExpression();
	if(current==RELOP){
		oprel=RelationalOperator();
		type2=SimpleExpression();
		if(type!=type2)
			Error("Types incompatibles dans Expression");
		cout << "\tpop %rax"<<endl;
		cout << "\tpop %rbx"<<endl;
		cout << "\tcmpq %rax, %rbx"<<endl;
		switch(oprel){
			case EQU:
				cout << "\tje Vrai"<<++TagNumber<<"\t# If equal"<<endl;
				break;
			case DIFF:
				cout << "\tjne Vrai"<<++TagNumber<<"\t# If different"<<endl;
				break;
			case SUPE:
				cout << "\tjae Vrai"<<++TagNumber<<"\t# If above or equal"<<endl;
				break;
			case INFE:
				cout << "\tjbe Vrai"<<++TagNumber<<"\t# If below or equal"<<endl;
				break;
			case INF:
				cout << "\tjb Vrai"<<++TagNumber<<"\t# If below"<<endl;
				break;
			case SUP:
				cout << "\tja Vrai"<<++TagNumber<<"\t# If above"<<endl;
				break;
			default:
				Error("Opérateur de comparaison inconnu");
		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<TagNumber<<endl;
		cout << "Vrai"<<TagNumber<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;
		cout << "Suite"<<TagNumber<<":"<<endl;
		return BOOLEAN;
	}
	return type;
}

void AssignementStatement(void){
	string variable;
	if(current!=ID)
		Error("Identificateur attendu");
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	variable=lexer->YYText();
	current=(TOKEN) lexer->yylex();
	if(current!=ASSIGN)
		Error("caractères ':=' attendus");
	current=(TOKEN) lexer->yylex();

        TYPE type=Expression();
        TYPE vartype=UNSIGNED_INT;
          if(VariableType.find(variable)!=VariableType.end())
             vartype=VariableType[variable];
          if(type!=vartype)
    Error("Type incompatible dans l'affectation : types différents");
if(type==DOUBLE){
    cout << "\tpop %rax" << endl;
    cout << "\tmovq %rax, " << variable << endl;
} else if(type==CHAR){
    cout << "\tpop %rax" << endl;
    cout << "\tmovb %al, " << variable << endl;
} else {
    cout << "\tpop " << variable << endl;
}
}
void IfStatement(void);
void WhileStatement(void);
void ForStatement(void);
void BlockStatement(void);
void CaseStatement(void);



void DisplayStatement(void){
    TYPE type;
    current=(TOKEN) lexer->yylex();
    type=Expression();
    if(type==UNSIGNED_INT){
        cout << "\tpop %rdx" << endl;
        cout << "\tmovq $FormatString1, %rsi" << endl;
        cout << "\tmovl $1, %edi" << endl;
        cout << "\tmovl $0, %eax" << endl;
        cout << "\tcall __printf_chk@PLT" << endl;
        
} else if(type==DOUBLE){
    cout << "\tpop %rax" << endl;
    cout << "\tmovq %rax, -8(%rsp)" << endl;
    cout << "\tmovsd -8(%rsp), %xmm0" << endl;
    cout << "\tmovq $FormatString2, %rdi" << endl;
    cout << "\tmovl $1, %eax" << endl;
    cout << "\tsubq $8, %rsp" << endl;
    cout << "\tcall printf@PLT" << endl;
    cout << "\taddq $8, %rsp" << endl;
}
 else if(type==CHAR){
        cout << "\tpop %rax" << endl;
        cout << "\tmovzbq %al, %rdx" << endl;
        cout << "\tmovq $FormatString3, %rsi" << endl;
        cout << "\tmovl $1, %edi" << endl;
        cout << "\tmovl $0, %eax" << endl;
        cout << "\tcall __printf_chk@PLT" << endl;
    } else {
        Error("DISPLAY attend un INTEGER, DOUBLE ou CHAR");
    }
}
void Statement(void);

void CaseStatement(void){
	unsigned long tag=++TagNumber;
	unsigned long endtag=tag;
	current=(TOKEN) lexer->yylex();
	TYPE type=Expression();
	if(current!=OFTOK)
		Error("OF attendu");
	current=(TOKEN) lexer->yylex();
	cout << "\tpop %rax" << endl;
	while(current!=ENDTOK && current!=ELSETOK && current!=FEOF){
		unsigned long casetag=++TagNumber;
		do {
			if(current==NUMBER){
				cout << "\tcmpq $" << atoi(lexer->YYText()) << ", %rax" << endl;
				current=(TOKEN) lexer->yylex();
			} else if(current==CHARCONST){
				char c=lexer->YYText()[1];
				cout << "\tcmpq $" << (int)c << ", %rax" << endl;
				current=(TOKEN) lexer->yylex();
			} else {
				Error("Constante attendue dans CASE");
			}
			cout << "\tje CaseMatch" << casetag << endl;
			if(current==COMMA)
				current=(TOKEN) lexer->yylex();
			else
				break;
		} while(true);
		cout << "\tjmp CaseNext" << casetag << endl;
		cout << "CaseMatch" << casetag << ":" << endl;
		if(current!=COLON)
			Error("':' attendu après les labels du CASE");
		current=(TOKEN) lexer->yylex();
		Statement();
		cout << "\tjmp CaseEnd" << endtag << endl;
		cout << "CaseNext" << casetag << ":" << endl;
		if(current==SEMICOLON)
			current=(TOKEN) lexer->yylex();
	}
	if(current==ELSETOK){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=ENDTOK)
		Error("END attendu");
	current=(TOKEN) lexer->yylex();
	cout << "CaseEnd" << endtag << ":" << endl;
}





void Statement(void){
	if(current==IFTOK)
		IfStatement();
	else if(current==WHILETOK)
		WhileStatement();
	else if(current==FORTOK)
		ForStatement();
	else if(current==BEGINTOK)
		BlockStatement();
	else if(current==DISPLAYTOK)
		DisplayStatement();
	else if(current==CASETOK)
		CaseStatement();
	else
		AssignementStatement();
}






void IfStatement(void){
	unsigned long tag=++TagNumber;
	current=(TOKEN) lexer->yylex();
	TYPE type=Expression();
	if(type!=BOOLEAN)
		Error("Expression booléenne attendue dans IF");
	cout << "\tpop %rax" << endl;
	cout << "\tcmpq $0, %rax" << endl;
	cout << "\tje Faux" << tag << endl;
	if(current!=THENTOK)
		Error("THEN attendu");
	current=(TOKEN) lexer->yylex();
	Statement();
	cout << "\tjmp Suite" << tag << endl;
	cout << "Faux" << tag << ":" << endl;
	if(current==ELSETOK){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	cout << "Suite" << tag << ":" << endl;
}

void WhileStatement(void){
	unsigned long tag=++TagNumber;
	cout << "While" << tag << ":" << endl;
	current=(TOKEN) lexer->yylex();
	TYPE type=Expression();
	if(type!=BOOLEAN)
		Error("Expression booléenne attendue dans WHILE");
	cout << "\tpop %rax" << endl;
	cout << "\tcmpq $0, %rax" << endl;
	cout << "\tje EndWhile" << tag << endl;
	if(current!=DOTOK)
		Error("DO attendu");
	current=(TOKEN) lexer->yylex();
	Statement();
	cout << "\tjmp While" << tag << endl;
	cout << "EndWhile" << tag << ":" << endl;
}

void ForStatement(void){
	unsigned long tag=++TagNumber;
	current=(TOKEN) lexer->yylex();
	if(current!=ID)
		Error("Identificateur attendu");
	string loopvar = lexer->YYText();
	AssignementStatement();
	if(current!=TOTOK)
		Error("TO attendu");
	current=(TOKEN) lexer->yylex();
	Expression();
	cout << "\tpop %r12" << endl;
	cout << "For" << tag << ":" << endl;
	cout << "\tpush " << loopvar << endl;
	cout << "\tpop %rax" << endl;
	cout << "\tcmpq %r12, %rax" << endl;
	cout << "\tjg EndFor" << tag << endl;
	if(current!=DOTOK)
		Error("DO attendu");
	current=(TOKEN) lexer->yylex();
	Statement();
	cout << "\tpush " << loopvar << endl;
	cout << "\tpush $1" << endl;
	cout << "\tpop %rbx" << endl;
	cout << "\tpop %rax" << endl;
	cout << "\taddq %rbx, %rax" << endl;
	cout << "\tpush %rax" << endl;
	cout << "\tpop " << loopvar << endl;
	cout << "\tjmp For" << tag << endl;
	cout << "EndFor" << tag << ":" << endl;
}


void BlockStatement(void){
	current=(TOKEN) lexer->yylex();
	Statement();
	while(current==SEMICOLON){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=ENDTOK)
		Error("END attendu");
	current=(TOKEN) lexer->yylex();
}

void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==SEMICOLON){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=DOT)
		Error("caractère '.' attendu");
	current=(TOKEN) lexer->yylex();
}

void Program(void){
        if(current==VARTOK)
		DeclarationPart();
	StatementPart();
}

int main(void){
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	current=(TOKEN) lexer->yylex();
	Program();
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(current!=FEOF){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error(".");
	}
}
