out := _out
LDFLAGS := $(shell pkg-config --libs libarchive)
CFLAGS := -g -Wall -Werror

all: $(out)/epub-archive

$(out)/%: %.c
	$(mkdir)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

%.valgrind: %
	-valgrind --log-file=$(out)/.vgdump --track-origins=yes --leak-check=full --show-leak-kinds=definite ./$* $(o)
	$(EDITOR) $(out)/.vgdump

mkdir = @mkdir -p $(dir $@)
