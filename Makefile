CC=gcc
CFLAGS=-O1 -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -fno-builtin -fPIC -mtune=amdfam10 -g3
LD=ld
LDLAGS=-nostdlib
AR=ar

KERN_SRCS:=$(wildcard sys/*.c)
LIBC_SRCS:=$(wildcard libc/*.c)
LD_SRCS:=$(wildcard ld/*.c)

$(USER).iso: kernel $(USER).img
	cp kernel bootcd/boot/kernel/kernel
	mkisofs -r -no-emul-boot -input-charset utf-8 -b boot/cdboot -o $@ bootcd/

$(USER).img: libc.a libc.so
	qemu-img create -f raw $@ 16M

kernel: $(KERN_SRCS:%.c=obj/%.o)
	${LD} ${LDLAGS} -o $@ -T linker.script $^

libc.a: $(LIBC_SRCS:%.c=obj/%.o)
	${AR} rcs $@ $^

libc.so: $(LIBC_SRCS:%.c=obj/%.o) ld.so
	${LD} ${LDLAGS} -shared -soname=$@ --dynamic-linker=/lib/ld.so --rpath-link=/lib -o $@ $^

ld.so: $(LD_SRCS:%.c=obj/%.o)
	${LD} ${LDLAGS} -shared -o $@ $^

obj/%.o: %.c $(wildcard include/*.h)
	@mkdir -p $(dir $@)
	${CC} -c ${CFLAGS} -o $@ $<

.PHONY: submit clean

SUBMITTO:=~mferdman/cse506-submit/

submit:
	tar -czf $(USER).tgz LICENSE Makefile sys include libc ld bootcd
	@gpg --quiet --import cse506-pubkey.txt
	gpg --yes --encrypt --recipient 'CSE506' $(USER).tgz
	rm -fv $(SUBMITTO)$(USER)=*.tgz.gpg
	cp -v $(USER).tgz.gpg $(SUBMITTO)$(USER)=`date +%F=%T`.tgz.gpg

clean:
	rm -rfv obj kernel libc.a libc.so ld.so
