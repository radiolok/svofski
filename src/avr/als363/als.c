//!\file

#include <inttypes.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>


#define NMODULES	5

#define SPI_MASTER

void init_io() {
	DDRC=0xff;	// outputs
	DDRD=0xff;
	DDRB=0xff;
	PORTC = 0x00;
	PORTD = 0x00;
	PORTB = 0x00;
	
	TCCR0 = 5;	// clk/1024
	TIMSK |= _BV(TOIE0);
	
	// SPI 
#ifdef SPI_MASTER
	// all output is set already
	SPCR = _BV(SPIE) | _BV(SPE) | _BV(MSTR) | _BV(SPR0);
	PORTB |= _BV(2); // slave select hi (off)
#else 
	SPCR = _BV(SPIE) | _BV(SPE); // not master
	DDRB &= ~(_BV(2) | _BV(3)); // SS and MOSI inputs, MISO output
#endif	
}

typedef union _bitfield {
	uint8_t	byte;
	struct {
		uint8_t a : 1;
		uint8_t b : 1;
		uint8_t c : 1;
		uint8_t d : 1;
		uint8_t e : 1;
		uint8_t f : 1;
		uint8_t g : 1;
		uint8_t h : 1;
	} bits;
} BitField;

void busc(uint8_t bits) {
	uint8_t bbits, dbits, dbitsn, bbitsn;
	BitField b;
	
	b.byte = bits;
	dbits = (b.bits.a << 5) | (b.bits.d << 7) | (b.bits.e << 6) | (b.bits.g << 4);
	bbits = (b.bits.b << 1) | (b.bits.c << 0) | (b.bits.f << 7);

	b.byte = ~bits;
	dbitsn = (b.bits.a << 5) | (b.bits.d << 7) | (b.bits.e << 6) | (b.bits.g << 4);
	bbitsn = (b.bits.b << 1) | (b.bits.c << 0) | (b.bits.f << 7);

	PORTD &= ~dbits; PORTD |= dbitsn;
	PORTB &= ~bbits; PORTB |= bbitsn;
}

enum EPORT {
	PB = 0,
	PC,
	PD
};

static const uint8_t amap[] = {PC,4, 
								PC,3,
								PB,6,
								PC,1,
								PC,0,
								PD,3,
								PD,2,
								PD,1,
								PD,0,
								PC,5};
static volatile const uint8_t* pmap[] = {&PORTB,&PORTC,&PORTD};								

static volatile uint8_t delay_var;

void sela(uint8_t column, uint16_t dtime, uint8_t off) {
	volatile uint8_t* port = NULL;
	uint8_t  bit = 0;
	
	static uint8_t lastbit = 0;
	static uint8_t* lastport = NULL;
	
	
	bit = amap[1+(column<<1)];
	port = pmap[amap[column<<1]];
	
	*port |= _BV(bit);
	if (off && lastport != NULL && lastbit != bit) {
		*lastport &= ~_BV(lastbit);
	}
	if (dtime == 0) for (delay_var = 0; delay_var < 80; delay_var++);
	_delay_us(dtime);
	
	lastport = port;
	lastbit = bit;
	//if (off) *port &= ~_BV(bit);
}

uint8_t countbits(uint8_t b) {
	uint8_t i;
	uint8_t result;
	
	for (i = result = 0; i < 8; i++, result += b&1, b >>= 1);

	return result;
}

extern const prog_uint8_t charrom[];

volatile uint8_t char1, char2;

//static const const prog_uint8_t message[] ="1234567890*КРОВАВЫЙ КЛОКАН \002\002 АЛС363А: Нанотехнологии в цирке   Считается, что торсия возникла при переходе от пелагического к бентосному образу жизни, поскольку при существовании в бентосе экзогастрическая доторсионная раковина весьма неудобна. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.    ";
static const const prog_uint8_t message[] =
"           \
 Цветной компьютер Вектор-06ц\007в FPGA   \
 CPU: T80 (КР580ВМ80)   \
 Таймер ВИ53   \
 64Кб ОЗУ + квазидиск 256Кб   \
 Видео 256х256 и 512х256 16 из 256 цветов   \
 Синтезатор YM-2149   \
 Эмулятор дисковода на SD картах           \
          http://sensi.org/~svo          \002    \007    \
 Графопостроитель  МОТОРИЙ  \007  \
 Шаговые двигатели и направляющие из принтеров   \
 Контроллер на A3984 и ATmega644     \
 Ограниченное подмножество HP-GL     \
 Возможна установка лазера для гравировки \
          http://sensi.org/~svo          \002    \007    ";

static uint16_t msgidx = 0;

#define SCROLL_PACE	16
static volatile uint8_t  scroll_pace = SCROLL_PACE;
static volatile uint8_t scroll_skip = SCROLL_PACE;


static volatile uint16_t duty_time = 400;

enum _MODES {
	START,
	NORMAL,
};

static volatile uint8_t mode = 0;
static volatile uint8_t start_column = 0;

int main() {
	uint8_t i, chr = 192, mask, col, colbits, bitcount;
	uint8_t sh = 1;
	uint8_t dir = 0;
	uint16_t dtime;
	uint16_t ttime;
	
	init_io();

	PORTD = 255;
	PORTB = 255;
	PORTD &= ~_BV(4);
	PORTD &= ~_BV(7);
	PORTD &= ~_BV(5);

	mask = 0xff;
	
	sei();
	
	for(i = 0; i < 10; i++) {
		sela(i, 0, 1);
	}
	
	for (i = 0;;i++) {
		if (mode == START) {
			busc(0377);
			sela(start_column, dtime, 1);
			busc(0);
			sela(start_column, dtime, 1);
		} else {
			uint16_t base = 5*char1;
			for (col = 0; col < 10; col++) {
				colbits = pgm_read_byte(&charrom[base+(col%5)]);
				bitcount = countbits(colbits);
				
				switch (bitcount) {
				case 0: dtime = 200; break;
				case 1: dtime = 0; break;	
				case 2: dtime = 2; break;
				case 3: dtime = 100; break;
				case 4: dtime = 400; break;
				default:
						dtime = 600; break;
				}
				busc(colbits);
				sela(col, dtime, 1);

				if (col == 4) {
					base = 5*char2;
				}
			}
		}
	}	
}

static volatile uint8_t spi_count;
uint8_t chardelay = 0;
void big_push() {
	uint8_t c;
	
	if (spi_count < (NMODULES-1)*2) {
		PORTB &= ~_BV(2); 
		c = pgm_read_byte(&message[msgidx+spi_count++]);
		SPDR = c;
	} else {
		PORTB |= _BV(2); 
		// two last ones
		char1 = pgm_read_byte(&message[msgidx+spi_count++]);
		char2 = pgm_read_byte(&message[msgidx+spi_count++]);

		if (chardelay != 0) {
			chardelay--;
			if (chardelay == 0) msgidx++;
			return;
		}
	
		if (pgm_read_byte(&message[msgidx+spi_count]) == 7) {
			if (chardelay == 0) chardelay = 2;
			return;
		}
	
		switch (char2) {
		case 0:
			msgidx = 0;
			break;
		default:
			msgidx++;
		}
	}
}

void SIG_OVERFLOW0( void ) __attribute__ ( ( signal ) );  
void SIG_OVERFLOW0( void ) {
	static int8_t swipe_dir = 1;
	
	TCNT0 = 256-128;
	
	static uint8_t ic = 0;
	
	if (--scroll_skip == 0) {

		switch (mode) {
		case START: {
			scroll_skip = swipe_dir == 1 ? 1 : 4;
			start_column = start_column + swipe_dir;
			if (start_column == 9) swipe_dir = -1;
			if (start_column == 0) {
				mode = NORMAL;
				msgidx = 0;
				swipe_dir = 1;
				scroll_skip = scroll_pace;
				char1 = char2 = '-';
			}
			break;
		}
		case NORMAL: {		
#ifdef SPI_MASTER
			scroll_skip = scroll_pace;
			spi_count = 0;
			big_push();
			//msgidx++;
#else 
#endif
/*
			uint8_t c = pgm_read_byte(&message[msgidx]);
			if (c != 0) {
				char1 = c;
				char2 = pgm_read_byte(&message[msgidx+1]);
				msgidx++;
			}
			else {
				char1 = ic;
				char2 = ic + 1;
				ic++;
				
				if (ic % 32 == 0) {
					mode = START;
				}
			}
			break;
*/		
		}
		}
	}
}

void SIG_SPI( void ) __attribute__ ( ( signal ) );  
void SIG_SPI( void ) {	
#ifdef SPI_MASTER
	big_push();
#else
	uint8_t tmp = char2;
	char1 = char2;
	char2 = SPDR;
	SPDR = tmp;
#endif	
}

// $Id$
