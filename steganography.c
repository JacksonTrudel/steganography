// Jackson Trudel
// Basic steganography program (via substitution) which works with bitmap files (.bmp)
// Can also duplicate files
// Note: This assumes the most simple (.bmp) file format, where the
//         color table is omitted.
//       This program may not work with all (.bmp) files

#include <stdlib.h>
#include <stdio.h>

enum UserACtion
{
    HIDE_MESSAGE = 1,
    PRINT_MESSAGE,
    DUPLICATE_FILE,
    QUIT = 9
};

enum PrintFormat
{
    BYTES,
    SUFFIX
};

int printMenu();
void hideMessage();
void printMessage();
void duplicateFile();
void copyBytes(FILE *ifp, FILE *ofp, unsigned long long numBytes);
void printPrettyNumber(unsigned long long size, int format);

// constants
#define STD_BUFFER_SIZE 1024
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define MESSAGE_SIZE_SECTION_SIZE 64
#define MAX_INPUT_LINE_LENGTH 10000
#define MIN_CHAR_LEFT_WITHOUT_NOTIFYING 20000

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
    printf("\n\n");
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
    char fpath[100];
    FILE *ifp, *ofp;

    printf("Please provide a path for the input file (.bmp):\n");
    scanf("%s", fpath);
    ifp = fopen(fpath, "rb");

    while (ifp == NULL)
    {
        printf("Could not open file. Please provide a path for the input file (.bmp):\n");
        scanf("%s", fpath);
        ifp = fopen(fpath, "rb");
    }

    printf("Please provide a path to the output file (include .bmp):\n");
    scanf("%s", fpath);

    ofp = fopen(fpath, "wb");

    while (ofp == NULL)
    {
        printf("Could not open or create the file. Please provide a path for the output file (include .bmp):\n");
        scanf("%s", fpath);
        ifp = fopen(fpath, "wb");
    }

    // get size of input file
    fseek(ifp, 0L, SEEK_END);
    unsigned long long inputFileSize = ftell(ifp);

    // bring ifp back to the beginning of the file
    rewind(ifp);

    //print the file size
    printf("Input file size: ");
    printPrettyNumber(inputFileSize, BYTES);
    printf("\n");

    // Get terminating character(s)
    printf("What are the 4 terminating characters for your message (ex. XXXX)? (These will not be included)\n");
    char terminatingChars[5];
    scanf("%s", terminatingChars);

    // Ask for message:
    fflush(stdin);
    unsigned long long maxMessageSize = (inputFileSize - (HEADER_SIZE + INFO_HEADER_SIZE + MESSAGE_SIZE_SECTION_SIZE)) / 8ull;
    printf("What message would you like to hide? You're limited to ");
    printPrettyNumber(maxMessageSize, BYTES);
    printf(" (%llu characters)", maxMessageSize);
    printf(":\n");

    // Read secret message
    unsigned long long messageAllocSize = (maxMessageSize < 5000ull) ? (maxMessageSize + 1) : 5000ull;
    char *message = malloc(messageAllocSize);
    unsigned long long messageSize = 0ull;

    // message input buffer variables
    char *inputBuffer = malloc(MAX_INPUT_LINE_LENGTH);
    int inputIdx;

    int keepReading = 1;
    while (keepReading)
    {
        // print number of chars remaining
        if (maxMessageSize - messageSize < MIN_CHAR_LEFT_WITHOUT_NOTIFYING)
        {
            printf("\n(chars left: ");
            printPrettyNumber(maxMessageSize - messageSize, SUFFIX);
            printf(")\n");
        }

        inputIdx = 0;

        // initialize input buffer with terminating characters
        for (unsigned long long idx = inputIdx; idx < MAX_INPUT_LINE_LENGTH; idx++)
        {
            inputBuffer[idx] = '\0';
        }

        // read in characters until we hit a new line character
        scanf(" %[^\n]s", inputBuffer);

        while (inputBuffer[inputIdx] != '\0')
        {
            // check for the terminating characters
            keepReading = 0;
            for (int i = 0; i < 4; i++)
            {
                if (inputBuffer[inputIdx + i] != terminatingChars[i])
                {
                    keepReading = 1;
                    break;
                }
            }
            if (!keepReading)
                break;

            // make sure we have enough memory
            if (inputIdx >= messageAllocSize / 2ull)
            {
                messageAllocSize *= 2ull;
            }

            // make sure the user didn't use too many characters
            if (messageSize >= maxMessageSize)
            {
                printf("Your message has been truncated because it used too many characters.");
                keepReading = 0;
                break;
            }

            // store message
            message[messageSize++] = inputBuffer[inputIdx++];
        }

        // add new line char
        if (keepReading)
        {
            message[messageSize++] = '\n';
        }
    }

    free(inputBuffer);

    // print message size
    printf("Message Size: ");
    printPrettyNumber(messageSize, BYTES);
    printf(" (");
    printPrettyNumber(messageSize, SUFFIX);
    printf(" characters)\n");

    // set pointers to beginning of each file
    fseek(ifp, 0L, SEEK_SET);
    fseek(ofp, 0L, SEEK_SET);

    unsigned char buffer[STD_BUFFER_SIZE];

    // Copy the header of the file
    fread(buffer, HEADER_SIZE + INFO_HEADER_SIZE, 1, ifp);
    fwrite(buffer, HEADER_SIZE + INFO_HEADER_SIZE, 1, ofp);

    // write the message length
    fread(buffer, MESSAGE_SIZE_SECTION_SIZE, 1, ifp);
    for (int byteIdx = 0; byteIdx < 8; byteIdx++)
    {
        for (int bitIdx = 0; bitIdx < 8; bitIdx++)
        {
            int bitPosition = (8 * byteIdx + bitIdx);
            buffer[bitPosition] = (buffer[bitPosition] & ~1) | ((char)((messageSize & (0x0001 << (63 - bitPosition))) >> (63 - bitPosition)));
        }
    }
    fwrite(buffer, MESSAGE_SIZE_SECTION_SIZE, 1, ofp);

    // Hide message
    unsigned long long messageBytesWritten = 0ull;
    int MESSAGE_BYTES_WRITABLE_PER_ITER = (sizeof(buffer) / 8);
    int messageBytesToWrite, inputBytesToWrite;
    while (messageBytesWritten < messageSize)
    {
        // calculate the number of *message* bytes we want to write
        // NOTE: One message byte is written across the LSBs of *8 input bytes*
        messageBytesToWrite = ((messageSize - messageBytesWritten) < MESSAGE_BYTES_WRITABLE_PER_ITER) ? (messageSize - messageBytesWritten) : MESSAGE_BYTES_WRITABLE_PER_ITER;

        // read input file bytes - 8 for each message byte
        inputBytesToWrite = messageBytesToWrite * 8;
        fread(buffer, inputBytesToWrite, 1, ifp);

        // for each byte of the input, hide a bit of the message in the LSB
        for (int byteIdx = 0; byteIdx < messageBytesToWrite; byteIdx++)
        {
            for (int bitIdx = 0; bitIdx < 8; bitIdx++)
            {
                int inputByteNumber = (8 * byteIdx + bitIdx);
                buffer[inputByteNumber] = (buffer[inputByteNumber] & ~1) | ((char)((message[messageBytesWritten + byteIdx] & (0x0001 << (7 - bitIdx))) >> (7 - bitIdx)));
            }
        }
        // write output file bytes -> same # as input bytes
        fwrite(buffer, inputBytesToWrite, 1, ofp);

        messageBytesWritten += messageBytesToWrite;
    }
    free(message);

    // write the remainder of the file
    unsigned long long totalBytesWritten = ftell(ifp);

    if (inputFileSize > totalBytesWritten)
        copyBytes(ifp, ofp, inputFileSize - totalBytesWritten);

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
    unsigned long inputFileSize = ftell(ifp);

    // bring ifp back to the beginning of the file
    rewind(ifp);

    //print the file size
    printf("Input file size: ");
    printPrettyNumber(inputFileSize, BYTES);
    printf("\n");

    // go to 53 bytes past the beginning of the file (skip the header)
    fseek(ifp, 54L, SEEK_SET);

    // read the message size
    unsigned long long messageSize = 0ull;
    unsigned char buffer[STD_BUFFER_SIZE];
    fread(buffer, MESSAGE_SIZE_SECTION_SIZE, 1, ifp);

    int nextByteInput, bitVal;
    // read the message size
    for (int bitIdx = 0; bitIdx < MESSAGE_SIZE_SECTION_SIZE; bitIdx++)
    {
        nextByteInput = buffer[bitIdx];
        messageSize = messageSize | ((nextByteInput & 1) << (63 - bitIdx));
    }
    printf("Message size: ");
    printPrettyNumber(messageSize, BYTES);
    printf(" (%lu characters)\n", messageSize);

    // read message
    unsigned long long messageBytesRead = 0ull;
    int MESSAGE_BYTES_READABLE_PER_ITER = sizeof(buffer) / 8;
    int messageBytesToRead, inputBytesToRead, bufferIdx;
    while (messageBytesRead < messageSize)
    {
        // calculate the number of *message* bytes we want to read
        // NOTE: One message byte is written across the LSBs of *8 input bytes*
        messageBytesToRead = ((messageSize - messageBytesRead) < MESSAGE_BYTES_READABLE_PER_ITER) ? (messageSize - messageBytesRead) : MESSAGE_BYTES_READABLE_PER_ITER;

        // read input file bytes - 8 for each message byte
        inputBytesToRead = messageBytesToRead * 8;
        fread(buffer, inputBytesToRead, 1, ifp);

        // for each byte of the input, read a bit of the message in the LSB
        for (int byteIdx = 0; byteIdx < messageBytesToRead; byteIdx++)
        {
            char messageByte = 0x00;
            for (int bitIdx = 0; bitIdx < 8; bitIdx++)
            {
                bufferIdx = 8 * byteIdx + bitIdx;
                messageByte = messageByte | ((buffer[bufferIdx] & 1) << (7 - bitIdx));
            }
            // overwrite beginning of buffer as we "decode" the message
            buffer[byteIdx] = messageByte;

            // print decoded char to screen
            printf("%c", messageByte);
        }
        // write output file bytes -> same # as *message* bytes
        fwrite(buffer, messageBytesToRead, 1, ofp);

        messageBytesRead += messageBytesToRead;
    }
    printf("\n\n");

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
    ifp = fopen(input_fpath, "rb");

    while (ifp == NULL)
    {
        printf("Could not open file. Please provide a path for the input file:\n");
        scanf("%s", input_fpath);
        ifp = fopen(input_fpath, "rb");
    }

    printf("Please provide a path to the output file (with extension):\n");
    scanf("%s", output_fpath);

    ofp = fopen(output_fpath, "wb");

    while (ofp == NULL)
    {
        printf("Could not open or create the file. Please provide a path for the output file (with extension):\n");
        scanf("%s", output_fpath);
        ifp = fopen(output_fpath, "wb");
    }

    // get size of input file
    fseek(ifp, 0L, SEEK_END);
    unsigned long long inputFileSize = ftell(ifp);

    // bring ifp back to the beginning of the file
    rewind(ifp);

    //print the file size
    printf("Input file size: ");
    printPrettyNumber(inputFileSize, BYTES);
    printf("\n\n");

    copyBytes(ifp, ofp, inputFileSize);

    fclose(ifp);
    fclose(ofp);
    return;
}

void copyBytes(FILE *ifp, FILE *ofp, unsigned long long numBytes)
{
    // Copy contents to new file
    unsigned long long bytesCopied = 0ull;
    unsigned char byteBuffer[STD_BUFFER_SIZE];

    while (bytesCopied < numBytes)
    {
        int bytesToCopy = (numBytes - bytesCopied <= sizeof(byteBuffer)) ? (numBytes - bytesCopied) : sizeof(byteBuffer);
        fread(byteBuffer, bytesToCopy, 1, ifp);
        fwrite(byteBuffer, bytesToCopy, 1, ofp);
        bytesCopied += bytesToCopy;
    }
}

void printPrettyNumber(unsigned long long size, int format)
{
    char pretty[10];

    // GB size
    if (size / 1000000000ull > 0)
    {
        double numGB = (double)(size / 10000000ull); // # bytes / 10,000,000 Bytes
        numGB /= 100.0;
        printf("%.2f", numGB);

        switch (format)
        {
        case BYTES:
            printf("GB");
            break;
        case SUFFIX:
            printf("B");
            break;
        default:
            break;
        }
    }
    else if (size / 1000000ull > 0)
    {
        double numMB = (double)(size / 10000ull); // # bytes / 10,000
        numMB /= 100.0;
        printf("%.2f", numMB);

        switch (format)
        {
        case BYTES:
            printf("MB");
            break;
        case SUFFIX:
            printf("M");
            break;
        default:
            break;
        }
    }
    else if (size / 1000ull > 0)
    {
        double numKB = (double)(size / 10000.0);
        printf("%.2f", numKB);

        switch (format)
        {
        case BYTES:
            printf("KB");
            break;
        case SUFFIX:
            printf("K");
            break;
        default:
            break;
        }
    }
    else
    {
        printf("%d", (unsigned int)size);

        switch (format)
        {
        case BYTES:
            printf(" bytes");
            break;
        case SUFFIX:
            break;
        default:
            break;
        }
    }
    return;
}