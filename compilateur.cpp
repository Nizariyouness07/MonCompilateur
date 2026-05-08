#include <string>
#include <iostream>
#include <cstdlib>
using namespace std;

char current;
char nextcar;
bool hasnext = false;

void ReadChar(void){
    while(cin.get(current) && (current==' '||current=='\t'||current=='\n'))
        cin.get(current);
    // Peek at next non-space character
    hasnext = false;
    char tmp;
    if(cin.get(tmp)){
        if(tmp==' '||tmp=='\t'||tmp=='\n'){
            // skip spaces for nextcar
            while(cin.get(tmp) && (tmp==' '||tmp=='\t'||tmp=='\n'));
        }
        nextcar = tmp;
        hasnext = true;
        cin.putback(nextcar);
    }
}

void Error(string s){
    cerr<< s << endl;
    exit(-1);
}

void AdditiveOperator(void){
    if(current=='+'||current=='-')
        ReadChar();
    else
        Error("Opérateur additif attendu");
}

void Digit(void){
    if((current<'0')||(current>'9'))
        Error("Chiffre attendu");
    else{
        cout << "\tpush $"<<current<<endl;
        ReadChar();
    }
}

void ArithmeticExpression(void);

void Term(void){
    if(current=='('){
        ReadChar();
        ArithmeticExpression();
        if(current!=')')
            Error("')' était attendu");
        else
            ReadChar();
    }
    else if(current>='0' && current<='9')
        Digit();
    else
        Error("'(' ou chiffre attendu");
}

void ArithmeticExpression(void){
    char adop;
    Term();
    while(current=='+'||current=='-'){
        adop=current;
        AdditiveOperator();
        Term();
        cout << "\tpop %rbx"<<endl;
        cout << "\tpop %rax"<<endl;
        if(adop=='+')
            cout << "\taddq\t%rbx, %rax"<<endl;
        else
            cout << "\tsubq\t%rbx, %rax"<<endl;
        cout << "\tpush %rax"<<endl;
    }
}

static int labelCount = 0;

void Expression(void){
    ArithmeticExpression();
    if(current=='=' || current=='<' || current=='>'){
        char op1 = current;
        char op2 = nextcar;
        string relop = "";

        if(op1=='=' ){
            relop = "=";
            ReadChar();
        } else if(op1=='<' && op2=='>'){
            relop = "<>";
            ReadChar(); ReadChar();
        } else if(op1=='<' && op2=='='){
            relop = "<=";
            ReadChar(); ReadChar();
        } else if(op1=='>' && op2=='='){
            relop = ">=";
            ReadChar(); ReadChar();
        } else if(op1=='<'){
            relop = "<";
            ReadChar();
        } else if(op1=='>'){
            relop = ">";
            ReadChar();
        }

        ArithmeticExpression();

        int lbl = labelCount++;
        cout << "\tpop %rbx"<<endl;
        cout << "\tpop %rax"<<endl;
        cout << "\tcmpq %rbx, %rax"<<endl;

        string jumpTrue, jumpFalse;
        if(relop=="=")       { jumpTrue="je";  }
        else if(relop=="<>") { jumpTrue="jne"; }
        else if(relop=="<")  { jumpTrue="jl";  }
        else if(relop=="<=") { jumpTrue="jle"; }
        else if(relop==">")  { jumpTrue="jg";  }
        else if(relop==">=") { jumpTrue="jge"; }

        cout << "\t" << jumpTrue << " Vrai"<<lbl<<endl;
        cout << "\tpush $0"<<endl;
        cout << "\tjmp Suite"<<lbl<<endl;
        cout << "Vrai"<<lbl<<":\tpush $1"<<endl;
        cout << "Suite"<<lbl<<":"<<endl;
    }
}

int main(void){
    cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
    cout << "\t.text\t\t# The following lines contain the program"<<endl;
    cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
    cout << "main:\t\t\t# The main function body :"<<endl;
    cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
    ReadChar();
    Expression();
    cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
    cout << "\tret\t\t\t# Return from main function"<<endl;
    if(cin.get(current)){
        cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
        Error(".");
    }
}
