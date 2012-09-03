#include <io.h>
#include <signal.h>


#define LOBIT  BIT4
#define HIBIT  BIT5
#define BUBIT  BIT3

#define REDBIT BIT6
#define YELBIT BIT7

#define MIN_DOT 30
#define RELEASE_TIME 10

#define DOT  50
#define DASH DOT*3


uint16_t on_time;
uint16_t off_time;
uint8_t  running;

volatile uint8_t  mode_auto = 1;

volatile uint16_t dot = DOT;
volatile uint16_t dash = DASH;

volatile uint8_t  adc_done;
volatile uint16_t adc_value;

uint8_t  button_state;

volatile uint16_t next_taccr1;

void adc_read(uint16_t);

int main(void)
{
    int temp;

    // Stop watchdog
    WDTCTL = WDTPW + WDTHOLD;

    // Set clock to many MHz
    DCOCTL= 0;
    BCSCTL1= CALBC1_16MHZ;
    DCOCTL= 0xff; //CALDCO_16MHZ; //CALDCO_1MHZ;

    // SMCLK = X MHz / 8 = ? KHz (SLAU144E p.5-15)
    BCSCTL2 |= DIVS_3;

    // PWM period
    TACCR0 = 3625;

    // Source Timer A from SMCLK (TASSEL_2), up mode (MC_1).
    // Up mode counts up to TACCR0. SLAU144E p.12-20
    TACTL = TASSEL_2 | MC_1;

    // OUTMOD_7 = Reset/set output when the timer counts to TACCR1/TACCR0
    // CCIE = Interrupt when timer counts to TACCR1
    TACCTL1 = OUTMOD_7 | CCIE;
    //TACCTL1 = CCIE;

    TACCR1 = next_taccr1 = TACCR0/2;// 50% duty cycle 

    P1SEL = REDBIT;                 // timer/compare output
    P1DIR = REDBIT | YELBIT;        // les leds
    P1OUT = LOBIT | HIBIT | BUBIT;  // enable pullups on inputs
    P1REN = LOBIT | HIBIT | BUBIT;  // pullup enable


    button_state = BUBIT;

    adc_read(INCH_1);

    // LPM0 (shut down the CPU) with interrupts enabled
    for(;;) {
        __bis_SR_register(CPUOFF | GIE);

        if (adc_done) {
            adc_done = 0;

            // tone control
            if (!mode_auto) {
                temp = TACCR0/2 - adc_value*2;
                next_taccr1 = (temp < 1) ? 1 : temp;
            }
            else {
                dot = adc_value/32 + 30;
                dash = dot*3;
            }
            adc_read(INCH_1);
        }
    }

    return 0;
}

void key_auto() 
{
    if (!running) {
        if ((P1IN & LOBIT) == 0) {
            on_time = dot;
            running = 1;
        }
        else if ((P1IN & HIBIT) == 0) {
            on_time = dash;
            running = 1;
        }
    }

    if (on_time) {
        TACCTL1 |= OUTMOD_7;
        if (--on_time == 0) {
            off_time = dot;
        }
    } 
    else {
        TACCTL1 &= ~OUTMOD_7;
    }

    if (off_time) {
        if (--off_time == 0) {
            running = 0;
        }
    }
}

void key_manual()
{
    if (off_time > 0) --off_time;

    if ((P1IN & LOBIT) && (P1IN & HIBIT)) {
        if (on_time > 0) --on_time;
        if (on_time == 0) {
            off_time = MIN_DOT;
            TACCTL1 &= ~OUTMOD_7;
        }
    }
    else {
        if (off_time == 0) {
            TACCTL1 |= OUTMOD_7;
            if (on_time > RELEASE_TIME) --on_time;
            if (on_time == 0) {
                on_time = MIN_DOT;
            }
        }
    }
}

void adc_read(uint16_t chan)
{
    ADC10CTL0 &= ~ENC;                          // Disable ADC
    ADC10CTL0 = ADC10SHT_3 + ADC10ON + ADC10IE; // 16 clock ticks, ADC On, enable ADC interrupt
    ADC10CTL1 = ADC10SSEL_3 + chan;             // Set 'chan', SMCLK
    ADC10CTL0 |= ENC + ADC10SC;                 // Enable and start conversion
}

void button_debounce()
{
    static uint8_t debounce = 0;

    // check mode button
    if ((P1IN & BUBIT) != button_state && debounce == 0) {
        debounce = 100; 
        button_state = (P1IN & BUBIT);
        if (button_state == 0) {
            mode_auto = !mode_auto;
        }
    }
    if (debounce > 0) --debounce;
}

interrupt(ADC10_VECTOR) adc10_isr(void)
{
    adc_value = ADC10MEM;
    adc_done = 1;
    __bic_SR_register_on_exit(CPUOFF);
}

interrupt(TIMERA1_VECTOR) ta1_isr(void)
{
    // Clear interrupt flag
    TACCTL1 &= ~CCIFG;

    button_debounce();
    if (mode_auto) {
        P1OUT |= YELBIT;
        key_auto();
    } 
    else {
        P1OUT &= ~YELBIT;

        key_manual();
    }

    while(TAR <= next_taccr1);
    TACCR1 = next_taccr1;
}


