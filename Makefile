#CROSS_COMPILE=../../beaglebone/compiler/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
#CROSS_COMPILE=../../raspberryPi/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-

CC=$(CROSS_COMPILE)gcc

INCLUDES = -I./examples/common \
	-I./simplelink/include \
	-I./simplelink/source \
	-I./platform/linux \
	-I./cc3100-spi-kernelmodul \


OPTIMIZE       = -Os

# _GNU_SOURCE defined, to use F_SETSIG in fctnl (user_functions.c)
CFLAGS	=  $(INCLUDES)  -w -D _GNU_SOURCE -pthread

DEBUGSPI = ./platform/linux/user_functions.c \
	   ./examples/spi_debug_tool/main.c

SRCSL =	./simplelink/source/device.c \
	./simplelink/source/driver.c \
	./simplelink/source/flowcont.c \
	./simplelink/source/fs.c \
	./simplelink/source/netapp.c \
	./simplelink/source/netcfg.c \
	./simplelink/source/nonos.c \
	./simplelink/source/socket.c \
	./simplelink/source/spawn.c \
	./simplelink/source/wlan.c \
	./platform/linux/user_functions.c \

TCP =	./examples/tcp_socket/main.c

WLANAP = ./examples/getting_started_with_wlan_ap/main.c

OBJDIR = .
OBJ = $(SRC:%.c=$(OBJDIR)/%.o)
EXECUTE = $(CC) $(CFLAGS) $^ -o $@

all: wlanap

wlanap: $(SRCSL) $(WLANAP)
	$(EXECUTE)

debug: $(DEBUGSPI)
	$(EXECUTE)

tcp: $(SRCSL) $(TCP)
	$(EXECUTE)

.SUFFIXES: .o .c .s

%.o: %.c
	$(CC) $(CFLAGS) -c  $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f wlanap debug tcp $(OBJ) \
