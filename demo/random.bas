2500 TM=RND(TIMER) : REM ADD ENTROPY
2510 RV=RND(LOF(3))
2520 GET #3,RV
2530 INPUT #3,VN$,VM,VC,S0,S1,S2,S3
2540 GET #1,VM+1
2550 INPUT #1,MD$,PR,PC
2560 IF MN$="RGB" THEN PL=PR ELSE PL=PC
2570 IF PL=-1 THEN GOTO 2510
2580 MD=VM
2590 CLS
2600 GOSUB 2400
2610 PRINT "PRESS ANY KEY TO STOP"
2620 FOR I=3 TO 1 STEP -1
2630 PRINT I;"...";
2640 FOR J=1 TO 250
2650 IF INKEY$<>"" THEN RETURN
2660 NEXT J : NEXT I
2670 PRINT " NEXT VIDEO!"
2680 GOTO 2510
