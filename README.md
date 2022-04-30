# iCEBlaster

iCEBlaster. STM32 based iCE40 SPI bitstream programmer tool with USB MSC Drag and Drop feature.

# Note 1

This programmer doesn't flash the SPI NOR flash or NVCM of the iCE40 but sending the data directly to CRAM. After power cycle or reset the iCE40. Bitstream need to be reprogrammed. Useful for frequence code testing (without the need of external SPI NOR Flash).

# Note 2

After user drag bitstream to the disk. It will take a while to copy the bitstream and flash to the iCE40. After that, the USB disk need to be reformatted manually (If using Windows, It'll automatically select the FAT12 as file system, since There's only 128 sector and 512 byte each).

# Note 3

I'm currently trying to implement the DapLink style VFS. If you ever use the DapLink Drag-n-Drop programmer (ST-Link on Discovery board as a example). There'll be a USB disk that you can drag and drop the .bin or .hex binary file to program the MCU. I still have no idea how they do that plus their source code is really complex to understand.
