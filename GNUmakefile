#!/usr/local/bin/gmake -f

TOOLS = eol cat2 echo2
REVN = 1.3
os:=$(shell uname -s)
hw:=$(shell uname -m)
ifeq "$(os)" "Darwin"
  ifeq "$(hw)" "i386"
    ARCH:=mach-i386
  else
    ARCH:=mach-ppc
  endif
else
ifeq "$(os)" "Linux"
    ARCH:=elf-i386
else
    ARCH:=elf-sparc
endif
endif

INSTALL_DIR=/eda/eklect/default

CC = gcc
CFLAGS = -Wall -pedantic

.PHONY: tools test all clean install good man arch

all: tools test arch man

tools: $(TOOLS)

%: %.c
	$(CC) $(CFLAGS) $? -o $@

arch: $(addsuffix -$(ARCH),$(TOOLS))

man: $(addsuffix .1,$(TOOLS))

test: test1.log test0.log
	@touch PASS

test1.log: ./eol GNUmakefile $(addsuffix .txt,mac dos unix eol) binary binary2
	@-rm -f PASS
	./eol *.txt bin* >test1.log
	@cmp test1.log test1.good || diff test1.log test1.good
	@echo "Test 1 passed!"

test0.log: ./eol GNUmakefile
	@-rm -f PASS
	./eol -0 binary2 >test0.log
	@cmp test0.log test0.good || diff test0.log test0.good
	@echo "Test 0 passed!"

install: PASS ./eol.1
	rsync -auv ./eol $(INSTALL_DIR)/bin
	rsync -auv ./eol.1 $(INSTALL_DIR)/man/man1/

good: test1.good test0.good

%-$(ARCH): ./%
	rsync -a $* $*-$(ARCH)

%.good: %.log
	rsync -a $*.log $*.good

%.1: %.c
	pod2man --section=1 --center='EDA script' --release=$(REVN) $*.c >$*.1

%.html: %.c GNUmakefile
	pod2html --title=$* --outfile=$*.html $*.c

%.txt: %.c GNUmakefile
	pod2text $*.c >$*.txt

clean:
	rm -fr $(TOOLS) $(foreach s,o 1,$(addsuffix .$s,$(TOOLS))) PASS test*.log pod2*
