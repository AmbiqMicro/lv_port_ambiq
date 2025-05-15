# Smartwatch Dynamic Wallpaper Resource Loading Guide

This guide explains how to load external dynamic wallpaper resources for the LVGL Smartwatch project using the Apollo510 EVB.


## Resource Location

Wallpaper resources are placed in lvgl_smartwatch\src\gui\img_wallpaper

The resources are provided in a compressed file: wallpaper.zip


## Hardware

Apollo510 EVB


## Step-by-Step Instructions

1. **Flash the Resource Loader Program**

   Use J-Link to flash the provided binary to the Apollo510:

   - **Binary File**: tinyusb_cdc_msc_emmc.bin, placed in lvgl_smartwatch\src\gui\img_wallpaper
   - **Flash Address**: `0x410000`

2. **Connect Apollo510 to PC via USB**

   After flashing is complete, **connect the Apollo510's USB interface (not J-Link)** to your PC.  
   Your computer will recognize the board as a USB drive.

3. **Prepare the Resource Files**

   - Unzip the `wallpaper.zip` file.
   - Copy **all extracted files** to the root directory of the newly mounted USB drive.

4. **Finalize**

   The Apollo510 will now be able to access the wallpaper resources from the external storage.


## Notes

- On **first use**, the mounted USB storage may require formatting. 