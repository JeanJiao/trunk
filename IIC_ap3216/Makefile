KERNELDIR := /home/jean/samba/alientek_code/alientek_kernel
CURRENT_PATH := $(shell pwd)
obj-m := drv_ap3216c.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean