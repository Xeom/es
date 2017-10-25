LIBFILES=lib/utf8 lib/vec lib/text lib/com
BINFILES=display read input

LIBCFILES=$(addprefix src/, $(addsuffix .c, $(LIBFILES)))
LIBHFILES=$(addprefix inc/, $(addsuffix .h, $(LIBFILES)))
LIBOFILES=$(addprefix obj/, $(addsuffix .o, $(LIBFILES)))

PROGRAMS=$(addprefix bin/esee, $(BINFILES))

FLAGS=-Wall -Wno-unused-parameter -Wextra -Wformat \
      -Wfatal-errors -Wpedantic -Werror -Wconversion \
      -Wmissing-prototypes -Wdeclaration-after-statement \
      -Wno-switch -Wno-unused-label \
      -Iinc -Iinc/lib -Llib/

obj/%.o: src/%.c $(LIBHFILES)
	mkdir -p $(@D)
	$(CC) $(FLAGS) -c -g -fPIC --std=c99 $< -o $@

lib/libesee.so: $(LIBOFILES)
	mkdir -p $(@D)
	$(CC) $(FLAGS) -g -shared $^ -o $@

lib/libesee.a: $(LIBOFILES)
	mkdir -p $(@D)
	ar rcs $@ $^

bin/esee%: src/%.c lib/libesee.a
	mkdir -p $(@D)
	$(CC) $(FLAGS) -g -static --std=c99 $< -lesee -o $@

uninstall:
	rm -f $(addprefix /usr/, $(PROGRAMS))

install: uninstall all
	ln -s $(realpath $(PROGRAMS)) /usr/bin

all: $(PROGRAMS)

.PHONY=uninstall install all
