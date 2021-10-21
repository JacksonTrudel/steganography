# steganography

Simple C program which offers utilities for substitution steganography. It can hide a message within a .bmp file and discover messages it has hidden. This is made for educational purposes, and shall not be used for criminal purposes.

## How to use this program

Simply compile and run the program. Follow the prompts to hide messages in bitmap ('.bmp') files, as well as recover hidden messages. The program also enables duplicating files.

## Making (.bmp) image files:

I created these test cases using Paint 3D (exported in .bmp format). This will store the (.bmp) file in a way (without compression or a color table) which is compatible with this program. See note in next section about compatibility issues.

## If something fails:

Make sure you specified the correct file extensions for both input and output files.

This program is only designed to work with (.bmp) files, but with minor changes it could work with any image file which stores the uncompressed value of each pixel.

If it is not working correctly, this could be due to the type of (.bmp) file passed as the original input:

- The (.bmp) must not be compressed
- The (.bmp) must not store a color table, just the pixel data. This program has not been tested with (.bmp) files which store a color table as metadata.
  The test cases were created using Paint 3D, which will export in a format compatible with this program.

## Todo:

- Detect if the (.bmp) file is compressed, and halt the operation
- Encrypting message using a password
