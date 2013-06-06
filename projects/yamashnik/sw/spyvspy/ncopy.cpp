#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <getopt.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "sendpacket.h"
#include "commands.h"
#include "serial.h"
#include "diags.h"

#define IOPORT "/dev/ttyS0"
#define BAUDRATE B38400

#define SRC 4
#define DST 2
#define CMND 6

#define SECTORSIZE  128

unsigned short int studentNo = 127;

void pingPong(PacketSender& ps, int adr) 
{
    PingPacket ping(0, adr);
    // Send ping
    info("\nSending PING.");
    do {
        ps.SendPacket(&ping);
    } while (!ps.ReceivePacket());
    info("\nGot PONG.\n");
}

void sendFile(const char* fileName, PacketSender& packetSender, int studentNo)
{
    uint8_t Sector[SECTORSIZE];
    FILE* infile;
    struct stat stat_p;
    int sectNo = 0;


    // check the size of file
    stat(fileName, &stat_p);

    infile = fopen(fileName, "rb");
    if (infile == 0) {
        eggog("Error: cannot open %s\n", fileName);
    }

    verbose("ncopy:sendFile: sending file %s to workstation %d\n", fileName, studentNo);

    // Send ping
    pingPong(packetSender, studentNo);

    // Create file on the net disk
    {
        NetCreateFilePacket createFile(0, studentNo, fileName);
        do {
           packetSender.SendPacket(&createFile);
        } while (!packetSender.ReceivePacket());
    }
    verbose("\nGot re: NET_CREATE_FILE.\n");

    // Reading the file sector-by-sector and writing them onto the net disk

    info("\nNumber of sectors to send: %zd\n", (size_t) ((stat_p.st_size / sizeof (Sector)) + (stat_p.st_size % sizeof (Sector))));

    while (!feof(infile))
    {
        if (fread(Sector, sizeof(Sector), 1, infile) == 0) break;

        {
            NetMasterDataPacket masterData(0, studentNo, Sector, SECTORSIZE);
            packetSender.SendPacket(&masterData);

            NetWriteFilePacket writeFile(0, studentNo, packetSender.GetNetFCB());
            do {
                packetSender.SendPacket(&writeFile);
            } while (!packetSender.ReceivePacket());
        }
        verbose("\nSent sector No.%d", sectNo);

        if ((sectNo+1) % 10 == 0) {
            info(". %d ", sectNo+1);
        }
        else {
            info(".");
        }

        sectNo++;
    }

    // close the file on the net disk
    {
        NetCloseFilePacket closeFile(0, studentNo, packetSender.GetNetFCB());
        do {
            packetSender.SendPacket(&closeFile);
        } while (!packetSender.ReceivePacket());
    }
    info("\nSendFile Done.\n");

}

void ncopy(const char* port, int studentNo, int nfiles, char* file[]) 
{
    SerialPort serialPort(port);
    PacketSender packetSender(&serialPort);

    if (serialPort.Setup() != ERR_NONE) {
       eggog("Error: cannot open %s\n", port);
    }

    for (int i = 0; i < nfiles; i++) {
        sendFile(file[i], packetSender, studentNo);
    }

    info("ncopy done\n");
}

void sendBASIC(PacketSender& packetSender, int studentNo, FILE* file)
{

}

/************************************************************************
 * ROM images
 ************************************************************************/
const int SIZE32K = 32*1024;
const int SIZE16K = 16*1024;
const int MAXBLKSIZE = 56;

uint8_t ROM2BIN_startCode[] =
{
/*
    commented because this code section is needed only for
    disk drive switching off, but we run on a diskless MSX

    0xFB,               // ei
    0x76,               // halt
    0x10,0xFD,          // djnz [go to halt]
    0x3E,0xC9,          // ld a,C9
    0x32,0x9F,0xFD,     // ld (FD9F),a
*/
    // this code switches RAM page on address 0x4000,
    // copies 16K from 0x9000 to 0x4000
    // (or to 0x8000 when "ld de,4000" patched to be "ld de,8000")
    // and then execues ROM code
    // (or returns to Basic if "jp hl" is patched to be "nop")

    0xCD,0x38,0x01,     // call 0138
    0xE6,0x30,          // and 30
    0x0F,               // rrca
    0x0F,               // rrca
    0x0F,               // rrca
    0x0F,               // rrca
    0x4F,               // ld c,a
    0x06,0x00,          // ld b,00
    0x21,0xC5,0xFC,     // ld hl,FCC5
    0x09,               // add hl,bc
    0x7E,               // ld a,(hl)
    0xE6,0x30,          // and 30
    0x0F,               // rrca
    0x0F,               // rrca
    0xB1,               // or c
    0xF6,0x80,          // or 80
    0x26,0x40,          // ld h,40
    0xCD,0x24,0x00,     // call 0024
    0xF3,               // di
    0x11,0x00,0x40,     // ld de,4000 (0x40 will be patched to 0x80)
    0x21,0x00,0x90,     // ld hl,9000
    0x01,0x00,0x40,     // ld bc,4000
    0xED,0xB0,          // ldir
    0x2A,0x02,0x40,     // ld hl,(4002)
    0xE9,               // jp hl (can be patched to reach next command)

    0x3E, 0x80,         // ld a,80
    0x26, 0x40,         // ld h,40
    0xCD,0x24,0x00,     // call 0024
    0xC9                // ret
};

unsigned char binBuf[SIZE32K + sizeof(ROM2BIN_startCode)];  // enough for any bloadable binary

void sendBlocks(PacketSender& packetSender, int studentNo, int start, int end)
{
    int lastblock = (end - start) / MAXBLKSIZE;

    info("sendROM: %d blocks to send\n", lastblock);

    int current = start;
    int binBufOffset = 0;

    for (int i = 0; i < lastblock; i++) {
        verbose("\nSending block %d\n", i + 1);
        if ((i+1) % 10 == 0) {
            info(". %d ", i+1);
        } 
        else {
            info(".");
        }

        {
            SHEXDataPacket data(0, studentNo, &binBuf[binBufOffset], MAXBLKSIZE, 0);
            packetSender.SendPacket(&data);
            packetSender.ReceivePacket();
        }
        current += MAXBLKSIZE;
        binBufOffset += MAXBLKSIZE;
    }

    // Calculate the rest
    verbose("\nLast block: %d bytes\n", end - current + 1);
    {
        SHEXDataPacket data(0, studentNo, &binBuf[binBufOffset], end - current + 1, 1);
        packetSender.SendPacket(&data);
        packetSender.ReceivePacket();
    }
}

void runROM(PacketSender& packetSender, int studentNo, uint16_t defusr)
{
    char command[40];
    sprintf(command, " _nete:DefUsr=&H%.4x:?Usr(0):_neti", defusr);
    info("\nRun command: '%s'\n", command);

    pingPong(packetSender, studentNo);

    packetSender.SendPacket(&SNDCMDPacket(0, studentNo, command));
    packetSender.ReceivePacket();
}

void sendROM(PacketSender& packetSender, int studentNo, FILE* file)
{
    //int numblocks, i;
    long romSize;

    if (fseek(file, 0, SEEK_END) == 0) {
        romSize = ftell(file);
    } 
    else {
        eggog("Error: sendROM: cannot get file size\n");
    }

    info("\nROM file, %ld bytes\n", romSize);

    if (romSize > SIZE32K) {
        eggog("\nSending ROMs bigger than 32K is not supported\n");
    }
    if (romSize != 8*1024 && romSize != SIZE16K && romSize != SIZE32K) {
        eggog("\nSending ROMs with non-standard size (not 8, 16 or 32K) is not supported\n");
    }

    if (romSize < SIZE32K) {
        uint16_t start = 0x9000;
        uint16_t end = start + romSize + sizeof(ROM2BIN_startCode) - 1;
        uint16_t run = start + romSize;

        rewind(file);
        fread(&binBuf, romSize, 1, file);
        memcpy(&binBuf[romSize], ROM2BIN_startCode, sizeof(ROM2BIN_startCode));

        binBuf[0] = 0; // destroy "AB" signature so it won't reboot

        printf ("Start: %x, End: %x, Run: %x\n", start, end, run);

        pingPong(packetSender, studentNo);

        morbose("Sending SHEX header packet\n");

        packetSender.SendPacket(&SHEXHeaderPacket(0, studentNo, start, end));
        packetSender.ReceivePacket();

        morbose("SHEX header acknowledged\n");

        sendBlocks(packetSender, studentNo, start, end);
        usleep(10000);
        runROM(packetSender, studentNo, run);
        usleep(500000);
    }
    else {
        eggog("2-part loading not yet implemented\n");
#if 0
        // send 1st 16K part

        start = 0x9000;
        end = start + SIZE16K + sizeof(ROM2BIN_startCode) - 1;
        run = start + SIZE16K;

        rewind (infile);
        fread (&binBuf, SIZE16K, 1, infile);
        memcpy(&binBuf[SIZE16K], ROM2BIN_startCode, sizeof (ROM2BIN_startCode));
        // replace last Z80 command "jp hl" with "nop"
        binBuf[SIZE16K + sizeof(ROM2BIN_startCode) - 9] = 0x00;
        // destroy "AB" signature so it won't restart ROM on reboot
        binBuf[0] = 0;

        printf ("\nSending 1st 16K ROM part: Start: %x, End: %x, Run: %x\n", start, end, run);

        Send ();
        Run();

        // send 2nd 16K part

        fread (&binBuf, SIZE16K, 1, infile);
        memcpy(&binBuf[SIZE16K], ROM2BIN_startCode, sizeof (ROM2BIN_startCode));
        // replace Z80 command "ld de,4000" with "ld de,8000"
        binBuf[SIZE16K + sizeof(ROM2BIN_startCode) - 21] = 0x80;

        printf ("\n\nSending 2nd 16K ROM part: Start: %x, End: %x, Run: %x\n", start, end, run);

        usleep (500000); // pause after previous 16K part
        Send ();
        Run();
#endif
    }

    printf ("\nSending ROM done.\n");
}

void sendBIN(PacketSender& packetSender, int studentNo, FILE* file)
{

}


void netSend(const char* port, int studentNo, int nfiles, char* file[])
{
    SerialPort serialPort(port);
    PacketSender packetSender(&serialPort);

    if (serialPort.Setup() != ERR_NONE) {
       eggog("Error: cannot open %s\n", port);
    }

    for (int fileIdx = 0; fileIdx < nfiles; fileIdx++) {
        uint8_t magic;

        FILE* infile = fopen(file[fileIdx], "rb");
        if (infile == 0) {
            eggog("Error: sendROM: cannot open file %s\n", file[fileIdx]);
        }

        if (fread(&magic, 1, 1, infile) != 1) {
            eggog("Error: file %s is empty\n", file[fileIdx]);
        }

        switch (magic) {
            case 0xfe:
                // Binary file
                sendBIN(packetSender, studentNo, infile);
                break;
            case 0xff:
                // tokenized BASIC
                sendBASIC(packetSender, studentNo, infile);
                break;
            case 0x41:
                // ROM image
                sendROM(packetSender, studentNo, infile);
                break;
            default:
                eggog("Error: %s has unsupported file type %02x - cannot send\n", file[fileIdx], magic);
        }
    }
}

void ping(const char* port, int studentNo) 
{
    SerialPort serialPort(port);
    PacketSender packetSender(&serialPort);

    if (serialPort.Setup() != ERR_NONE) {
       eggog("Error: cannot open %s\n", port);
    }

    verbose("ping: opened serial port %s studentNo=%d\n", port, studentNo);

    for(;; pingPong(packetSender, studentNo), usleep(10000));
}

void sendCommand(const char* port, int studentNo, const char* command) 
{
    SerialPort serialPort(port);
    PacketSender packetSender(&serialPort);

    if (serialPort.Setup() != ERR_NONE) {
       eggog("Error: cannot open %s\n", port);
    }

    pingPong(packetSender, studentNo);

    packetSender.SendPacket(&SNDCMDPacket(0, studentNo, command));
    packetSender.ReceivePacket();
}

void halp() 
{
    info("Usage: msxnet [options] [command] file1 file2 ...\n");
    info("  options:\n");
    info("      --verbose|--morbose be verbose | be morbidly verbose\n");
    info("      --port=path         path to serial port\n");
    info("      --dst=number        target workstation address, 0 = broadcast (default 0)\n");
    info("  commands:\n");
    info("      --ping              ping workstation endlessly\n");
    info("      --cpm               issue _CPM command at workstation\n");
    info("      --ncopy             copy files to CP/M ramdisk\n");
    info("      --send              send BIN, ROM or BASIC files\n");
}

int main(int argc, char *argv[]) {
    char portBuf[32] = IOPORT;
    char* port = portBuf;

    int nfiles = 0;
     
    while (1) {
        static struct option long_options[] =
         {
           /* These options set a flag. */
           {"verbose", no_argument,       0, 0},
           {"morbose", no_argument,       0, 1},
           /* These options don't set a flag.
              We distinguish them by their indices. */
           {"port",     required_argument,      0, 2},
           {"dst",      required_argument,      0, 3},
           {"cpm",      no_argument,            0, 4},
           {"ping",     no_argument,            0, 5},
           {"ncopy",    no_argument,            0, 6},
           {"send",     no_argument,            0, 7},
           {0, 0, 0, 0}
         };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long_only(argc, argv, "",
                        long_options, &option_index);
     
        /* Detect the end of the options. */
        if (c == -1) {
            halp();
            break;
        }
     
        switch (c) {
            case 0:
                LogLevel = LOGLEVEL_VERBOSE;
                break;
     
            case 1:
                LogLevel = LOGLEVEL_MORBOSE;
                break;
        
                // serial port file
            case 2:
                port = (char *) malloc(strlen(optarg) + 1);
                strcpy(port, optarg);
                break;
     
                // destination address
            case 3:
                studentNo = atoi (optarg);
                // "127" means "to all"
                if (studentNo == 0) studentNo = 127;
                break;
     
            case 4:
                sendCommand(port, studentNo, " _CPM");
                exit(0);
                break;
     
            case 5:
                ping(port, studentNo);
                // no exit
                break;
     
                // CPM ncopy mode
            case 6:
                if ((nfiles = argc - optind) > 0) {
                    ncopy(port, studentNo, nfiles, &argv[optind]);
                    exit(0);
                }
                else {
                    eggog("No files to send");
                }
                break;

                // SendROM
            case 7:
                if ((nfiles = argc - optind) > 0) {
                    netSend(port, studentNo, nfiles, &argv[optind]);
                    exit(0);
                }
                else {
                    eggog("No files to send");
                }
                break;

            default:
                eggog("Option error");
        }
    }
}

