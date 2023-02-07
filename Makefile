out := _out
LDFLAGS := $(shell pkg-config --libs libarchive)
CFLAGS := -g -Wall -Werror

all: $(out)/epub-archive

$(out)/%: %.c
	$(mkdir)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

%.valgrind: %
	-valgrind --log-file=$(out)/.vgdump --tool=memcheck --leak-check=yes --show-reachable=yes ./$* $(o)
	$(EDITOR) $(out)/.vgdump

mkdir = @mkdir -p $(dir $@)
