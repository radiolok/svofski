#ifndef SERIAL_H_
#define SERIAL_H_

#define RXBUF_SIZE 8
#define SERIAL_BAUDRATE 9600

class Serial
{
public:
	Serial();
	void putchar(char c);
	int getchar();
	int avail() const { return head != tail; }
	void puts(const char *c);	

public:
	void receive();
	
private:	
	int head, tail;
	char rxbuf[RXBUF_SIZE];
};

extern Serial com;

#endif /*SERIAL_H_*/
