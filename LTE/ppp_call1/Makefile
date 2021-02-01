APP = ppp_call
NETWAY = netway ppp

INSTALL_DIR ?= ../winstall/

CROSS_COMPILE ?= /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
CFLAGS ?=  -march=armv7ve -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi

CC :=$(CROSS_COMPILE)gcc

all:$(APP)

$(APP): *.c
	$(CC) $(CFLAGS) $^ -o $@

install: $(APP)
	cp $^ $(INSTALL_DIR)/bin/
	cp $(NETWAY) $(INSTALL_DIR)/etc/ -a

clean:
	-rm -f *.o $(APP)
