
all: gpioset sc32_set_gain


CFLAGS += -I. -D_GNU_SOURCE

LDFLAGS += -L. -lgpiod


gpioset: gpioset.o tools-common.o

sc32_set_gain: sc32_set_gain.o

