#include <inttypes.h>
#include <string.h>

#include "msp430fr5739.h"
#include "Serial.h"
#include "FraunchLeds.h"
#include "Buttons.h"
#include "I2CMaster.h"
#include "OpenServo.h"

// These golabal variables are used in the ISRs 


void systemInit(void)
{
    // Startup clock system in max. DCO setting ~8MHz
    // This value is closer to 10MHz on untrimmed parts  
    CSCTL0_H = 0xA5;                          // Unlock register
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = vlo; MCLK = DCO
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers 
    CSCTL0_H = 0x01;                          // Lock Register

    // Turn off temp. 
    REFCTL0 |= REFTCOFF; 
    REFCTL0 &= ~REFON;  

    // Terminate Unused GPIOs
    // P1.0 - P1.6 is unused
    P1OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);   
    P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7); 
    P1REN |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);   

    // P1.4 is used as input from NTC voltage divider
    // Set it to output low
    P1OUT &= ~BIT4;      
    P1DIR |= BIT4; 

    // P2.2 - P2.6 is unused
    P2OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);    
    P2DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6); 
    P2REN |= (BIT2 + BIT3 + BIT4 + BIT5 + BIT6);   

    // P2.7 is used to power the voltage divider for the NTC thermistor
    P2OUT &= ~BIT7;     
    P2DIR |= BIT7; 

    // P3.0,P3.1 and P3.2 are accelerometer inputs
    P3OUT &= ~(BIT0 + BIT1 + BIT2);   
    P3DIR &= ~(BIT0 + BIT1 + BIT2); 
    P3REN |= BIT0 + BIT1 + BIT2;

    // PJ.0,1,2,3 are used as LEDs
    // crystal pins for XT1 are unused
    PJOUT &= ~(BIT4+BIT5);  
    PJDIR &= ~(BIT4+BIT5);
    PJREN |= BIT4 + BIT5;  
}

void LongDelay(void)
{
  __delay_cycles(250000);
  __no_operation();
}

void startUpSequence() {
	for (int i = 0; i < 8; i++) {
		blueleds.Set(i, 1);
		LongDelay();
		LongDelay();
		LongDelay();
	}
	for (int i = 0; i < 8; i++) {
		blueleds.Set(i, 0);
		LongDelay();
		LongDelay();
		LongDelay();
	}
}

 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

 /* itoa:  convert n to characters in s */
 void itoa(int _n, char s[])
 {
     int i, sign;
     int32_t n = _n;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}


int atoi(char* s, int* result) 
{
    int pos;
    *result = 0;
    
    reverse(s);
    for(pos = 1; *s != 0; pos*=10, s++) {
        switch (*s) {
        case '-': 
            *result = -*result;
            return 0;
        case '+':
            return 0;
        default:
            if (*s >= '0' && *s <= '9') {
                *result += (*s - '0') * pos;
                continue;
            } else {
                return -1;
            }
        }
    }
    return 0;
}


void printInt(int x) {
    char buff[8];
    itoa(x, buff);
    com.puts(buff);
}


OpenServo os(0x10);

void probeOS()
{
    uint8_t maj, min;
    
    //OpenServo os(0x10);
    
    i2c.Init();

    if (os.GetVersion(&maj, &min) != 0) {    
        com.puts("\nCould not read version\n");
        return;
    }
    com.puts("\nOpenServo version:");
    printInt(maj);
    com.putchar('.');
    printInt(min);
    com.puts("\n");
}

uint16_t probeany(int w, OSRegister reg, const char* name, char nl)
{
    uint16_t data;

    i2c.Init();
    
    com.putchar('\n');
    
    if ((w ? os.Read16(reg, &data) : os.Read8(reg, reinterpret_cast<uint8_t*>(&data))) != 0) {
        com.puts("Could not read "); 
        com.puts(name);
        com.putchar('\n');
        return 0177777;
    }
    com.puts(name);
    com.puts(": ");
    printInt(w ? data : (data & 0377));
    
    com.putchar(nl);
    
    return data;
}


uint16_t probe16(OSRegister reg, const char* name, char nl)
{
    return probeany(1, reg, name, nl);
}

uint16_t probe8(OSRegister reg, const char* name, char nl)
{
    return probeany(0, reg, name, nl);
}


uint16_t write16(OSRegister reg, uint16_t value)
{
    uint16_t data;
    
    //OpenServo os(0x10);
    
    i2c.Init();
    os.Write16(reg, value);
    
    return 0;
}

void oscommand(OSCommand c)
{
    i2c.Init();
    os.Command(c);
}


uint16_t probePos() 
{
    probe16(POSITION_HI, "position", '\n');
}

volatile int gruu;

void ndelay() 
{
    for (gruu = 0; gruu < 255; gruu++);
}

void load_pi(int p, int i) 
{
    oscommand(WRITE_ENABLE);
    ndelay();
    write16(PID_PGAIN_HI, p);
    ndelay();
    write16(PID_DGAIN_HI, i);
    ndelay();
    oscommand(WRITE_DISABLE);
}

void setGainFromStr(OSRegister reg, char *cmd) 
{
    int value;
    
    if (atoi(cmd, &value) == 0) {
        oscommand(WRITE_ENABLE);
        ndelay();
        write16(reg, value);
        ndelay();
        oscommand(WRITE_DISABLE);
        ndelay();
    }
}

void setReg8FromStr(OSRegister reg, char *cmd) 
{
    int value;
    
    if (atoi(cmd, &value) == 0) {
        oscommand(WRITE_ENABLE);
        ndelay();

        i2c.Init();
        os.Write8(reg, (uint8_t)value);

        ndelay();
        oscommand(WRITE_DISABLE);
        ndelay();
    }
}



void printGains() 
{
    ndelay();
    probe16(PID_PGAIN_HI, "pid_p", ' ');
    probe16(PID_IGAIN_HI, "pid_i", ' ');
    probe16(PID_DGAIN_HI, "pid_d", '\n');
}


void parseCommand(char* cmd) 
{
    int state = 0;
    char *command = cmd;
    int query = 0;
    
    com.puts("\nparsingCommand: '");
    com.puts(cmd);
    com.puts("'\n");
    
    for(; state != -1; cmd++) {
        switch (state) {
        case 0:
            query = 0;            
            switch(*cmd) {
            case 0:
                state = -1;
                // make it a query by default
            case '?':
                query = 1;
                // fall through
            case '=':
                *cmd = 0;
                if (strcmp(command, "p") == 0) state = 10;
                else if (strcmp(command, "d") == 0) state = 20;
                else if (strcmp(command, "i") == 0) state = 100;
                else if (strncmp(command, "vel", 3) == 0) state = 30;
                else if (strncmp(command, "see", 3) == 0) state = 40;
                else if (strncmp(command, "pow", 3) == 0) state = 50;
                else if (strncmp(command, "pos", 3) == 0) state = 60;
                else if (strncmp(command, "pwm", 3) == 0) state = 70;
                else if (strcmp(command, "reverse") == 0) state = 80;
                else if (strcmp(command, "reset") == 0) state = 90;
                else if (strncmp(command, "version", 3) == 0) state = 110;
                else {
                    com.puts("unknown command");
                    state = -1;
                }
                if (state != -1 && query) state += 5;  
                break;
            default:
                break;
            }
            break;
        case 10:
            setGainFromStr(PID_PGAIN_HI, cmd);
        case 15:
            printGains();
            state = -1;
            break;
            
        case 20:
            setGainFromStr(PID_DGAIN_HI, cmd);
        case 25:
            printGains();
            state = -1;
            break;

        case 100:
            setGainFromStr(PID_IGAIN_HI, cmd);
        case 105:
            printGains();
            state = -1;
            break;

            
        case 30:
            setGainFromStr(SEEK_VELOCITY_HI, cmd);
            // intentional fall-through
        case 35:
            probe16(SEEK_VELOCITY_HI, "seek velocity", '\n');
            state = -1;
            break;
        
        case 40:
            setGainFromStr(SEEK_POSITION_HI, cmd);
        case 45:
            probe16(SEEK_POSITION_HI, "seek", '\n');
            state = 65; // query position too
            break;
            
        case 50:
        case 55:
            probe16(POWER_HI, "power", '\n');
            state = -1;
            break;
            
        case 60:
        case 65:
            probe16(POSITION_HI, "position", '\n');
            state = -1;
            break;

        case 70:
            setGainFromStr(PWM_FREQ_DIVIDER_HI, cmd);
        case 75:
            probe16(PWM_FREQ_DIVIDER_HI, "pwm divider", '\n');
            state = -1;
            break;

        case 80:
            setReg8FromStr(REVERSE_SEEK, cmd);
        case 85:
            probe8(REVERSE_SEEK, "reverse_seek", '\n');
            state = -1;
            break;

        case 90:
        case 95:
            oscommand(RESET);
            com.puts("Device reset\n");
            state = -1;
            break;
            
        case 110:
        case 115:
            probeOS();
            state = -1;
            break;

        default:
            com.puts("error\n");
            state = -1;
            break;
        }
    }
}

int main(void)
{
    uint16_t pgain = 0, igain = 0, dgain = 0;
    int input = 0;
    char buf[32];
    int bufidx = 0;
    int pwm = 0;
  
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  
    systemInit();                             // Init the Board
  
    startUpSequence();                        // Light up LEDs
  
    com.puts("Gruuuu....\n");
  
    while(1) {
        __bis_SR_register(LPM2_bits + GIE);
        __no_operation();			            // For debugger
    
        while (com.avail()) {
        	int c = com.getchar();
            switch (c) {
                case '\r':
                case '\n':  buf[bufidx] = 0;
                            bufidx = 0;
                            input = 1;
                            com.putchar(c);
                            break;
                case 010:
                case 127:
                            if (bufidx > 0) {
                                bufidx--;
                                com.putchar('\\');
                            }
                            break;
                case '`':   oscommand(pwm ? PWM_DISABLE : PWM_ENABLE);
                            pwm = !pwm;
                            break;
                default:    buf[bufidx++] = c;
                            com.putchar(c);
                            break;
            }
        }
                    
        if (input) {
            input = 0;
            parseCommand(buf);
        }
            
#if 0            
                case '1':   probeOS();
                            break;
                case '2':   probe16(POSITION_HI, "position");
                            break;
                case '3':   probe16(VELOCITY_HI, "velocity");
                            break;
                case '4':   probe16(SEEK_POSITION_HI, "seek");
                            break;
                case '5':   command(VOLTAGE_READ);
                            for (gruu = 0; gruu < 255; gruu++);
                            probe16(POWER_HI, "power");
                            break;
                case '6':   probe16(PWM_DIRA, "dira/b");
                            break;
                case '7':   probe16(SEEK_VELOCITY_HI, "seek velocity");
                            break;
                case '&':   write16(SEEK_VELOCITY_HI, 0x50);
                            break;
                case '8':   probe16(FLAGS_HI, "flags");
                            break;
                case '9':   
                schnugg:
                            ndelay();
                            pgain = probe16(PID_PGAIN_HI, "pid_p");
                            igain = probe16(PID_IGAIN_HI, "pid_i");
                            dgain = probe16(PID_DGAIN_HI, "pid_d");
                            break;
                case '0':   write16(SEEK_POSITION_HI, 300);
                            break;
                case '-':   write16(SEEK_POSITION_HI, 500);
                            break;
                case '=':   write16(SEEK_POSITION_HI, 700);
                            break;
                case 'q':   command(PWM_ENABLE);
                            break;
                case 'w':   command(PWM_DISABLE);
                            break;
                case 'x':   pgain+=1;
                            command(WRITE_ENABLE);
                            ndelay();
                            write16(PID_PGAIN_HI, pgain);
                            ndelay();
                            command(WRITE_DISABLE);
                            goto schnugg;    
                            break;
                case 'z':   pgain-=10;
                            command(WRITE_ENABLE);
                            ndelay();
                            write16(PID_PGAIN_HI, pgain);
                            ndelay();
                            command(WRITE_DISABLE);
                            goto schnugg;
                            break;
                case 'v':   igain+=10;
                            command(WRITE_ENABLE);
                            ndelay();
                            write16(PID_IGAIN_HI, igain);
                            ndelay();
                            command(WRITE_DISABLE);
                            goto schnugg;    
                            break;
                case 'c':   igain-=1;
                            command(WRITE_ENABLE);
                            ndelay();
                            write16(PID_IGAIN_HI, igain);
                            ndelay();
                            command(WRITE_DISABLE);
                            goto schnugg;
                            break;
                case 'n':   dgain+=10;
                            command(WRITE_ENABLE);
                            ndelay();
                            write16(PID_DGAIN_HI, dgain);
                            ndelay();
                            command(WRITE_DISABLE);
                            goto schnugg;    
                            break;
                case 'b':   dgain-=10;
                            command(WRITE_ENABLE);
                            ndelay();
                            write16(PID_DGAIN_HI, dgain);
                            ndelay();
                            command(WRITE_DISABLE);
                            goto schnugg;
                            break;
                case 'A':   com.puts("loading 0x600,0x0c00\n");
                            load_pi(0x600,0xc00);
                            goto schnugg;
                case 'S':   
                            load_pi(0x400,0xc00);
                            goto schnugg;
                case 'D':   
                            load_pi(0x200,0xc00);
                            goto schnugg;
                case 'F':   
                            load_pi(0x400,-3096);
                            goto schnugg;
                default:
                            com.puts("\nInvalid input\n");
                            break;
            }
        }
#endif    
        // Wake up from LPM because user has entered a mode
        switch(buttons.Selection())
        {
          default:
            // This is not a valid mode
            // Blink LED1 to indicate invalid entry
            // Switch S2 was pressed w/o mode select
            while(buttons.SelectionActive() == 0)
              {
                blueleds.Set(7, -1);
                LongDelay();
              }
    	    break;
        }
    }
  
    return 0;    
}
 

/**********************************************************************//**
 * @brief  Timer A0 ISR for MODE2, Slow FRAM writes, 40ms timer
 *************************************************************************/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{  
  __bic_SR_register_on_exit(LPM2_bits);
}



