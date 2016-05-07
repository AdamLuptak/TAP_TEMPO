#include <Arduino.h>
#include <avr/wdt.h>

#define OUT_PIN 3
#define INPUT_PIN 2
#define OUTPUT_LED 13
#define INTERRUPT_NUMBER 0

const int debounceDelay = 150;
volatile int tapCount = 0;
const int tempoCountConst = 4;
float cas = 15.624;
int counterForDelay = 0;
volatile int turnOnDelayInterval = 600;
volatile boolean turnOnSequence = false;
volatile int sequenceCounter = 0;
long tempos[10];
long timeNow = 0;
long timeForPause = 5000;
volatile long timePre = 0;

void delayBetweenTapsProtection();

void setupTimer();

void setupInterruptRoutin();

void setupWatchDog();

void setupPins();

void setupSerial();

void setup() {

    setupSerial();

    setupPins();

    setupInterruptRoutin();

    setupTimer();

    setupWatchDog();

    Serial.println("Can't get here");


}

void setupSerial() { Serial.begin(9600); }

void setupPins() {
    pinMode(OUTPUT_LED, OUTPUT);
    pinMode(OUT_PIN, OUTPUT);
    pinMode(INPUT_PIN, INPUT);
}

void setupWatchDog() { wdt_enable(WDTO_1S); }

void setupInterruptRoutin() { attachInterrupt(INTERRUPT_NUMBER, tapDetect, HIGH); }

void setupTimer() {
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1 = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = cas;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {//timer1 interrupt 1Hz toggles pin 13 (LED)/*
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
//compare match register = [ 16,000,000Hz/ (prescaler * desired interrupt frequency) ] - 1
//compare match register = [16,000,000 / (1024 * 1) ] -1
//= 15,624 pre  1ms, 14,625
    if (turnOnSequence) {
        if (sequenceCounter == 6) {
            sequenceCounter = 0;
            turnOnSequence = false;
        } else {
            (counterForDelay < turnOnDelayInterval / 2) ? (PORTD = 0b0001000) : (PORTD = 0b0000000);
            (counterForDelay < turnOnDelayInterval / 2) ? (PORTB = 0b0100000) : (PORTB = 0b0000000);
            counterForDelay = counterForDelay + 1;
            if (counterForDelay == turnOnDelayInterval) {
                counterForDelay = 0;
                sequenceCounter++;
            }
        }
    }
}

int calculateInterval() {
    int countTempos = 0;
    for (int i = 0; i < tempoCountConst / 2; ++i) {
        countTempos += tempos[i + 1] - tempos[i];
    }
    int avarage = countTempos / 2; //1/4 notes
    avarage = (avarage / 4) * 3;
    return avarage;
}

void tapDetect() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > debounceDelay) {
        if (tapCount < tempoCountConst) {
            timePre = interrupt_time;
            tempos[tapCount] = interrupt_time;
            tapCount++;
        }
        if (tapCount == tempoCountConst) {
            tapCount = 0;
            turnOnDelayInterval = calculateInterval();
            turnOnSequence = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

void loop() {
    delayBetweenTapsProtection();
    //Protection
    wdt_reset();
}

void delayBetweenTapsProtection() {
    timeNow = millis();
    if (tapCount != 0) {
        if (timeNow - timePre > timeForPause) {
            tapCount = 0;
            timePre = timeNow;
        }
    }
}

