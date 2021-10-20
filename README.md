# steganography
Simple C program which offers utilities for substitution steganography. It can hide a message within a .bmp file and discover messages it has hidden. 


## How to use this program
Simply compile and run the program. Follow the prompts to hide messages in bitmap ('.bmp') files, as well as recover hidden messages. The program also enables duplicating files.

## This program is not perfect - still a WIP
Still working out some unexpected behaviors. This program *DOES* work well for ".bmp" input files consisting of one color (very limiting, I know..). An working example can be found in "test_case_A/". The input file is "test_case_A/no_message.bmp".
An failing example can be found in "test_case_B/". The input file is "test_case_B/no_message.bmp". The message "test_case_B/secret_message.txt" was hidden in "no_message.bmp" to make "test_case_B/message_hidden.bmp".  

