#include <stdio.h>
#include <sys/stat.h>
#include "serial.h"
#include "sendpacket.h"

#define SECTORSIZE  128

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
