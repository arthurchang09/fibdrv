CONFIG_MODULE_SIG = n
TARGET_MODULE := fibdrv_bn

obj-m := $(TARGET_MODULE).o
fibdrv_bn-objs := \
	fibdrv.o \
	bn.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) client
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) client out
load:
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null

client: client.c
	$(CC) -o $@ $^

PRINTF = env printf
PASS_COLOR = \e[32;01m
NO_COLOR = \e[0m
pass = $(PRINTF) "$(PASS_COLOR)$1 Passed [-]$(NO_COLOR)\n"

check: all
	$(MAKE) unload
	$(MAKE) load
	sudo ./client > out
	$(MAKE) unload
	$(call pass)
	@scripts/verify.py
PREFIX = data
POSTFIX = .txt
plot: all
	$(MAKE) unload
	$(MAKE) load
	for i in 1 2 3 4 5 6 7 8 9 10; do \
		sudo taskset 0x01 ./client > data/time/${PREFIX}$$i${POSTFIX}; \
	done
	$(MAKE) unload
	@scripts/time.py
	gnuplot scripts/time.gp
	eog time.png