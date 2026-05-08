#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
using namespace std;

char current, lookedAhead;
int NLookedAhead = 0;

void ReadChar(void){
    if(NLookedAhead > 0){
        current = lookedAhead;
        NLookedAhead--;
    } else {
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
    }
}

void LookAhead(void){
    while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
    NLookedAhead++;
}

void Error(string s){
    cerr << s << endl;
    exit(-1);
}

set<char> declaredVars;

// Number := Digit{Digit}
void Number(void){
    if(current < '0' || current > '9')
        Error("Chiffre attendu");
    string num = "";
    while(current >= '0' && current <= '9'){
        num += current;
        ReadChar();
    }
    cout << "\tpush $" << num << endl;
}

void Expression(void);

// Factor := Number | Letter | "(" Expression ")" | "!" Factor
void Factor(void){
    if(current >= '0' && current <= '9'){
        Number();
    } else if(current >= 'a' && current <= 'z'){
        if(declaredVars.find(current) == declaredVars.end())
            Error(string("Variable non déclarée : ") + current);
        cout << "\tpush " << current << endl;
        ReadChar();
    } else if(current == '('){
        ReadChar();
        Expression();
        if(current != ')')
            Error("')' attendu");
        ReadChar();
    } else if(current == '!'){
        ReadChar();
        Factor();
        cout << "\tpop %rax" << endl;
        cout << "\tnotq %rax" << endl;
        cout << "\tpush %rax" << endl;
    } else {
        Error("Facteur attendu");
    }
}

// Term := Factor {MultiplicativeOperator Factor}
void Term(void){
    Factor();
    while(current=='*' || current=='/' || current=='%' ||
         (current=='&' && (LookAhead(), lookedAhead=='&'))){
        char op = current;
        if(op == '&'){
            ReadChar(); ReadChar(); // consume &&
            Factor();
            cout << "\tpop %rbx" << endl;
            cout << "\tpop %rax" << endl;
            cout << "\tandq %rbx, %rax" << endl;
            cout << "\tpush %rax" << endl;
        } else {
            ReadChar();
            Factor();
            cout << "\tpop %rbx" << endl;
            cout << "\tpop %rax" << endl;
            if(op == '*')
                cout << "\timulq %rbx" << endl;
            else if(op == '/'){
                cout << "\tcqto" << endl;
                cout << "\tidivq %rbx" << endl;
            } else if(op == '%'){
                cout << "\tcqto" << endl;
                cout << "\tidivq %rbx" << endl;
                cout << "\tpush %rdx" << endl;
                continue;
            }
            cout << "\tpush %rax" << endl;
        }
    }
}

// SimpleExpression := Term {AdditiveOperator Term}
void SimpleExpression(void){
    Term();
    while(current=='+' || current=='-' ||
         (current=='|' && (LookAhead(), lookedAhead=='|'))){
        char op = current;
        if(op == '|'){
            ReadChar(); ReadChar(); // consume ||
            Term();
            cout << "\tpop %rbx" << endl;
            cout << "\tpop %rax" << endl;
            cout << "\torq %rbx, %rax" << endl;
            cout << "\tpush %rax" << endl;
        } else {
            ReadChar();
            Term();
            cout << "\tpop %rbx" << endl;
            cout << "\tpop %rax" << endl;
            if(op == '+')
                cout << "\taddq %rbx, %rax" << endl;
            else
                cout << "\tsubq %rbx, %rax" << endl;
            cout << "\tpush %rax" << endl;
        }
    }
}

static int labelCount = 0;

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
void Expression(void){
    SimpleExpression();
    string relop = "";
    if(current == '=' || current == '!' || current == '<' || current == '>'){
        LookAhead();
        if(current=='=' && lookedAhead=='='){
            relop="=="; ReadChar(); ReadChar();
        } else if(current=='!' && lookedAhead=='='){
            relop="!="; ReadChar(); ReadChar();
        } else if(current=='<' && lookedAhead=='='){
            relop="<="; ReadChar(); ReadChar();
        } else if(current=='>' && lookedAhead=='='){
            relop=">="; ReadChar(); ReadChar();
        } else if(current=='<'){
            relop="<"; ReadChar();
        } else if(current=='>'){
            relop=">"; ReadChar();
        }
    }
    if(relop != ""){
        SimpleExpression();
        int lbl = labelCount++;
        cout << "\tpop %rbx" << endl;
        cout << "\tpop %rax" << endl;
        cout << "\tcmpq %rbx, %rax" << endl;
        string jump;
        if(relop=="==")      jump="je";
        else if(relop=="!=") jump="jne";
        else if(relop=="<")  jump="jl";
        else if(relop==">")  jump="jg";
        else if(relop=="<=") jump="jle";
        else if(relop==">=") jump="jge";
        cout << "\t" << jump << " Vrai" << lbl << endl;
        cout << "\tpush $0" << endl;
        cout << "\tjmp Suite" << lbl << endl;
        cout << "Vrai" << lbl << ":\tpush $-1" << endl;
        cout << "Suite" << lbl << ":" << endl;
    }
}

// AssignementStatement := Letter "=" Expression
void AssignementStatement(void){
    if(current < 'a' || current > 'z')
        Error("Variable attendue");
    if(declaredVars.find(current) == declaredVars.end())
        Error(string("Variable non déclarée : ") + current);
    char var = current;
    ReadChar();
    if(current != '=')
        Error("'=' attendu");
    ReadChar();
    Expression();
    cout << "\tpop " << var << endl;
}

// Statement := AssignementStatement
void Statement(void){
    AssignementStatement();
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
    Statement();
    while(current == ';'){
        ReadChar();
        Statement();
    }
    if(current != '.')
        Error("'.' attendu en fin de programme");
}

// DeclarationPart := "[" Letter {"," Letter} "]"
void DeclarationPart(void){
    if(current != '[')
        Error("'[' attendu");
    ReadChar();
    if(current < 'a' || current > 'z')
        Error("Variable attendue");
    declaredVars.insert(current);
    ReadChar();
    while(current == ','){
        ReadChar();
        if(current < 'a' || current > 'z')
            Error("Variable attendue");
        declaredVars.insert(current);
        ReadChar();
    }
    if(current != ']')
        Error("']' attendu");
    ReadChar();
}

int main(void){
    cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;

    ReadChar();

    // Declaration part (optional)
    if(current == '['){
        DeclarationPart();
        cout << "\t.data" << endl;
        for(char v : declaredVars)
            cout << v << ":\t.quad 0" << endl;
    }

    cout << "\t.text" << endl;
    cout << "\t.globl main" << endl;
    cout << "main:" << endl;
    cout << "\tmovq %rsp, %rbp" << endl;

    StatementPart();

    cout << "\tmovq %rbp, %rsp" << endl;
    cout << "\tret" << endl;

    char extra;
    if(cin.get(extra)){
        cerr << "Caractères en trop à la fin du programme : [" << extra << "]";
        Error(".");
    }
}
