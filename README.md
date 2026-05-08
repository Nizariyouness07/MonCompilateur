# MonCompilateur
A compiler from a Pascal-like language to 64-bit x86 assembly (AT&T syntax).

## Build and test
make
./test

## Personal work (beyond the base provided by the teacher)
- ForStatement : boucle FOR/TO/DO avec registre %r12 pour stocker la limite
- CaseStatement : instruction CASE/OF/ELSE/END avec labels multiples et CHAR
- Type DOUBLE : opérations flottantes SSE (addsd, subsd, mulsd, divsd)
- Type CHAR : déclaration .byte, affectation movb, affichage %c
- DISPLAY : gestion des 3 types INTEGER, DOUBLE, CHAR
- Vérification de types dans les affectations et expressions
- Opérations arithmétiques sur DOUBLE avec registres XMM

## Features implemented
- Typed variable declarations (INTEGER, BOOLEAN, DOUBLE, CHAR)
- Arithmetic operations (+, -, *, /, %)
- Boolean operations (&&, ||, !)
- Relational operators (==, !=, <, >, <=, >=)
- Type checking
- IF/THEN/ELSE
- WHILE/DO
- FOR/TO/DO
- BEGIN/END blocks
- DISPLAY instruction
- CASE/OF/ELSE/END with multiple labels
- Floating point (DOUBLE) with SSE instructions
- Character type (CHAR)

## Test examples
VAR a,b,c : INTEGER; x,y : DOUBLE; ok : BOOLEAN; ch : CHAR.
a:=10; b:=3; c:=a+b; DISPLAY c;
ok:=a>b; IF ok THEN DISPLAY a;
c:=0; WHILE c<5 DO BEGIN c:=c+1; DISPLAY c END;
FOR b:=1 TO 5 DO BEGIN c:=c+b END; DISPLAY c;
CASE a OF 5 : DISPLAY b; 10 : DISPLAY a ELSE DISPLAY c END;
x:=3.14; y:=2.0; DISPLAY x+y; DISPLAY x*y;
ch:='Z'; DISPLAY ch.
