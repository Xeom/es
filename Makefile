LIBFILES=lib/utf8 lib/vec lib/text lib/com
BINFILES=display read input

LIBCFILES=$(addprefix src/, $(addsuffix .c, $(LIBFILES)))
LIBHFILES=$(addprefix inc/, $(addsuffix .h, $(LIBFILES)))
LIBOFILES=$(addprefix obj/, $(addsuffix .o, $(LIBFILES)))

PROGRAMS=$(addprefix bin/esee, $(BINFILES))

RESET="\e[0m"
DGRN="\e[38;2;90;220;20m"
LGRN="\e[38;2;160;240;60m"
BGRN="\e[38;2;180;255;120m"
DYEL="\e[38;2;200;180;20m"
LYEL="\e[38;2;240;200;60m"
BYEL="\e[38;2;240;230;110m"
DORN="\e[38;2;220;120;20m"
LORN="\e[38;2;240;150;60m"
BORN="\e[38;2;255;200;120m"
DRED="\e[38;2;240;80;20m"
LRED="\e[38;2;255;100;60m"
BRED="\e[38;2;255;170;150m"

FLAGS=-Wall -Wno-unused-parameter -Wextra -Wformat \
      -Wfatal-errors -Wpedantic -Werror -Wconversion \
      -Wmissing-prototypes -Wdeclaration-after-statement \
      -Wno-switch -Wno-unused-label \
      -Iinc -Iinc/lib -Llib/

obj/%.o: src/%.c $(LIBHFILES)
	@echo $(DGRN)[$(LGRN)* *$(DGRN)]$(BGRN) Compiling $@ ...$(RESET)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c -g -fPIC --std=c99 $< -o $@

lib/libesee.so: $(LIBOFILES)
	@echo $(DYEL)[$(LYEL)\> \<$(DYEL)]$(BYEL) Assembling $@ ...$(RESET)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -g -shared $^ -o $@

lib/libesee.a: $(LIBOFILES)
	@echo $(DYEL)[$(LYEL)\> \<$(DYEL)]$(BYEL) Assembling $@ ...$(RESET)
	@mkdir -p $(@D)
	@ar rcs $@ $^

bin/esee%: src/%.c lib/libesee.a
	@echo $(DORN)[$(LORN)\>*\<$(DORN)]$(BORN) Compiling $@ ...$(RESET)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -g -static --std=c99 $< -lesee -o $@

uninstall:
	@echo $(DRED)[$(LRED) x $(DRED)]$(BRED) Removing symlinks ...$(RESET)
	@rm -f $(addprefix /usr/, $(PROGRAMS))

install: all uninstall
	@echo $(DRED)[$(LRED)\<-\>$(DRED)]$(BRED) Creating symlinks ...$(RESET)
	@ln -s $(realpath $(PROGRAMS)) /usr/bin

all: lib/libesee.a lib/libesee.so $(PROGRAMS)

.DEFAULT_GOAL=all
.PHONY=uninstall install all
