#pragma once

#include <inttypes.h>
#include <unistd.h>

#define BAUDRATE 		38400
#define SELECT_WAIT		100000

enum _SIO_ERR {ERR_NONE, ERR_NOFILE, ERR_WTF};

class SerialPort;


class SerialListener {
public:
	virtual int RxHandler() = 0;
};

typedef int (*SerialRxHandler) (SerialPort&); 
typedef int (SerialListener::*SerialRxHandlerMethod) (); 

class SerialPort {
private:
	int m_fd;
	//SerialRxHandler m_RxHandler;
	SerialListener* m_RxListener;
	//SerialRxHandlerMethod m_RxHandlerMethod;

public:
	SerialRxHandlerMethod m_Boblor;

	SerialPort(const char* device);
	~SerialPort() { if (m_fd != -1) close(m_fd); }
	int Setup();

	void SendByte(unsigned char b) const { ::write(m_fd, &b, 1); }

	size_t read(uint8_t* buf, size_t len) const;
	size_t write(uint8_t* buf, size_t len) const;

	//void SetRxHandler(const SerialRxHandler h) { m_RxHandler = h; }
	void SetRxListener(SerialListener* listener) { 
		m_RxListener = listener;
	};

	int waitRx();
};