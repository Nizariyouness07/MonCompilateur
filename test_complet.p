VAR
    a,b,c : INTEGER;
    x,y : DOUBLE;
    ok : BOOLEAN;
    ch : CHAR.
a:=10;
b:=3;
c:=a+b;
DISPLAY c;
c:=a-b;
DISPLAY c;
c:=a*b;
DISPLAY c;
c:=a/b;
DISPLAY c;
ok:=a>b;
IF ok THEN DISPLAY a;
c:=0;
WHILE c<5 DO BEGIN c:=c+1; DISPLAY c END;
c:=0;
FOR b:=1 TO 5 DO BEGIN c:=c+b END;
DISPLAY c;
CASE a OF
    5 : DISPLAY b;
    10 : DISPLAY a
ELSE DISPLAY c
END;
x:=3.14;
y:=2.0;
DISPLAY x+y;
DISPLAY x*y;
ch:='Z';
DISPLAY ch.
