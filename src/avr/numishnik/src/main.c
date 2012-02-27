
/* Name: main.c
 * Project: AVR USB driver for CDC interface on Low-Speed USB
 * Author: Osamu Tamura
 * Creation Date: 2006-05-12
 * Tabsize: 4
 * Copyright: (c) 2006 by Recursion Co., Ltd.
 * License: Proprietary, free under certain conditions. See Documentation.
 *
 * 2006-07-08   removed zero-sized receive block
 * 2006-07-08   adapted to higher baud rate by T.Kitazawa
 *
 */

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

//#include <stdio.h>
#include "usbdrv.h"
#include "cdcuart.h"
#include "util.h"
#include "timer.h"
#include "globals.h"

enum {
    SEND_ENCAPSULATED_COMMAND = 0,
    GET_ENCAPSULATED_RESPONSE,
    SET_COMM_FEATURE,
    GET_COMM_FEATURE,
    CLEAR_COMM_FEATURE,
    SET_LINE_CODING = 0x20,
    GET_LINE_CODING,
    SET_CONTROL_LINE_STATE,
    SEND_BREAK
};


static PROGMEM const char configDescrCDC[] = {   /* USB configuration descriptor */
    9,          /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    67,
    0,          /* total length of data returned (including inlined descriptors) */
    2,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7),                           /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */

    /* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT,   /* endpoints excl 0: number of endpoint descriptors to follow */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */

    /* CDC Class-Specific descriptor */
    5,           /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    0,           /* header functional descriptor */
    0x10, 0x01,

    4,           /* sizeof(usbDescrCDC_AcmFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    2,           /* abstract control management functional descriptor */
    0x02,        /* SET_LINE_CODING,    GET_LINE_CODING, SET_CONTROL_LINE_STATE    */

    5,           /* sizeof(usbDescrCDC_UnionFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    6,           /* union functional descriptor */
    0,           /* CDC_COMM_INTF_ID */
    1,           /* CDC_DATA_INTF_ID */

    5,           /* sizeof(usbDescrCDC_CallMgtFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    1,           /* call management functional descriptor */
    3,           /* allow management on data interface, handles call management by itself */
    1,           /* CDC_DATA_INTF_ID */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x80|USB_CFG_EP3_NUMBER,        /* IN endpoint number */
    0x03,        /* attrib: Interrupt endpoint */
    8, 0,        /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL,        /* in ms */

    /* Interface Descriptor  */
    9,           /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE,           /* descriptor type */
    1,           /* index of this interface */
    0,           /* alternate setting for this interface */
    2,           /* endpoints excl 0: number of endpoint descriptors to follow */
    0x0A,        /* Data Interface Class Codes */
    0,
    0,           /* Data Interface Class Protocol Codes */
    0,           /* string index for interface */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x01,        /* OUT endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
    8, 0,        /* maximum packet size */
    0,           /* in ms */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x81,        /* IN endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
    8, 0,        /* maximum packet size */
    0,           /* in ms */
};


uchar usbFunctionDescriptor(usbRequest_t *rq)
{

    if(rq->wValue.bytes[1] == USBDESCR_DEVICE){
        usbMsgPtr = (uchar *)usbDescriptorDevice;
        return usbDescriptorDevice[0];
    }else{  /* must be config descriptor */
        usbMsgPtr = (uchar *)configDescrCDC;
        return sizeof(configDescrCDC);
    }
}


uchar               sendEmptyFrame;
static uchar        intr3Status;    /* used to control interrupt endpoint transmissions */

static uchar        stopbit, parity, databit;
static usbDWord_t   baud;

static void resetUart(void)
{

    //uartInit(baud.dword, parity, stopbit, databit);
    irptr    = 0;
    iwptr    = 0;
    urptr    = 0;
    uwptr    = 0;
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

uchar usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */

        if( rq->bRequest==GET_LINE_CODING || rq->bRequest==SET_LINE_CODING ){
            return 0xff;
        /*    GET_LINE_CODING -> usbFunctionRead()    */
        /*    SET_LINE_CODING -> usbFunctionWrite()    */
        }
        if(rq->bRequest == SET_CONTROL_LINE_STATE){
            //UART_CTRL_PORT	= (UART_CTRL_PORT&~(1<<UART_CTRL_DTR))|((rq->wValue.word&1)<<UART_CTRL_DTR);

#if USB_CFG_HAVE_INTRIN_ENDPOINT3
            /* Report serial state (carrier detect). On several Unix platforms,
             * tty devices can only be opened when carrier detect is set.
             */
            if( intr3Status==0 )
                intr3Status = 2;
#endif
        }
#if 1
        /*  Prepare bulk-in endpoint to respond to early termination   */
        if((rq->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_HOST_TO_DEVICE)
            sendEmptyFrame  = 1;
#endif
    }

    return 0;
}


/*---------------------------------------------------------------------------*/
/* usbFunctionRead                                                          */
/*---------------------------------------------------------------------------*/

uchar usbFunctionRead( uchar *data, uchar len )
{

    data[0] = baud.bytes[0];
    data[1] = baud.bytes[1];
    data[2] = baud.bytes[2];
    data[3] = baud.bytes[3];
    data[4] = stopbit;
    data[5] = parity;
    data[6] = databit;

    return 7;
}


/*---------------------------------------------------------------------------*/
/* usbFunctionWrite                                                          */
/*---------------------------------------------------------------------------*/

uchar usbFunctionWrite( uchar *data, uchar len )
{

    /*    SET_LINE_CODING    */
    baud.bytes[0] = data[0];
    baud.bytes[1] = data[1];
    baud.bytes[2] = data[2];
    baud.bytes[3] = data[3];

    stopbit    = data[4];
    parity     = data[5];
    databit    = data[6];

    if( parity>2 )
        parity    = 0;
    if( stopbit==1 )
        stopbit    = 0;

    resetUart();

    return 1;
}


void usbFunctionWriteOut( uchar *data, uchar len )
{

    /*  usb -> rs232c:  transmit char    */
    for( ; len; len-- ) {
        uchar   uwnxt;

        uwnxt = (uwptr+1) & TX_MASK;
        if( uwnxt!=irptr ) {
            tx_buf[uwptr] = *data++;
            uwptr = uwnxt;
        }
    }

    /*  postpone receiving next data    */
    if( uartTxBytesFree()<=HW_CDC_BULK_OUT_SIZE )
        usbDisableAllRequests();
}


static void hardwareInit(void)
{

    /* activate pull-ups except on USB lines */
    USB_CFG_IOPORT   = (uchar)~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));
    /* all pins input except USB (-> USB reset) */
#ifdef USB_CFG_PULLUP_IOPORT    /* use usbDeviceConnect()/usbDeviceDisconnect() if available */
    USBDDR    = 0;    /* we do RESET by deactivating pullup */
    usbDeviceDisconnect();
#else
    USBDDR    = (1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT);
#endif

    /* 250 ms disconnect */
    wdt_reset();
    _delay_ms(250);

#ifdef USB_CFG_PULLUP_IOPORT
    usbDeviceConnect();
#else
    USBDDR    = 0;      /*  remove USB reset condition */
#endif

    /*    USART configuration    */
    baud.dword  = 9600;
    stopbit = 0;
    parity  = 0;
    databit = 8;
    resetUart();
}

// Number definitions from the datasheet
#define NUMI0   (BV6(3,4,5,6,8,9)   >>2)
#define NUMI1   (BV2(3,4)           >>2)
#define NUMI2   (BV5(3,5,7,8,9)     >>2)
#define NUMI3   (BV5(3,4,5,7,8)     >>2)
#define NUMI4   (BV4(3,4,6,7)       >>2)
#define NUMI5   (BV5(4,5,6,7,8)     >>2)
#define NUMI6   (BV6(4,5,6,7,8,9)   >>2)
#define NUMI7   (BV3(3,4,5)         >>2)
#define NUMI8   (BV7(3,4,5,6,7,8,9) >>2)
#define NUMI9   (BV6(3,4,5,6,7,8)   >>2)

#define NUMIA   (BV6(1,2,3,4,5,7)   )
#define NUMIB   (BV5(2,4,5,6,7)     )
#define NUMIC   (BV4(3,4,6,7)       )
#define NUMID   (BV5(1,2,5,6,7)     )
#define NUMIE   (BV5(3,4,5,6,7)     )
#define NUMIF   (BV4(3,4,5,7)       )

const PROGMEM uint8_t number2segment[16] = {NUMI0, NUMI1, NUMI2, NUMI3, NUMI4, 
                                            NUMI5, NUMI6, NUMI7, NUMI8, NUMI9,
                                            NUMIA, NUMIB, NUMIC, NUMID, NUMIE, NUMIF};

/*
uint32_t numitronsSegmentsFromNumbers(uint8_t h1, uint8_t h2, uint8_t m1, uint8_t m2)
{
    return ((uint32_t)number2segment[m2]) |
          (((uint32_t)number2segment[m1]) << 8) |
          (((uint32_t)number2segment[h2]) << 16) |
          (((uint32_t)number2segment[h1]) << 24);
}

uint32_t numitronsSegmentsFromBCD(uint16_t num) 
{
    return numitronsSegmentsFromNumbers(017 & (num>>12), 017 & (num>>8), 
                                        017 & (num>>4), 017 & num);
}
*/

void numitronsInit() 
{
    DDRC |= _BV(0);     // Latch Enable for Macroblocks, output
    PORTC &= ~_BV(0);   // LE = 0, disable
    DDRB |= BV2(3,5);   // MOSI, SCK outputs
    DDRB &= ~_BV(4);    // MISO input

    SPCR = BV4(SPE, MSTR, CPHA, SPR1);
    SPCR |= _BV(DORD);
    SPCR &= ~_BV(CPHA);
}


static void spi_wait() {
    while (!(SPSR & _BV(SPIF)));
}

static volatile uint16_t numitrons = 0;
static volatile uint8_t numitrons_blank = 0;

void numitronsShift(uint8_t bits)
{
    PORTC &= ~_BV(0);   // LE = 0
    SPDR = bits & 0377;
    spi_wait();
    SPDR = (bits >> 8) & 0377;
    spi_wait();
    SPDR = (bits >> 16) & 0377;
    spi_wait();
    SPDR = (bits >> 24) & 0377;
    spi_wait();
    PORTC |= _BV(0);   // latch le data
}

void numitronsBCD(uint16_t num)
{
    PORTC &= ~_BV(0);   // LE = 0
    SPDR = blinkdot | ((numitrons_blank & 1) ? pgm_read_byte(&number2segment[017 & num]) : 0);
    spi_wait();
    SPDR = (numitrons_blank & 2) ? pgm_read_byte(&number2segment[017 & (num>>4)]) : 0;
    spi_wait();
    SPDR = (numitrons_blank & 4) ? pgm_read_byte(&number2segment[017 & (num>>8)]) : 0;
    spi_wait();
    SPDR = (numitrons_blank & 8) ? pgm_read_byte(&number2segment[017 & (num>>12)]) : 0;
    spi_wait();
    PORTC |= _BV(0);   // latch le data
}


int main(void)
{
    char c;
    int gor = 100;
    int dgor = 1;

    numitronsInit();
    numitronsBCD(0);

    wdt_enable(WDTO_1S);
#if USB_CFG_HAVE_MEASURE_FRAME_LENGTH
        oscInit();
#endif
    hardwareInit();
    usbInit();

    timer0_init();

    intr3Status = 0;
    sendEmptyFrame  = 0;

    sei();


    numitrons_blank = 0x0f;
    numitronsBCD(numitrons);


    for(;;){    /* main event loop */
        wdt_reset();
        usbPoll();
        uartPoll();

        if ((gor -= dgor) == 0) {
            numitrons++;
            numitronsBCD(numitrons);
            gor = 512;
        }

        numitronsBCD((the_time.hour << 8) | the_time.minute);

#if 0
        switch ((gor>>4) & 3) {
        case 0: numitrons_blank = 0x7; break;
        case 1: numitrons_blank = 0xe; break;
        case 2: numitrons_blank = 0xd; break;
        case 3: numitrons_blank = 0xb; break;
        }
        numitronsBCD(numitrons);
#endif
        

#if 0
        switch (gor & 3) {
        case 0: numitrons_blank = 3; break;
        case 1: numitrons_blank = 6; break;
        case 2: numitrons_blank = 12; break;
        case 3: numitrons_blank = 9; break;
        }
        numitronsBCD(numitrons);
#endif
#if 0
        numitrons_blank = 0;
        numitronsBCD(0);
        numitrons_blank = 0x0f;
        numitronsBCD(numitrons);
        numitronsBCD(numitrons);
        numitronsBCD(numitrons);
#endif


        if (cdc_dsr()) {
            cdc_putchar(c = cdc_getchar());
            numitronsBCD(numitrons);
            switch (c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                numitronsShift(1<<(c-'0'));
                break;
            case 'a':
                numitronsShift(NUMI4);
                break;
            case ' ':
                numitrons_blank ^= 0x0f;
                break;
            case '.':
                dgor = dgor ? 0 : 1;
                break;
            default:
                break;
            }
        }

#if USB_CFG_HAVE_INTRIN_ENDPOINT3
        /* We need to report rx and tx carrier after open attempt */
        if(intr3Status != 0 && usbInterruptIsReady3()){
            static uchar serialStateNotification[10] = {0xa1, 0x20, 0, 0, 0, 0, 2, 0, 3, 0};

            if(intr3Status == 2){
                usbSetInterrupt3(serialStateNotification, 8);
            }else{
                usbSetInterrupt3(serialStateNotification+8, 2);
            }
            intr3Status--;
        }
#endif
    }

    return 0;
}

