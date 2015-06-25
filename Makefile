USE_ENGINE = 1
USE_CHIPTUNE = 1
NO_SDCARD=1

NAME=mmat

GAMES = memmat painta simone
GAME_C_FILES = main.c song.c ticktock.c build/tmap.c common.c $(GAMES:%=%.c)
GAME_H_FILES = common.h $(GAMES:%=%.h)
GAME_BINARY_FILES = build/sprite.spr

include $(BITBOX)/lib/bitbox.mk

build/main.o : build/tmap.h

# sprite
build/sprite.spr: grabber_on.png grabber_off.png 
	@mkdir -p $(dir $@)
	python $(BITBOX)/scripts/couples_encode.py $@ $^ 

# tileset 
build/%.c build/%.h: %.tmx
	@mkdir -p $(dir $@)
	python $(BITBOX)/scripts/tmx.py -o $(dir $@) -c $^ > build/$*.h

# utiliser le linker direct ?
%_dat.o: %
	@mkdir -p $(dir $@)
	cd $(dir $<); xxd -i $(notdir $<) | sed "s/unsigned/const unsigned/" > $(*F).c
	$(CC) $(ALL_CFLAGS) $(AUTODEPENDENCY_CFLAGS) -c $*.c -o $@

