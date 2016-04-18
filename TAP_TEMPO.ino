#include <Arduino.h>

#define OUTPIN 3
#define INPUTPIN 2
#define OUTPUT_LED 13
#define INTERRUPT_NUMBER 0

const int debounceDelay = 200;
volatile int tapCount = 0;

// pre timer
float cas =  15.624; //
int pocitadlo = 0;
int interval_zap = 400;
volatile int zapni =600;
//

long tempos[10];


void setup() {

    Serial.begin(9600);

    pinMode(OUTPUT_LED, OUTPUT);
    pinMode(INPUTPIN, INPUT);

    attachInterrupt(INTERRUPT_NUMBER, tapDetect, HIGH);


    cli();
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = cas;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    sei();
}




ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)/*
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
//compare match register = [ 16,000,000Hz/ (prescaler * desired interrupt frequency) ] - 1
//compare match register = [16,000,000 / (1024 * 1) ] -1
//= 15,624 pre  1ms, 14,625
    cli();
     (pocitadlo < zapni/2) ? (PORTB = 0b0100000) : (PORTB = 0b0000000);
      pocitadlo = pocitadlo + 1;
      pocitadlo = (pocitadlo == zapni) ? 0 : pocitadlo;

    sei();
}


void tapDetect() {

    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > debounceDelay)
    {
        if(tapCount < 4){
            tempos[tapCount] = interrupt_time;
            tapCount++;
        }
        if(tapCount == 4){
            tapCount = 0;
        }

    }
    last_interrupt_time = interrupt_time;
}


void loop() {
    for (int i = 0; i < 4; ++i) {
        Serial.print(tempos[i]);
        Serial.print(", ");
    }
    Serial.println("");
}

