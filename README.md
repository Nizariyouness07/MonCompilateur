# MonCompilateur
A compiler from a Pascal-like language to 64-bit x86 assembly (AT&T syntax).

## Build and test
make
./test

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

## Test example
VAR a,b,c : INTEGER; x,y : DOUBLE; ok : BOOLEAN; ch : CHAR.
a:=10; b:=3; c:=a+b; DISPLAY c;
ok:=a>b; IF ok THEN DISPLAY a;
c:=0; WHILE c<5 DO BEGIN c:=c+1; DISPLAY c END;
CASE a OF 5 : DISPLAY b; 10 : DISPLAY a ELSE DISPLAY c END;
x:=3.14; y:=2.0; DISPLAY x+y;
ch:='Z'; DISPLAY ch.
