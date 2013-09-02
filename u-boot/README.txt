Build Instructions

From:
http://www.at91.com/linux4sam/bin/view/Linux4SAM/SAM9x5Page#Getting_AT91Bootstrap_sources


Configuring U-Boot and choosing where the U-Boot environment resides

Before building U-Boot, you have to configure it for 5series boards and to indicate where you want to store the environment. If you want to store it in NAND FLASH use:

make at91sam9x5ek_nandflash_config

And if you want to store it into the DataFlash use:

make at91sam9x5ek_dataflash_config

Compiling U-Boot

Now you have to launch the U-Boot cross-compilation:

make CROSS_COMPILE=<path_to_cross-compiler/cross-compiler-prefix->

