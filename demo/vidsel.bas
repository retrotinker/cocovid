1900 SV=1
1910 GOSUB 2200
1920 GOSUB 1000
1930 PRINT @ 48-(LEN(CT$)/2), CT$;
1940 IF NV>0 THEN 1960
1950 I=1 : GOTO 2000
1960 FOR I=1 TO NV
1970 PRINT @ 67+I*32, I;"- ";VF$(I);
1980 NEXT I
1990 IF NV = 8 THEN GOTO 2060
2000 PRINT @ 67+I*32, I;"- CATEGORY MENU";
2010 PRINT @ 132+I*32, "CHOICE";
2020 INPUT CH : IF CH = I THEN RETURN
2030 IF CH < 1 OR CH > I+1 THEN GOTO 1920 ELSE VD=VF(CH)
2040 GOSUB 2160
2050 GOTO 1920
2060 PRINT @ 355, I;"- MORE VIDEOS";
2070 PRINT @ 386, I+1;"- CATEGORY MENU";
2080 PRINT @ 452, "CHOICE";
2090 INPUT CH : IF CH = I+1 THEN RETURN
2100 IF CH = I THEN GOTO 2140
2110 IF CH < 1 OR CH > I+2 THEN GOTO 1920 ELSE VD=VF(CH)
2120 GOSUB 2160
2130 GOTO 1920
2140 SV=EV
2150 GOTO 1910
2160 GET #3,VD
2170 INPUT #3,VN$,VM,VC,S0,S1,S2,S3
2180 GOSUB 2400
2190 RETURN
