out := _out
LDFLAGS := $(shell pkg-config --libs libzip)
CFLAGS := -g -Wall -Werror

all: $(out)/epub-zip

$(out)/%: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

# make _out/epub-zip.valgrind o='_out/3.zip epub-zip.c'
%.valgrind: %
	-valgrind --log-file=$(out)/.vgdump --tool=memcheck --leak-check=yes --show-reachable=yes ./$* $(o)
	$(EDITOR) $(out)/.vgdump
