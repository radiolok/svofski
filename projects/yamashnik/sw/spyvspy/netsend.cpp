// Non-CPM file send commands: BASIC, ROM, BIN
// Stuff formerly implemented in send.c

#include <stdio.h>
#include "diags.h"
#include "serial.h"
#include "sendpacket.h"
#include "ncopy.h"

void sendBASIC(PacketSender& packetSender, int studentNo, FILE* file)
{
    eggog("Sending BASIC is not implemented");
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
    eggog("sending BIN is not implemented");
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
