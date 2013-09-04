Building Kernel

From:
http://www.at91.com/linux4sam/bin/view/Linux4SAM/SAM9x5Page#Getting_AT91Bootstrap_sources

Building the Linux kernel

Getting source code

The at91 5series patch is based on the 2.6.39 Linux kernel version, so the first step is to get it:

wget http://www.kernel.org/pub/linux/kernel/v2.6/linux-2.6.39.tar.bz2
tar xvjf linux-2.6.39.tar.bz2
cd linux-2.6.39

Now you have to apply the Atmel patch series:
Download experimental patch archive:

wget ftp://ftp.linux4sam.org/pub/linux/2.6.xx.at91/2.6.xx-at91-exp.y.tar.gz

Decompress archive with:

tar xvzf 2.6.xx-at91-exp.y.tar.gz

And then apply patchset one patch after the other in proper order:

for p in 2.6.xx-at91-exp.y/*; do patch -p1 < $p ; done

For this 2.6.39/5series example you will have to execute:

wget ftp://ftp.linux4sam.org/pub/linux/2.6.39-at91/2.6.39-at91-exp.tar.gz
tar xvzf 2.6.39-at91-exp.tar.gz
for p in 2.6.39-at91-exp/* ; do patch -p1 < $p ; done

Configuring and building the kernel

Firstly, use the 5series default kernel configuration:

!!! Swap out linux-2.6.39/arch/arm/mach-at91/board-sam9x5ek.c file before this step !!!

make ARCH=arm at91sam9x5ek_defconfig

Then you can customize the kernel configuration with:

make ARCH=arm menuconfig

Build the Linux kernel image:

make ARCH=arm  CROSS_COMPILE=<path_to_cross-compiler/cross-compiler-prefix->

To produce a kernel image suitable for U-Boot:
mkimage -A arm -O linux -C none -T kernel -a 20008000 -e 20008000 -n linux-2.6 -d arch/arm/boot/zImage uImage-2.6.39-at91sam9x5ek.bin


TO FLASH USING SAM-BA and SAM-ICE:

The procedure to flash the kernel image is the same as U-Boot, only two steps change:
1. Choose the NandFlash media tab in the SAM-BA GUI.
2. Initialize the media choosing the Enable NandFlash action in the Scripts rolling menu and press Execute.
3. Select Enable OS PMECC parameters in the Scripts rolling menu and press Execute. The default ECC configuration should be ok in the pop-up dialog box so just press the OK button.
4. To erase only the U-Boot part into the NAND FLASH, type this command after the SAM-BA prompt:
NANDFLASH::EraseBlocks 0x200000 0x7FFFFF
The SAM-BA EraseBlocks command takes two parameter: a start and an end address of FLASH that you want to erase.
5. Press Send File Name Browse button and choose your kernel image.
6. Enter the proper address on media in the Address text field. You can find the product map in GettingStarted_9x5. So in our case, the value for the kernel image is 0x200000.
7. Press Send File button.
8. Close SAM-BA, remove the USB cable.

