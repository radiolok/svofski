#pragma once

#include <inttypes.h>
#include <glob.h>
#include "diags.h"
#include "spydata.h"
#include "spy.h"

class NetBDOS
{
private:
    int m_disk;
    glob_t m_globbuf;
    unsigned int m_globbor;

    SpyRequest* m_req;
    SpyResponse* m_res;

    void announce(const char* functionName) {
        info("\033[1mNetBDOS %02X\033[0m: %s", GetBDOSFunc(), functionName);
    }

    int internalGetFileSize(const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (f == 0) return -1;

        fseek(f, 0, SEEK_END);
        long pos = ftell(f);
        if (pos > 737820) {
            pos = 737820;
        }

        return pos;
    }

    void selectDisk() {
        int d = m_req->GetAuxData(0);
        announce("select disk");
        if (d >= 0 && d <= 8) {
            info(" %c:\n", 'A' + m_disk);
        } else {
            info(" <probing drives>\n");
        }
        m_res->SetAuxData(0, 0x01); // number of available drives
        m_res->respond((uint8_t[]){REQ_BYTE, 0});
    }

    void openFile() {
        char filename[13];
        m_req->GetFCB()->GetFileName(filename);

        announce("open file");
        info(" '%s' ", filename);

        m_res->AssignFCB(m_req->GetFCB());

        FILE* f = fopen(filename, "rb");
        do {
            if (f == 0) {
                m_res->SetAuxData(0, 0xff);
                info("not found");
                break;
            } 

            m_res->GetFCB()->Drive = m_disk + 1;

            fseek(f, 0, SEEK_END);
            long pos = ftell(f);
            if (pos > 737820) {
                pos = 737820;
            }

            m_res->GetFCB()->FileSize = pos;
            m_res->GetFCB()->DeviceId = 0x40 + m_disk;
            m_res->GetFCB()->RandomRecord = 0;

            m_res->SetAuxData(0, 0x0);

            info(" size=%d", m_res->GetFCB()->FileSize);
        } while(0);

        fclose(f);

        info("\n");

        m_res->respond((uint8_t[]){REQ_FCB, REQ_BYTE, 0});
    }

    void closeFile() {
        char filename[13];
        m_req->GetFCB()->GetFileName(filename);
        announce("close file");
        info(" '%s'\n", filename);
        m_res->SetAuxData(0, 0x00);
        m_res->respond((uint8_t[]){REQ_BYTE, 0});
    }

    void getCurrentDrive() {
        announce("get current drive");
        info(" Result: %02x\n", m_disk);
        m_res->SetAuxData(0, m_disk);
        m_res->respond((uint8_t[]){REQ_BYTE, 0});
    }

    void searchFirst() {
        char filename[13];
        m_req->GetFCB()->GetFileName(filename);
        announce("search first");
        info(" '%s'\n", filename);

        if (m_globbuf.gl_pathc > 0) {
            globfree(&m_globbuf);
        }
        
        glob(filename, 0, /* errfunc */0, &m_globbuf);

        morbose("glob() pathc=%d\n", m_globbuf.gl_pathc);

        m_globbor = 0;

        searchNext();
    }

    void searchNext() {
        DIRENT dirent;
        int result = 0xff;

        announce("search next-");

        if (m_globbor < m_globbuf.gl_pathc) {
            morbose("glob() first match=%s\n", m_globbuf.gl_pathv[m_globbor]);

            FCB fcb;

            fcb.SetNameExt(m_globbuf.gl_pathv[m_globbor]);
            fcb.FileSize = internalGetFileSize(m_globbuf.gl_pathv[m_globbor]);

            dirent.InitFromFCB(&fcb);

            result = 0;
        } 
        m_globbor++;

        m_res->AssignDMA((uint8_t *)&dirent, sizeof(DIRENT));
        m_res->SetAuxData(0, result);

        m_res->respond((uint8_t[]){REQ_DMA, REQ_BYTE, 0});
    }

    void randomBlockRead() {
        char filename[13];

        announce("random block read");

        m_req->GetFCB()->GetFileName(filename);

        int nrecords = m_req->GetAuxData(0 + DEBUGBLOCKSIZE);
        int recordsize = m_req->GetFCB()->RecordSize();
        if (recordsize != 1) {
            info("ERROR: records of sizes other than 1 not supported\n");
        }
        uint32_t recordno = m_req->GetFCB()->RandomRecord;

        info(" '%s' NRecs=%d RecordSize=%d Cur=%d", filename, nrecords, recordsize, recordno*recordsize);
        morbose("*** DEBUG DATA ***\n DMA= %04x SP=%04x USER SP=%04x [%04x %04x %04x]\n",
            m_req->GetAuxData(0),
            m_req->GetAuxData(1),
            m_req->GetAuxData(2),
            m_req->GetAuxData(3),
            m_req->GetAuxData(4),
            m_req->GetAuxData(5)
            );

        m_res->AssignFCB(m_req->GetFCB());

        FILE* f = fopen(filename, "rb");
        do {
            if ((recordno*recordsize >= m_req->GetFCB()->FileSize) ||
                (fseek(f, recordno*recordsize, SEEK_SET)) != 0) {
                m_res->SetDMASize(0);
                m_res->SetAuxData(0,0);
                m_res->SetAuxData(1,1);
                info("<eof>");
                break;
            }
            m_res->AllocDMA(nrecords*recordsize);
            int recordsread = fread(m_res->GetDMA(), recordsize, nrecords, f);
            m_res->SetDMASize(recordsread * recordsize);
            m_res->SetAuxData(0, recordsread); // records read
            m_res->SetAuxData(1, 0);           // no error
            m_res->GetFCB()->RandomRecord += recordsread;
        } while(0);
        info("\n");

        fclose(f);
        m_res->respond((uint8_t[]){REQ_WORD, REQ_DMA, REQ_FCB, REQ_BYTE, 0});
    }

    void randomRead() {
        char filename[13];
        m_req->GetFCB()->GetFileName(filename);

        int recordsize = 128;

        uint32_t recordno = m_req->GetFCB()->RandomRecord;
        recordno = recordno & 0x00ffffff;

        m_res->AssignFCB(m_req->GetFCB());

        announce("random read");
        info(" '%s' Rec=%d Ofs=%d", filename, recordno, recordno*recordsize);

        morbose("*** DEBUG DATA ***\n DMA= %04x SP=%04x USER SP=%04x [%04x %04x %04x]\n",
            m_req->GetAuxData(0),
            m_req->GetAuxData(1),
            m_req->GetAuxData(2),
            m_req->GetAuxData(3),
            m_req->GetAuxData(4),
            m_req->GetAuxData(5)
            );

        m_res->AllocDMA(recordsize);
        m_res->SetDMASize(recordsize);
        memset(m_res->GetDMA(), 0, recordsize);
        m_res->SetAuxData(0,1);

        FILE* f = fopen(filename, "rb");
        do {
            if ((recordno*recordsize >= m_req->GetFCB()->FileSize) ||
                (fseek(f, recordno*recordsize, SEEK_SET) != 0) ||
                (ftell(f) != recordno*recordsize)) {
                info("<eof>");
                break;
            }

            int bytesread = fread(m_res->GetDMA(), 1, recordsize, f);

            // Set result
            m_res->SetAuxData(0, bytesread > 0 ? 0 : 1); 

            // Update Current Record within Extent for sequential read
            m_res->GetFCB()->CurrentRecord = recordno; 
            m_res->GetFCB()->RecordSizeLo = 0;
            m_res->GetFCB()->RecordSizeHi = recordsize;
        } while(0);
        fclose(f);
        info("\n");

        m_res->respond((uint8_t[]){REQ_DMA, REQ_FCB, REQ_BYTE, 0});
    }

    void sequentialRead() {
        announce("sequential read");
        char filename[13];
        int recordsize = 128;
        m_req->GetFCB()->GetFileName(filename);
        

        uint32_t recordno = m_req->GetFCB()->CurrentRecord;
        recordno = recordno & 0x00ffffff;

        m_res->AssignFCB(m_req->GetFCB());

        info(" '%s' Rec=%d Ofs=%d\n", filename, recordno, recordno*recordsize);

        // Always return a full record, padded with zeroes if EOF
        m_res->AllocDMA(recordsize);
        m_res->SetDMASize(recordsize);
        memset(m_res->GetDMA(), 0, recordsize);
        m_res->SetAuxData(0,1);

        FILE* f = fopen(filename, "rb");
        do {
            if ((recordno*recordsize >= m_req->GetFCB()->FileSize) ||
                (fseek(f, recordno*recordsize, SEEK_SET) != 0) ||
                (ftell(f) != recordno*recordsize)) {
                info("<eof>");
                break;
            }

            int bytesread = fread(m_res->GetDMA(), 1, recordsize, f);
            // set result
            m_res->SetAuxData(0, bytesread > 0 ? 0 : 1); 
        } while(0);
        fclose(f);

        m_res->GetFCB()->RecordSizeLo = 0;
        m_res->GetFCB()->RecordSizeHi = recordsize;

        m_res->GetFCB()->CurrentRecord = m_res->GetFCB()->CurrentRecord + 1;
        if (m_res->GetFCB()->CurrentRecord == 0x80) {
            m_res->GetFCB()->CurrentRecord = 0;
            m_res->GetFCB()->SetExtent(m_res->GetFCB()->GetExtent() + 1);
        }
        verbose(" advance: record no=%d", m_res->GetFCB()->CurrentRecord);
        info("\n");

        m_res->respond((uint8_t[]){REQ_DMA, REQ_FCB, REQ_BYTE, 0});
    }

    void getLoginVector() {
        announce("get login vector");
        info("\n");
        m_res->SetAuxData(0, 0x0001); // LSB corresponds to drive A
        m_res->respond((uint8_t[]){REQ_WORD, 0});
    }

    void getFileSize() {
        char filename[13];

        announce("get file size");

        m_req->GetFCB()->GetFileName(filename);
        int size = internalGetFileSize(filename);
        m_res->AssignFCB(m_req->GetFCB());
        m_res->GetFCB()->FileSize = size;

        m_res->SetAuxData(0, size == -1 ? 0xff : 0);
        m_res->respond((uint8_t[]) {REQ_FCB, REQ_BYTE, 0});
    }

    void getAllocInfo() {
        announce("get allocation information");

        uint8_t drive = m_req->GetAuxData(0);
        uint8_t dpb[32];

        memset(dpb, 0, 32);

        m_res->AllocDMA(32);
        m_res->SetDMASize(32);
        m_res->AssignDMA((uint8_t *)dpb, 32);

        m_res->SetAuxData(0, 0xF195);
        m_res->SetAuxData(1, 0xE595);
        m_res->SetAuxData(2, 0x200);    // BC, sector size
        m_res->SetAuxData(3, 0x2c9);    // DE, total clusters
        m_res->SetAuxData(4, 0x2c9);    // HL, free clusters
        m_res->SetAuxData(5, 2);        // A, sectors per cluster

        m_res->respond((uint8_t[]) {
            /* pointer to DPB */ REQ_WORD,  /* IX  = F195 */
            /* DPB */            REQ_DMA,
            /* pointer to FAT */ REQ_WORD,  /* IY = E595 */
            /* FAT */            REQ_DMA,
            /* Sector size */    REQ_WORD,
            /* Total clusters */ REQ_WORD,
            /* Free clusters */  REQ_WORD,
            /* Sctrs per clstr */REQ_BYTE
        });
    }

private:
    int test_fileNameFromPCB() {
        char filename[13];
        FCB fcb;
        memcpy(&fcb.NameExt, "AUTOEXECBAT", 11);
        fcb.GetFileName(filename);
        info("test_fileNameFromPCB 1 '%s'\n", filename);

        if (strcmp("AUTOEXEC.BAT", filename) != 0) return 0;

        memcpy(&fcb.NameExt, "FOO     BAR", 11);
        fcb.GetFileName(filename);
        info("test_fileNameFromPCB 2 '%s'\n", filename);
        if (strcmp("FOO.BAR", filename) != 0) return 0;

        memcpy(&fcb.NameExt, "FUUU    SO ", 11);
        fcb.GetFileName(filename);
        info("test_fileNameFromPCB 3 '%s'\n", filename);
        if (strcmp("FUUU.SO", filename) != 0) return 0;

        return 1;
    }

    int test_FCBFromFileName() {
        FCB fcb;
        fcb.SetNameExt("foo.bar");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("foo.b");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("foo.");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("foo");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));


        fcb.SetNameExt(".e");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt(".err");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("autoexec.bat");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("/home/babor/autoexec.bat");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("/home/babor/longnameislong.bat");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("/home/babor/okname.longext");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        fcb.SetNameExt("okname.longext");
        dump(info, (uint8_t *) &fcb, sizeof(FCB));

        return 1;
    }

    int test_searchFirst() {
        m_req = new SpyRequest();
        m_req->GetFCB()->SetNameExt("AUTOEXEC.???");
        searchFirst();

        m_req = new SpyRequest();
        m_req->GetFCB()->SetNameExt("NOSUCH.FIL");
        searchFirst();

        delete m_req;

        return 1;
    }

public:
    NetBDOS() : m_disk(0) {}

    ~NetBDOS() {
        if (m_globbuf.gl_pathc > 0) {
            globfree(&m_globbuf);
        }
    }

    int test() {
        
        return test_fileNameFromPCB() 
            && test_FCBFromFileName()
            && test_searchFirst();

    }

    int GetBDOSFunc() const { return m_req->GetFunc() + 14; }

    void ExecFunc(SpyRequest* request, SpyResponse* response) {
        m_req = request;
        m_res = response;

        switch(request->GetFunc()) {
        case F0E_SELECT_DISK:
            selectDisk();
            break;
        case F0F_OPEN_FILE:
            openFile();
            break;
        case F10_CLOSE_FILE:
            closeFile();
            break;
        case F11_SEARCH_FIRST:
            searchFirst();
            break;
        case F12_SEARCH_NEXT:
            searchNext();
            break;
        case F13_DELETE:
            break;
        case F14_SEQ_READ:
            sequentialRead();
            break;
        case F15_SEQ_WRITE:
            break;
        case F16_CREAT:
            break;
        case F17_RENAME:
            break;
        case F18_GETLOGINVECTOR:
            getLoginVector();
            break;
        case F19_GET_CURRENT_DRIVE:
            getCurrentDrive();
            break;
        case F1B_GET_ALLOC_INFO:
            getAllocInfo();
            break;
        case F21_RANDOM_READ:
            randomRead();
            break;
        case F22_RANDOM_WRITE:
            break;
        case F23_GET_FILE_SIZE:
            getFileSize();
            break;
        case F24_SET_RANDOM_RECORD:
            break;
        case F26_RANDOM_BLOCK_WRITE:
            break;
        case F27_RANDOM_BLOCK_READ:
            randomBlockRead();
            break;
        case F28_RANDOM_WRITE_ZERO:
            break;
        case F2F_ABS_SECTOR_READ:
            break;
        case F30_ABS_SECTOR_WRITE:
            break;
        default:
            break;
        }
    }
};
