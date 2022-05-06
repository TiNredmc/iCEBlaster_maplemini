﻿# iCEBlaster

iCEBlaster. STM32 based iCE40 SPI bitstream loader tool with USB MSC Drag and Drop feature.

# Note 1

This programmer doesn't flash the SPI NOR flash or NVCM of the iCE40 but sending the data directly to CRAM. After power cycle or reset the iCE40. Bitstream need to be reprogrammed. Useful for frequence code testing (without the need of external SPI NOR Flash).

# Note 2

I already implemented the self-formatting that will reset the drive to their default state and ready to Drag N Drop right away.
