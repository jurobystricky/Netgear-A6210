ifeq ($(WIFI_MODE),)
RT28xx_MODE = STA
else
RT28xx_MODE = $(WIFI_MODE)
endif

TARGET = LINUX

# CHIPSET
# rt2860, rt2870, rt2880, rt2070, rt3070, rt3090, rt3572, rt3062, rt3562, rt3593, rt3573
# rt3562(for rt3592), rt3050, rt3350, rt3352, rt5350, rt5370, rt5390, rt5572, rt5592,
# rt8592(for rt85592),
# mt7601e, mt7601u,
# mt7620,
# mt7650e, mt7630e, mt7610e, mt7650u, mt7630u, mt7610u
# mt7662e, mt7632e, mt7612e, mt7662u, mt7632u, mt7612u

ifeq ($(CHIPSET),)
CHIPSET = mt7662u mt7632u mt7612u
endif

MODULE = $(word 1, $(CHIPSET))

#OS ABL - YES or NO
OSABL = NO

#RT28xx_DIR = home directory of RT28xx source code
RT28xx_DIR = $(shell pwd)

include $(RT28xx_DIR)/os/linux/config.mk

RTMP_SRC_DIR = $(RT28xx_DIR)/RT$(MODULE)

#PLATFORM: Target platform
PLATFORM = PC

MAKE = make

LINUX_SRC = /lib/modules/$(shell uname -r)/build
LINUX_SRC_MODULE = /lib/modules/$(shell uname -r)/kernel/drivers/net/wireless/
CROSS_COMPILE =

export OSABL RT28xx_DIR RT28xx_MODE LINUX_SRC CROSS_COMPILE CROSS_COMPILE_INCLUDE PLATFORM RELEASE CHIPSET MODULE RTMP_SRC_DIR LINUX_SRC_MODULE TARGET HAS_WOW_SUPPORT

# The targets that may be used.
PHONY += all clean uninstall install debug release

all: debug


release:
	@echo ""
	@echo "*** Building driver without debug messages ***"
	@echo ""
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules

debug:
	export DBGFLAGS
	@echo ""
	@echo "*** Building driver with debug messages ***"
	@echo ""
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) DBGFLAGS=-DDBG SUBDIRS=$(RT28xx_DIR)/os/linux modules

clean:
	cp -f os/linux/Makefile.clean os/linux/Makefile
	$(MAKE) -C os/linux clean
	rm -rf os/linux/Makefile

uninstall:
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.6 uninstall


install:
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.6 install

.PHONY: $(PHONY)




