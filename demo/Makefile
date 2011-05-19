DEMO_PARTS=start.bas frame.bas mainmenu.bas setup.bas monitor.bas vidmode.bas \
	palmap.bas category.bas vidsel.bas vidfilt.bas vidplay.bas random.bas

all: demo.bas genmodes.bas gencat.bas genvids.bas

demo.bas: $(DEMO_PARTS)
	cat $(DEMO_PARTS) > $@

genmodes.bas: mkgenmodes modes
	./$< modes > $@

gencat.bas: mkgencat categories
	./$< categories > $@

genvids.bas: mkgenvids videos
	./$< ./videos demo.img > $@
