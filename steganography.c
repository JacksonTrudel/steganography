// Jackson Trudel
// Basic steganography program (via substitution) which works with bitmap files (.bmp)
// Can also duplicate files
// Note: This does not work well for files/bitmaps containing FF (pure B/R/G or all) => (major issue)

#include <stdlib.h>
#include <stdio.h>

enum UserACtion
{
    HIDE_MESSAGE = 1,
    PRINT_MESSAGE,
    DUPLICATE_FILE,
    QUIT = 9
};

int printMenu();
void hideMessage();
void printMessage();
void duplicateFile();
void printPrettyFileSize(unsigned long long size);
void printCharBinary(char in);

int main()
{
    printf("This is a basic steganography program.\n");

    int userChoice = 0;

    while (userChoice != 9)
    {
        userChoice = printMenu();

        switch (userChoice)
        {
        case HIDE_MESSAGE:
            hideMessage();
            break;
        case PRINT_MESSAGE:
            printMessage();
            break;
        case DUPLICATE_FILE:
            duplicateFile();
            break;
        case QUIT:
            break;
        default:
            printf("Invalid value\n");
            break;
        }
    }

    printf("Please provide a file path:\n");
    return 0;
}

int printMenu()
{
    int userChoice;

    printf("Please make a selection from the following choices:\n");
    printf("\t(1) - Hide a message inside a file\n");
    printf("\t(2) - Print a message hidden inside a file\n");
    printf("\t(3) - Duplicate a file\n");
    printf("\t(9) - Escape\n");

    scanf("%d", &userChoice);
    return userChoice;
}

void hideMessage()
{
    char input_fpath[100];
    char output_fpath[100];
    FILE *ifp, *ofp;

    printf("Please provide a path for the input file (.bmp):\n");
    scanf("%s", input_fpath);
    ifp = fopen(input_fpath, "r");

    while (ifp == NULL)
    {
        printf("Could not open file. Please provide a path for the input file (.bmp):\n");
        scanf("%s", input_fpath);
        ifp = fopen(input_fpath, "r");
    }

    printf("Please provide a path to the output file (include .bmp):\n");
    scanf("%s", output_fpath);

    ofp = fopen(output_fpath, "w");

    while (ofp == NULL)
    {
        printf("Could not open or create the file. Please provide a path for the output file (include .bmp):\n");
        scanf("%s", output_fpath);
        ifp = fopen(output_fpath, "w");
    }

    // get size of input file
    fseek(ifp, 0L, SEEK_END);
    unsigned long long input_file_size = ftell(ifp);

    // bring ifp back to the beginning of the file
    rewind(ifp);

    //print the file size
    printf("Input file size: ");
    printPrettyFileSize(input_file_size);
    printf("\n");

    // Get terminating character(s)
    printf("What are the 4 terminating characters for your message? (These will not be included)\n");
    char terminatingChars[5];
    scanf("%s", terminatingChars);

    // Ask for message:
    fflush(stdin);
    printf("What message would you like to hide? You're limited to ");
    printPrettyFileSize(input_file_size / 8ull);
    printf(" (%llu characters)", input_file_size / 8ull);
    printf(":\n");

    unsigned long long messageAllocSize = 5000ull;
    unsigned long long messageSize = 0;
    char *message = malloc(messageAllocSize);
    char *inputBuffer;
    int keepReading = 1;
    unsigned long long charIdx;
    /*
    while (keepReading)
    {
        inputBuffer = calloc(5000, 1);
        scanf("%s", inputBuffer);
        charIdx = 0;

        while (inputBuffer[charIdx] != '\0')
        {
            // check whether this starts the terminating characters
            keepReading = 0;
            for (int i = 0; i < 4; i++)
            {
                if (inputBuffer[charIdx + i] != terminatingChars[i])
                {
                    keepReading = 1;
                    break;
                }
            }

            if (!keepReading)
            {
                break;
            }

            // make sure we have enough memory
            if (messageSize >= messageAllocSize / 2ull)
            {
                messageAllocSize *= 2ull;
                message = realloc(message, messageAllocSize);
            }

            // make sure the user didn't use too many characters
            if (messageSize >= input_file_size / 8)
            {
                printf("Your message has been truncated because it used too many characters");
                keepReading = 0;
                break;
            }

            // store message
            message[messageSize++] = inputBuffer[charIdx];
            charIdx++;
        }

        // add new line char
        if (keepReading)
        {
            message[messageSize++] = '\n';
        }

        free(inputBuffer);
    }
*/
    while (keepReading)
    {
        messageAllocSize = 5000ull;
        unsigned long long bufferSize = 100000ul;
        inputBuffer = malloc(bufferSize);
        charIdx = 0ull;

        for (unsigned long long idx = charIdx; idx < bufferSize; idx++)
        {
            inputBuffer[idx] = '\0';
        }

        scanf(" %[^\n]s", inputBuffer);

        while (inputBuffer[charIdx] != '\0')
        {
            // check whether this starts the terminating characters
            keepReading = 0;
            for (int i = 0; i < 4; i++)
            {
                if (inputBuffer[charIdx + i] != terminatingChars[i])
                {
                    keepReading = 1;
                    break;
                }
            }

            if (!keepReading)
            {
                break;
            }

            // make sure we have enough memory
            if (messageSize >= messageAllocSize / 2ull)
            {
                messageAllocSize *= 2ull;
            }

            // make sure the user didn't use too many characters
            if (messageSize >= input_file_size / 8)
            {
                printf("Your message has been truncated because it used too many characters");
                keepReading = 0;
                break;
            }

            // store message
            message[messageSize++] = inputBuffer[charIdx];
            charIdx++;
        }

        // add new line char
        if (keepReading)
        {
            message[messageSize++] = '\n';
        }

        free(inputBuffer);
    }

    printf("Message Size: ");
    printPrettyFileSize(messageSize);
    printf(" (%lu characters)\n", messageSize);

    unsigned long messageByteIdx = 0;
    int messageBitIdx = 0;
    int nextByteInput = fgetc(ifp);
    char altByte;
    char bitMask;

    int headerBytesPrinted = 0;
    int messageSizeBytesPrinted = 0, messageSizeBitsPrinted = 0;

    fseek(ifp, 1L, SEEK_SET);
    fseek(ofp, 0L, SEEK_SET);
    unsigned long long inputBytesRead = 0ull;
    while (inputBytesRead++ < input_file_size)
    {
        // copy header bytes
        if (headerBytesPrinted < 54)
        {
            headerBytesPrinted++;
            fputc(nextByteInput, ofp);
        }
        else if (messageSizeBytesPrinted < 8)
        {
            // print the size of the message (unsigned long)
            int bitPosition = (8 * messageSizeBytesPrinted + messageSizeBitsPrinted);
            altByte = (nextByteInput & ~1) | ((char)((messageSize & (0x0001 << (63 - bitPosition))) >> (63 - bitPosition)));

            /* Testing
            printf("BitPosition: %d, val (bit): %d, alt: %d, altBinary: ", bitPosition, ((char)((messageSize & (0x0001 << (63 - bitPosition))) >> (63 - bitPosition))) , altByte);
            printCharBinary(altByte);
            printf("\n");
            */

            fputc(altByte, ofp);

            // increment message indices
            messageSizeBitsPrinted++;
            if (messageSizeBitsPrinted >= 8)
            {
                messageSizeBitsPrinted = 0;
                messageSizeBytesPrinted++;
            }
        }
        else if (messageByteIdx >= messageSize)
        {
            fputc(nextByteInput, ofp);
        }
        else
        {
            altByte = (nextByteInput & ~1) | ((char)((message[messageByteIdx] & (0x0001 << (7 - messageBitIdx))) >> (7 - messageBitIdx)));
            fputc(altByte, ofp);

            // increment message indices
            messageBitIdx++;
            if (messageBitIdx >= 8)
            {
                messageBitIdx = 0;
                messageByteIdx++;
            }
        }

        nextByteInput = fgetc(ifp);
    }

    free(message);
    fclose(ifp);
    fclose(ofp);
    printf("\nFinished!\n\n");
    return;
}

void printMessage()
{

    char input_fpath[100];
    char output_fpath[100];
    FILE *ifp, *ofp;

    printf("Please provide a path for the input file (.bmp):\n");
    scanf("%s", input_fpath);
    ifp = fopen(input_fpath, "r");

    while (ifp == NULL)
    {
        printf("Could not open file. Please provide a path for the input file (.bmp):\n");
        scanf("%s", input_fpath);
        ifp = fopen(input_fpath, "r");
    }

    printf("Please provide a path to the output file of the message (include .txt):\n");
    scanf("%s", output_fpath);

    ofp = fopen(output_fpath, "w");

    while (ofp == NULL)
    {
        printf("Could not open or create the file. Please provide a path for the output file (include .bmp):\n");
        scanf("%s", output_fpath);
        ifp = fopen(output_fpath, "w");
    }

    // get size of input file
    fseek(ifp, 0L, SEEK_END);
    unsigned long input_file_size = ftell(ifp);

    // bring ifp back to the beginning of the file
    rewind(ifp);

    //print the file size
    printf("Input file size: ");
    printPrettyFileSize(input_file_size);
    printf("\n");

    // go to 53 bytes past the beginning of the file (skip the header)
    fseek(ifp, 54L, SEEK_SET);

    unsigned long messageSize = 0ul;

    int nextByteInput, bitVal;
    // read the message size
    for (int bitIdx = 0; bitIdx < 64; bitIdx++)
    {
        nextByteInput = fgetc(ifp);
        messageSize = messageSize | ((nextByteInput & 1) << (63 - bitIdx));
    }
    printf("Message size: ");
    printPrettyFileSize(messageSize);
    printf(" (%lu characters)\n", messageSize);

    // read message
    printf("\nHidden message: \n\n");
    for (unsigned long bytesRead = 0ul; bytesRead < messageSize; bytesRead++)
    {
        char charASCII = 0;
        for (int bitsRead = 0; bitsRead < 8; bitsRead++)
        {
            nextByteInput = fgetc(ifp);
            charASCII = charASCII | ((nextByteInput & 1) << (7 - bitsRead));
        }

        fputc(charASCII, ofp);
        printf("%c", charASCII);
    }

    fclose(ifp);
    fclose(ofp);
    return;
}

void duplicateFile()
{
    char input_fpath[100];
    char output_fpath[100];
    FILE *ifp, *ofp;

    printf("Please provide a path for the input file:\n");
    scanf("%s", input_fpath);
    ifp = fopen(input_fpath, "r");

    while (ifp == NULL)
    {
        printf("Could not open file. Please provide a path for the input file:\n");
        scanf("%s", input_fpath);
        ifp = fopen(input_fpath, "r");
    }

    printf("Please provide a path to the output file (with extension):\n");
    scanf("%s", output_fpath);

    ofp = fopen(output_fpath, "w");

    while (ofp == NULL)
    {
        printf("Could not open or create the file. Please provide a path for the output file (with extension):\n");
        scanf("%s", output_fpath);
        ifp = fopen(output_fpath, "w");
    }

    // get size of input file
    fseek(ifp, 0L, SEEK_END);
    unsigned long long input_file_size = ftell(ifp);

    // bring ifp back to the beginning of the file
    rewind(ifp);

    //print the file size
    printf("Input file size: ");
    printPrettyFileSize(input_file_size);
    printf("\n");

    // Copy contents to new file
    unsigned char nextChar = fgetc(ifp);
    while (!feof(ifp) && nextChar != EOF)
    {
        fputc(nextChar, ofp);
        nextChar = fgetc(ifp);
    }

    fclose(ifp);
    fclose(ofp);
    return;
}

void printPrettyFileSize(unsigned long long size)
{
    char pretty[10];

    // GB size
    if (size / 1000000000ull > 0)
    {
        double numGB = (double)(size / 10000000ull); // # bytes / 10,000,000 Bytes
        numGB /= 100.0;
        printf("%.2fGB", numGB);
    }
    else if (size / 1000000ull > 0)
    {
        double numMB = (double)(size / 10000ull); // # bytes / 10,000
        numMB /= 100.0;
        printf("%.2fMB", numMB);
    }
    else if (size / 1000ull > 0)
    {
        double numKB = (double)(size / 10000.0);
        printf("%.2fKB", numKB);
    }
    else
    {
        printf("%d bytes", (unsigned int)size);
    }
    return;
}

void printCharBinary(char in)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        printf("%d", !!((in << i) & 0x80));
    }
}