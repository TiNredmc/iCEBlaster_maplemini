# iCEBlaster

iCEBlaster. STM32 based iCE40 SPI bitstream loader tool with USB MSC Drag and Drop feature.

# Description

iCEBlaster is a Drag and Drop firmware loader via SPI for iCE40LP1K and iCE40UL1K. The brain of this project is based on STM32F103CBT6 Arm Cortex M3 microcontroller sitting of the Maple Mini stm32 dev board. 

By using USB-FS library working as USB Mass storage device class (USB MSC) and with interface I/O code with internal Flash. Working together as USB Disk pre formatted with FAT12 file system with 512 Bytes per sector, 128 total sectors, 3 sectors reserved for master boot record (sector 0) and File allocation tables(sector 1-2). And using total of 64 internal Flash pages (1Kbytes per page, page 64 to 127, total of 64KBytes).

By copying the iCE40 FPGA Bitstream to the disk, 2 while loops help to locate the physical address of Bitstream on the internal Flash (for spi tx pointer) and to figure out the Bitstream size (for spi tx size). After that the Bitstream loading sequence begin. 

The Bitstream loading sequence is something like this 
```
// Reset and enter SPI mode sequence.
1. Pull SPI CE Pin to Low
2. Pull CRESET Pin to Low
3. Wait for 2ms.
4. Release CRESET (to HIGH). At this point iCE40 is now in Slave SPI mode.
5. wait for 200ms to make sure iCE40's Configuration RAM (CRAM) is cleared.

// Bitstream loading sequence.
6. Sending Bitstream data via SPI bus (MSB first for each byte).
7. Send 12 dummy bytes (0x00).
8. (optional) Send 7 dummy bytes (0x00) and the SPI pin can be later reconfigure to use as normal PIO

// End of loading.
9. Release SPI CE Pin (to HIGH). The iCE40 will automagically start.
```

When done, The mcu release CRESET to let FPGA run the firmware that we just flash. Then perform the internal flash reformat to have a new and ready to Drag and Drop USB Disk.

# Link(s)

[Hackaday.io](https://hackaday.io/project/185263-iceblaster)
