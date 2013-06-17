#pragma once

#include <inttypes.h>
#include <strings.h>
#include <glob.h>
#include "serial.h"
#include "diags.h"



enum {
	REQ_BYTE = 1, REQ_WORD, REQ_FCB, REQ_DMA, REQ_END = 0,
};



class Spy 
{
private:
	SerialPort m_serial;

	const char* m_argv0;

	int m_studentNo;
	int m_nfiles;
	char** m_file;

	char *m_ExePath;

	uint8_t* m_MSXRAM;
	uint8_t* m_BDOS;
	uint8_t* m_COMFile;

	void extractExePath();
	int loadFile(const char* filename, uint8_t** buf, int expectedSize);

public:
	Spy(const char* port, const char* argv0, int studentNo, int nfiles, char* file[]) 
		: m_serial(port),
		  m_argv0(argv0),
		  m_studentNo(studentNo),
		  m_nfiles(nfiles),
		  m_file(file),
		  m_ExePath(0),     
		  m_MSXRAM(0),
		  m_BDOS(0),
		  m_COMFile(0)
	{}

	~Spy() 
	{
		if (m_ExePath) delete m_ExePath;
		if (m_MSXRAM) delete m_MSXRAM;
		if (m_BDOS) delete m_BDOS;
		if (m_COMFile) delete m_COMFile;
	}

	int initData();

	int Bootstrap();
};

enum SpyState { SPY_BOOTSTRAP = 0, SPY_POLL = 1, SPY_GETFUNC = 2, SPY_RXDATA = 3};

enum SpyRequestCode {
	F0E_SELECT_DISK			= 0,					
	F0F_OPEN_FILE,			
	F10_CLOSE_FILE,			
	F11_SEARCH_FIRST,		
	F12_SEARCH_NEXT, 		
	F13_DELETE,				
	F14_SEQ_READ,			
	F15_SEQ_WRITE, 			
	F16_CREAT,				
	F17_RENAME,				
	F18_GETLOGINVECTOR,		
	F19_GET_CURRENT_DRIVE,	

	F1B_GET_ALLOC_INFO 		= 0x0D, 	
	F21_RANDOM_READ			= 0x13,		
	F22_RANDOM_WRITE,		
	F23_GET_FILE_SIZE,		
	F24_SET_RANDOM_RECORD,	

	F26_RANDOM_BLOCK_WRITE 	= 0x18,
	F27_RANDOM_BLOCK_READ,	
	F28_RANDOM_WRITE_ZERO,	

	F2F_ABS_SECTOR_READ		= 0x21,	
	F30_ABS_SECTOR_WRITE	= 0x22,	
};


struct FCB {
/*  0 */	uint8_t		Drive;
/*  1 */	uint8_t		NameExt[11];
/* 12 */	uint16_t	CurrentBlock;
/* 14 */	uint16_t 	RecordSize;

/* 16 */	uint32_t	FileSize;

/* 20 */	uint16_t	Date;
/* 22 */	uint16_t	Time;

/* 24 */	uint8_t		DeviceId;			// 40h + Drive (A=40h)
/* 25 */	uint8_t		DirectoryLocation;
/* 26 */	uint16_t	TopCluster;
/* 28 */	uint16_t	LastClusterAccessed;
/* 30 */	uint16_t 	RelativeLocation;

/* 32 */	uint8_t		CurrentRecord;		// for seq access
/* 33 */	uint32_t	RandomRecord;
/* 36 */	uint32_t	Padding;
/* 40 */	uint8_t 	Padding2[3];

	FCB() {
		memset(this, 0, sizeof(FCB));
	}

	int SetNameExt(const char* fname) {
		char* shrt = (shrt = strrchr(fname, '/')) ? shrt + 1 : (char *)fname;

		char* dot = strchr(shrt, '.');
		int namelen = dot == 0 ? strlen(shrt) : dot - shrt;
		int extlen = strlen(shrt) - namelen - 1;
		if (namelen > 8 || extlen > 3) {
			info("ERROR: filenames must be 8.3\n");
			return 0;
		}

		memset(NameExt, ' ', sizeof(NameExt));
		memcpy(NameExt, shrt, namelen);
		if (extlen > 0) {
			memcpy(NameExt + 8, shrt + namelen + 1, extlen);
		}		

		return 1;
	}

	void GetFileName(char* normalname) {
		int i = 0;
		for (int c; i < 8 && (c = NameExt[i]) != ' '; normalname[i] = c, i++);
		normalname[i++] = '.';
		for (int s = 8, c; s < 11 && (c = NameExt[s]) != ' '; normalname[i] = c, i++, s++);
		normalname[i] = 0;
	}
} __attribute__((packed));

struct DIRENT {
			uint8_t AlwaysFF;
/*  0 */	char NameExt[11];
/* 11 */	uint8_t Attrib;
/* 12 */	uint8_t Space[10];
/* 22 */	uint16_t Time;
/* 24 */	uint16_t Date;
/* 26 */	uint16_t Cluster;
/* 28 */	uint32_t FileSize;

	DIRENT() {
		memset(this, 0, sizeof(DIRENT));
		AlwaysFF = 0xff;
	}

	void InitFromFCB(FCB* fcb) {
		memcpy(NameExt, fcb->NameExt, sizeof(NameExt));
		FileSize = fcb->FileSize;
	}
} __attribute__((packed));;

class SpyResponse;

class SpyRequest 
{
	FCB		m_FCB;
	uint8_t m_DMA[65536];
	int 	m_dmasize;
	int 	m_data[8];
	uint8_t m_result;
	uint8_t m_func;

	uint8_t* m_rxpattern;
	int m_rxcursor;

	int m_datacursor;
	int m_bufoffset;

public:
	uint8_t getFunc() const { return m_func; }
	FCB* GetFCB() { return &m_FCB; }
	const uint8_t* getDMA() const { return m_DMA; }
	int getDMASize() const { return m_dmasize; }
	int getAuxData(int index) const { return m_data[index]; }

	void setFunc(const uint8_t func) {
		m_func = func;
	}

	void expect(const uint8_t RxPattern[]) {
		m_rxpattern = new uint8_t[strlen((const char*)RxPattern) + 1];
		strcpy((char *)m_rxpattern, (const char *)RxPattern);
		m_rxcursor = 0;
		m_datacursor = 0;
		m_bufoffset = 0;
		m_dmasize = 0;
	}

	uint16_t getWordBigEndian(uint8_t *cursor) {
		//morbose("\n getWordBigEndian: %x,%x = %x\n",
		//	cursor[0], cursor[1],
		//	(uint16_t)(cursor[1] | ((uint16_t)cursor[0] << 8)));
			
		return cursor[1] | ((uint16_t)cursor[0] << 8);
	}

	int eat(uint8_t* buf, int len) {
		int avail = len - m_bufoffset;

		uint8_t* cursor = buf + m_bufoffset;

		// nothing to expect, we're good
		if (m_rxpattern[m_rxcursor] == 0) return 1;

		// something is expected but nothing is given yet
		if (avail == 0) return 0;

		// check what have we got here
		switch(m_rxpattern[m_rxcursor]) {
		case REQ_BYTE:
			m_data[m_datacursor] = cursor[0];	// get byte
			//morbose("REQ_BYTE: %x\n", m_data[m_datacursor]);
			m_bufoffset += 1;					// input offset ++
			m_datacursor++;						// data offset ++
			m_rxcursor++; 						// next expected token
			break;

		case REQ_WORD:
			if (avail >= 2) {				
				// get word: big endian
				m_data[m_datacursor] = getWordBigEndian(cursor);
				m_bufoffset += 2;				// advance by 2
				m_datacursor++;
				m_rxcursor++;					// next token
			}
			break;

		case REQ_FCB:
			if (avail >= (int) sizeof(FCB) + 2) {
				// first 2 bytes are length of FCB: big endian
				// validate just for fun
				int length = getWordBigEndian(cursor);
				cursor += 2;

				if (length != sizeof(FCB)) {
					info("error: received FCB length=%d, must be %zd",
						length, sizeof(FCB));
				}

				// copy FCB data
				memcpy(&m_FCB, cursor, sizeof(FCB));

				m_bufoffset += 2 + sizeof(FCB); // shift buffer offset
				m_rxcursor++; 					// next token

				verbose("Received NetFCB, next state=%d\n", m_rxpattern[m_rxcursor]);
				dump(verbose, (uint8_t*) &m_FCB, sizeof(FCB));
			}
			break;

		case REQ_DMA:
			if (avail >= 2) {
				int dmasize = getWordBigEndian(cursor);
				if (avail >= dmasize + 2) {
					// got all data, hoorj
					m_dmasize = dmasize;
					cursor += 2;
					memcpy(&m_DMA, cursor, m_dmasize);

					m_bufoffset += 2 + m_dmasize;
					m_rxcursor++;
				}
			}
			break;

		case REQ_END:
		default:
			break;
		}

		// return 1 when reached end of expected pattern
		return (m_rxpattern[m_rxcursor] == 0) ? 1 : 0;
	}

	int NeedsData() const { return m_rxpattern[0] != 0; }
};


class SpyTransport : SerialListener
{
private:
	SerialPort* m_serial;
	SpyState m_state;
	uint8_t m_func;
	SpyRequest* m_rq;

	uint8_t m_rxbuf[65536];
	int m_rxpos;


public:
	SpyTransport(SerialPort* serial) 
		: m_serial(serial),
		  m_state(SPY_BOOTSTRAP)
		{
			m_serial->SetRxListener(this);
		}
	
	void SendByte(uint8_t b, printfunc p = 0) const;
	void SendWord(uint16_t w, printfunc p = 0) const;
	void SendChunk(uint8_t* data, int length, printfunc p = 0) const;

	int RxHandler();

	void SendMemory(uint8_t* data, uint16_t addrDst, int length) const;

	void SendCommand(uint8_t cmd) const { SendByte(cmd); }

	int Poll(uint8_t studentNo);
	int ReceiveRequest(SpyRequest* request);
	int TransmitResponse(SpyResponse* response);  
};

class SpyResponse 
{
private:
	FCB*		m_FCB;
	uint8_t* 	m_DMA;
	int 		m_dmasize;
	int 		m_data[8];
	uint8_t 	m_result;

	uint8_t* m_txpattern;

public:
	SpyResponse() : m_FCB(0), m_DMA(0) {}

	~SpyResponse() {
		if (m_FCB) delete m_FCB;
		if (m_DMA) delete[] m_DMA;
		if (m_txpattern) delete[] m_txpattern;
	}

	void respond(const uint8_t TxPattern[]) {
		m_txpattern = new uint8_t[strlen((const char*)TxPattern) + 1];
		strcpy((char *)m_txpattern, (const char *)TxPattern);
	}

	void AssignFCB(const FCB* fcb) {
		m_FCB = new FCB();
		memcpy(m_FCB, fcb, sizeof(FCB));
	}

	FCB* GetFCB() const { return m_FCB; }

	void AllocDMA(int length) {
		m_dmasize = length;
		m_DMA = new uint8_t[m_dmasize];
	}

	uint8_t* getDMA() const { return m_DMA;	}

	void SetDMASize(int n) {
		m_dmasize = n;
	}

	void AssignDMA(const uint8_t *data, int length) {
		if (m_DMA) delete[] m_DMA;
		m_dmasize = length;
		m_DMA = new uint8_t[m_dmasize];
		memcpy(m_DMA, data, m_dmasize);
	}

	void SetAuxData(uint8_t idx, int value)  {
		m_data[idx] = value;
	}

	void emit(SpyTransport* transport) {
		morbose("SpyResponse::emit: ");
		for(int done = 0, txcursor = 0, datacursor = 0; !done;) {
			switch(m_txpattern[txcursor]) {
			case REQ_END:
				morbose("<end>\n");
				done = 1;
				break;
			case REQ_BYTE:
				transport->SendByte(m_data[datacursor], morbose);
				datacursor++;
				txcursor++;
				usleep(10000);
				break;
			case REQ_WORD:
				transport->SendWord(m_data[datacursor], morbose);
				datacursor++;
				txcursor++;
				usleep(10000);
				break;
			case REQ_FCB:
				transport->SendWord(sizeof(FCB), morbose);
				usleep(10000);
				transport->SendChunk((uint8_t*)m_FCB, sizeof(FCB), morbose);
				txcursor++;
				usleep(10000);
				break;
			case REQ_DMA:
				transport->SendWord(m_dmasize, morbose);
				usleep(10000);
				transport->SendChunk(m_DMA, m_dmasize, morbose);
				usleep(10000);
				txcursor++;
				break;
			}
		}
	}
};

class NetBDOS
{
private:
	int m_disk;
	glob_t m_globbuf;
	int m_globbor;

	SpyRequest* m_req;
	SpyResponse* m_res;

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
		int d = m_req->getAuxData(0);
		info("NetBDOS: select disk");
		if (d >= 0 && d <= 8) {
		 	info("%c:\n", 'A' + m_disk);
		} else {
			info(" <probing drives>\n");
		}
		m_res->SetAuxData(0, 0x01); // number of available drives
		m_res->respond((uint8_t[]){REQ_BYTE, 0});
	}

	void openFile() {
		char filename[13];
		m_req->GetFCB()->GetFileName(filename);
		info("NetBDOS: openFile '%s' ", filename);
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

			info(" pos=%d %d ", pos, (int)pos);

			m_res->GetFCB()->FileSize = pos;
			m_res->GetFCB()->DeviceId = 0x40 + m_disk;
			m_res->GetFCB()->RandomRecord = 0;

			m_res->SetAuxData(0, 0x0);

			info("found: size=%d", m_res->GetFCB()->FileSize);
		} while(0);

		fclose(f);

		info("\n");

		m_res->respond((uint8_t[]){REQ_FCB, REQ_BYTE, 0});
	}

	void closeFile() {
		char filename[13];
		m_req->GetFCB()->GetFileName(filename);
		info("NetBDOS: closeFile '%s'\n", filename);
		m_res->SetAuxData(0, 0x00);
		m_res->respond((uint8_t[]){REQ_BYTE, 0});
	}

	void getCurrentDrive() {
		info("NetBDOS: getCurrentDrive, Result: %02x\n", m_disk);
		m_res->SetAuxData(0, m_disk);
		m_res->respond((uint8_t[]){REQ_BYTE, 0});
	}

	void searchFirst() {
		char filename[13];
		m_req->GetFCB()->GetFileName(filename);
		info("Search first: '%s'\n", filename);

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
		m_req->GetFCB()->GetFileName(filename);

		int nrecords = m_req->getAuxData(0);
		int recordsize = m_req->GetFCB()->RecordSize;
		if (recordsize != 1) {
			info("ERROR: records of sizes other than 1 not supported\n");
		}

		dump(morbose, (uint8_t *) m_req->GetFCB(), sizeof(FCB));

		uint32_t recordno = m_req->GetFCB()->RandomRecord;

		m_res->AssignFCB(m_req->GetFCB());

		FILE* f = fopen(filename, "rb");
		do {
			if (fseek(f, recordno*recordsize, SEEK_SET) != 0) {
				m_res->SetDMASize(0);
				m_res->SetAuxData(0,0);
				m_res->SetAuxData(1,1);
				info("seek error");
				break;
			}
			m_res->AllocDMA(nrecords*recordsize);
			int recordsread = fread(m_res->getDMA(), recordsize, nrecords, f);
			m_res->SetDMASize(recordsread * recordsize);
			m_res->SetAuxData(0, recordsread); // records read
			m_res->SetAuxData(1, 0); 		   // no error
			m_res->GetFCB()->RandomRecord += recordsread;
		} while(0);


		fclose(f);
		m_res->respond((uint8_t[]){REQ_WORD, REQ_DMA, REQ_FCB, REQ_BYTE, 0});
	}

	void randomRead() {
		char filename[13];
		m_req->GetFCB()->GetFileName(filename);

		int nrecords = 1;
		int recordsize = 128;

		dump(morbose, (uint8_t *) m_req->GetFCB(), sizeof(FCB));

		uint32_t recordno = m_req->GetFCB()->RandomRecord;
		recordno = recordno & 0x00ffffff;

		m_res->AssignFCB(m_req->GetFCB());

		FILE* f = fopen(filename, "rb");
		do {
			if (fseek(f, recordno*recordsize, SEEK_SET) != 0) {
				m_res->SetDMASize(0);
				m_res->SetAuxData(0,0);
				m_res->SetAuxData(1,1);
				info("seek error");
				break;
			}
			m_res->AllocDMA(nrecords*recordsize);
			int recordsread = fread(m_res->getDMA(), recordsize, nrecords, f);
			m_res->SetDMASize(recordsread * recordsize);
			m_res->SetAuxData(1, 0); 		   // no error
			m_res->GetFCB()->RandomRecord += recordsread;
		} while(0);


		fclose(f);
		m_res->respond((uint8_t[]){REQ_DMA, REQ_FCB, REQ_BYTE, 0});
	}

	void getLoginVector() {
		info("NetBDOS: get login vector\n");
		m_res->SetAuxData(0, 0x0001); // LSB corresponds to drive A
		m_res->respond((uint8_t[]){REQ_WORD, 0});
	}

	void getFileSize() {
		char filename[13];
		m_req->GetFCB()->GetFileName(filename);
		int size = internalGetFileSize(filename);
		m_res->AssignFCB(m_req->GetFCB());
		m_res->GetFCB()->FileSize = size;

		m_res->SetAuxData(0, size == -1 ? 0xff : 0);
		m_res->respond((uint8_t[]) {REQ_FCB, REQ_BYTE, 0});
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

	void ExecFunc(SpyRequest* request, SpyResponse* response) {
		m_req = request;
		m_res = response;

		switch(request->getFunc()) {
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


