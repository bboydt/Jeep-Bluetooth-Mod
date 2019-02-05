// Jeep.ino
// Benjamin Thomas
// boydbt48@gmail.com
// Jan 2019
// A script that allows an Arduino to become a fake CD changer for a 2004 Jeep Grand Cherokee Radio.
// https://github.com/TildeOrange/Jeep-Bluetooth-Mod


// #define DEBUG_OUTPUT // Uncomment if you want debug info (might be slower)

#define PIN_IN 6
#define PIN_OUT 7
#define ERROR_TIMEDOUT -1
#define ERROR_INVALID_CRC -2

#define BUFF_LEN 16
byte buff[BUFF_LEN];

unsigned long last_sent = 0;

void setup()
{
#ifdef DEBUG_OUTPUT
	Serial.begin(9600);
#endif // DEBUG_OUTPUT

	pinMode(PIN_IN, INPUT);
	pinMode(PIN_OUT, OUTPUT);
	digitalWrite(PIN_OUT, LOW);
}

// A simple loop that checks for a poll frame from the radio and responds as if it was a CD changer. This opens up the audio input.
void loop()
{
	digitalWrite(PIN_OUT, LOW);   // just to be safe

	int bytes_read = read_frame(PIN_IN, buff, BUFF_LEN, 2000); // read from PIN_IN into the buff and timeout after 2 seconds

	if (0 < bytes_read && bytes_read <= 12)   // TODO figure out max length of frame
	{

#ifdef DEBUG_OUTPUT
        // print incoming frame (without check sum)
		Serial.print("Incoming frame: [ ");
		for (int i = 0; i < bytes_read - 1; i++)
		{
			Serial.print("0x" + String(buff[i], HEX) + " ");
		}
		Serial.println("]");
#endif // DEBUG_OUTPUT


		unsigned long current_time = millis();

		// If received frame is a poll from the radio or we haven't sent anything for a bit, then send a response frame.
		if (buff[0] == 0x8D || current_time - last_sent >= 1000)
		{
			int len = 5;
			byte buff[len] = { 0x8D, 0x94, 0x4, 0x14 };
			buff[len - 1] = CRC8(buff, len - 1);


#ifdef DEBUG_OUTPUT
            // print outgoing frame (with CRC)
			Serial.print("Outgoing frame: [ ");
			for (int i = 0; i < len; i++)
			{
				Serial.print("0x" + String(buff[i], HEX) + " ");
			}
			Serial.println("]");
#endif // DEBUG_OUTPUT

            // TODO wait for a quiet moment

			send_frame(PIN_OUT, buff, len);

			last_sent = current_time;
		}
	}
	else
	{
		switch (bytes_read)
		{
		case ERROR_TIMEDOUT:
			Serial.println("Timed out");
			break;
		case ERROR_INVALID_CRC:
			Serial.print("Read frame with an invalid CRC: [");
			for (int i = 0; i < 4; i++)
			{
				Serial.print("0x" + String(buff[i], HEX) + " ");
			}
			Serial.println("]");
			break;
		default:
			Serial.println("Read too many/few bytes" + String(bytes_read));
			break;
		}
	}
	//*/
}

// Disclaimer:
// This is a copy of a function written by Michael Wolf for the AVR J1850 VPW Interface he wrote. (https://www.mictronics.de/projects/j1850-vpw-interface/)
// I do not understand this so best of luck to you if it breaks.
//
// Creates an 8 bit cyclical redundancy check (CRC) from a bunch of bytes.
// The CRC can be used to check the validaty of those bytes durring transmission and is used to verify received data.
//
// byte *buff : data used to create CRC
// int len    : length of data
byte CRC8(byte *buff, int len)
{
	byte crc_reg = 0xff;
	byte poly;
	byte byte_count;
	byte bit_count;
	byte *byte_point;
	byte bit_point;

	for (byte_count = 0, byte_point = buff; byte_count < len; ++byte_count, ++byte_point)
	{
		for (bit_count = 0, bit_point = 0x80; bit_count < 8; ++bit_count, bit_point >>= 1)
		{
			if (bit_point & *byte_point)         // case for new bit = 1
			{
				if (crc_reg & 0x80)
				{
					poly = 1;             // define the polynomial
				}
				else
				{
					poly = 0x1c;
				}

				crc_reg = ((crc_reg << 1) | 1) ^ poly;
			}
			else         // case for new bit = 0
			{
				poly = 0;
				if (crc_reg & 0x80)
				{
					poly = 0x1d;
				}
				crc_reg = (crc_reg << 1) ^ poly;
			}
		}
	}
	return ~crc_reg;
}

// TODO remove this complicated mess
//
// Measures how long, in microseconds, a voltage was held.
// Blocks until the pin's voltage changes or the timeout is reached.
// Returns the time in microseconds that the voltage was constant.
//
// int pin                  : the pin to watch
// int initial_voltage      : what the voltage currenly is
// unsigned long start_time : when the voltage first became the initial voltage in microseconds (set to 0 if you aren't too concerned about accruacy)
// unsigned long timeout    : how many microseconds is too long to wait (warning: using this will add some overhead and decrease accruacy)
unsigned long get_signal_length(int pin, int initial_voltage, unsigned long start_time, unsigned long timeout)
{
	unsigned long start = start_time != 0 ? start_time : micros();

	if (timeout > 0)
	{
		unsigned long elapsed_time;

		// wait for voltage to change
		while (digitalRead(pin) == initial_voltage && elapsed_time < timeout)
		{
			elapsed_time = micros() - start;
		}

		return elapsed_time;
	}
	else
	{
		while (digitalRead(pin) == initial_voltage) {}
		return micros() - start;
	}
}

// Bit Symbols Cheat Sheet
// High for 64uS  = 1
// Low  for 128uS = 1
// High for 128uS = 0
// Low  for 64uS  = 0

// Reads a frame into a byte buffer.
// Blocks until a response was found or the timeout is reached.
// Returns the number of bytes read or ERROR_TIMEDOUT if the timeout was reached.
//
// int pin               : the pin to watch
// byte *buff            : the buffer to write into
// int len               : the length of the buffer
// unsigned long timeout : how many milliseconds is too long to wait
int read_frame(int pin, byte *buff, int len, unsigned long timeout)
{
	unsigned long start_time = millis();
	bool found_EOD = false;
	unsigned long signal_length;
	unsigned long signal_end = 0L;
	int current_voltage = LOW;   // we expect this to be low after the SOF signal
	String bits_str = "";

	// look for SOF signal (high for 200uS)
	while (signal_end == 0L)
	{
		while (digitalRead(pin) != HIGH)
		{
			if (millis() - start_time >= timeout)
			{
				return ERROR_TIMEDOUT;
			}
		}

		signal_length = get_signal_length(pin, HIGH, 0, timeout);

		if (abs(signal_length - 200) <= 15)     // if the signal length was 200uS +/- 15uS, then this is a valid signal
		{
			signal_end = micros();
			break;
		}

		if (millis() - start_time >= timeout)
		{
			return ERROR_TIMEDOUT;
		}
	}

	while (!found_EOD)
	{
		current_voltage = digitalRead(PIN_IN);
		signal_length = get_signal_length(pin, current_voltage, signal_end, 300);
		signal_end = micros();

		if (signal_length <= 96)
		{
			if (current_voltage == HIGH)
			{
				bits_str += '1';
			}
			else
			{
				bits_str += '0';
			}
		}
		else if (signal_length <= 164)
		{
			if (current_voltage == LOW)
			{
				bits_str += '1';
			}
			else
			{
				bits_str += '0';
			}
		}
		else if (current_voltage == LOW && signal_length > 164)
		{
			found_EOD = true;
		}
	}

	// parse data
	size_t bit_count = bits_str.length();
	size_t byte_count = bit_count / 8;

	for (int p = 0; p < byte_count; p++)
	{
		for (int b = p * 8; b < (p + 1) * 8; b++)
		{
			buff[p] <<= 1;
			if (bits_str[b] == '1')
			{
				buff[p]++;
			}
		}
	}

	if (buff[byte_count - 1] != CRC8(buff, byte_count - 1))
	{
		return ERROR_INVALID_CRC;
	}

	return (int)byte_count;
}

// TODO avoid talking over other accessories
// Sends a frame.
// Make the last byte in data is a CRC for the data.
//
// int pin    : the pin to write to
// byte *buff : the buffer to write from
// int len    : the length of the buffer
void send_frame(int pin, byte *data, int len)
{
	int current_voltage = LOW;

	// parse data (we do that now to speed up sending the data)
	byte t[len * 8];   // delays
	for (int p = 0; p < len; p++)
	{
		for (int b = 0; b < 8; b++)
		{
			if (((data[p] >> 7 - b) & 1) == 0)         // if bit is 0
			{
				t[p * 8 + b] = (current_voltage == LOW) ? 64 : 128;
			}
			else
			{
				t[p * 8 + b] = (current_voltage == HIGH) ? 64 : 128;
			}
			current_voltage = (current_voltage == LOW) ? HIGH : LOW;
		}
	}

	current_voltage = LOW;

	// send SOF
	digitalWrite(pin, HIGH);
	delayMicroseconds(200);
	digitalWrite(pin, LOW);

	// send data
	for (int b = 0; b < len * 8; b++)
	{
		current_voltage = (current_voltage == LOW) ? HIGH : LOW;
		delayMicroseconds(t[b]);
		digitalWrite(pin, current_voltage);
	}

	// send EOF
	digitalWrite(pin, LOW);
	delayMicroseconds(280);
}
