1200 GOSUB 1000
1210 GOSUB 1600
1220 PRINT @ 139, "SETUP MENU";
1230 PRINT @ 201, "1 - MONITOR";
1240 PRINT @ 233, "2 - VIDEO MODE";
1250 PRINT @ 265, "3 - MAIN MENU";
1260 PRINT @ 329, "CHOICE";
1270 INPUT CH : IF CH = 3 AND PL <> -1 THEN RETURN
1280 ON CH GOSUB 1300,1400
1290 GOTO 1200