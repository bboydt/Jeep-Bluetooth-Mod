#define F_CPU 8000000UL

#define SET_HIGH(pin) PORTB |= _BV(pin)
#define SET_LOW(pin) PORTB &= ~_BV(pin)
#define TOGGLE_PIN(pin) PORTB ^= _BV(pin)

#define IN_PIN  0
#define OUT_PIN 1
#define LED_PIN 2
#define RX_PIN 	3
#define TX_PIN 	4

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void SendFrame(const uint8_t *data, uint8_t len);

int main()
{
    // setup pins and enable interrupts
    DDRB = _BV(OUT_PIN) | _BV(LED_PIN) | _BV(TX_PIN);
    PORTB = 0;

    const uint8_t frameA_data[4] = { 0x55, 0x55, 0x55, 0x55 };
	const uint8_t frameB_data[4] = { 0x11, 0x00, 0x33, 0xff };

    while (1)
    {
        _delay_ms(500);
        SET_HIGH(LED_PIN);
        SendFrame(frameA_data, 4);
        _delay_us(500);
        SendFrame(frameB_data, 4);
        SET_LOW(LED_PIN);
    }
}


//
// J1850 Protocall stuffs
//

// Timing
#define PIN_CHANGE_DELAY 1
 // I'm not sure the cause of this, but there is a 12 (or 8?) us delay but only for the start of frame signal, so we account for that here
#define START_OF_FRAME_LEN 200 - PIN_CHANGE_DELAY - 12
#define END_OF_FRAME_LEN 200 - PIN_CHANGE_DELAY - 12
#define ACTIVE_LOGIC_1_LEN 64 - PIN_CHANGE_DELAY
#define ACTIVE_LOGIC_0_LEN 128 - PIN_CHANGE_DELAY
#define PASSIVE_LOGIC_1_LEN 128 - PIN_CHANGE_DELAY
#define PASSIVE_LOGIC_0_LEN 64 - PIN_CHANGE_DELAY

void SendFrame(const uint8_t *data, uint8_t len)
{
    SET_HIGH(OUT_PIN);
    _delay_us(200);
    uint8_t byte = 0;
    uint8_t bit = 0;
    for (byte = 0; byte < len; byte++)
    {
        for (bit = 0; bit < 8; bit++)
        {
            TOGGLE_PIN(OUT_PIN);
            if (bit % 2 == 0)
            {
                // passive logic
                _delay_us(PASSIVE_LOGIC_1_LEN);
            }
            else
            {
                // active logic
                _delay_us(ACTIVE_LOGIC_1_LEN);
            }
        }
    }
    SET_LOW(OUT_PIN);
    _delay_us(200);
}
