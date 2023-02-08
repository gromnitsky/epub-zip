out := _out
LDFLAGS := $(shell pkg-config --libs libarchive)
CFLAGS := -g -Wall -Werror

all: $(out)/epub-archive $(out)/epub-archive3

$(out)/epub-archive3: LDFLAGS = $(shell pkg-config --libs libzip)

$(out)/%: %.c
	$(mkdir)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

%.valgrind: %
	-valgrind --log-file=$(out)/.vgdump --tool=memcheck --leak-check=yes --show-reachable=yes ./$* $(o)
	$(EDITOR) $(out)/.vgdump

mkdir = @mkdir -p $(dir $@)
