#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
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

FILE *infile;

uint8_t Sector[SECTORSIZE];

unsigned short int studentNo = 0;

struct stat stat_p;

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

int main(int argc, char *argv[]) {
    unsigned int i;
    int argNo = 1;
    int sectNo = 0;
    char portBuf[32] = IOPORT;
    char* port = portBuf;

    if (argc < 3) {
        eggog("Usage: ncopy [-port <e.g. com2 or /dev/ttyS1>] <Student No.> <File> [-verbose]\n");
    }

    if (strcmp("-port", argv[1]) == 0)
    {
        argNo = 3;
        if (argv[2][0] == '/')
        {
            port = argv[2];
        }
        else
        {
            strcpy(portBuf+5, argv[2]);
        }
    }

    SerialPort serialPort(port);
    PacketSender packetSender(&serialPort);

    if (serialPort.Setup() != ERR_NONE) {
       eggog("Error: cannot open %s\n", port);
    }

    studentNo = atoi (argv [argNo]);
    // "127" means "to all"
    if (studentNo == 0) studentNo = 127;

    argNo++;

    if (argc > (argNo+1) && strcmp("-verbose", argv[argNo+1]) == 0) {
        LogLevel = LOGLEVEL_VERBOSE;
    }

    verbose("argc=%d argNo=%d argv[argNo]=%s\n", argc, argNo, argv[argNo]);

    // *************************

    if (strcmp("_PING", argv[argNo]) == 0) {
        for(;; pingPong(packetSender, studentNo), usleep(10000));
    }

    if (strcmp("_CPM", argv[argNo]) == 0)
    {
        pingPong(packetSender, studentNo);

        packetSender.SendPacket(&SNDCMDPacket(0, studentNo, " _CPM"));
        packetSender.ReceivePacket();
        exit(0);
    }

    // check the size of file
    stat(argv[argNo], &stat_p);

    infile = fopen(argv[argNo], "rb");
    if (infile == 0) {
        eggog("Error: cannot open %s\n", argv [argNo]);
    }

    // Send ping
    pingPong(packetSender, studentNo);

    // Create file on the net disk
    {
        NetCreateFilePacket createFile(0, studentNo, argv[argNo]);
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

        packetSender.SendPacket(0, studentNo, NET_MASTER_DATA, Sector, SECTORSIZE);

        do {
            packetSender.SendPacket(0, studentNo, NET_WRITE_FILE, (uint8_t *)packetSender.GetPacketData(), sizeof (*packetSender.GetPacketData()));
        } while (!packetSender.ReceivePacket());

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
    do {
        packetSender.SendPacket(0, studentNo, NET_CLOSE_FILE, (uint8_t *)packetSender.GetPacketData(), sizeof (*packetSender.GetPacketData()));
    } while (!packetSender.ReceivePacket());

    info("\nDone.\n");
}

