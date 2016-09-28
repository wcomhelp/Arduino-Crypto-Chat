/* Chat: Two Arduinos are connected
through the Serial3 port.
Whanever one types on one of them,
the message shows up on the screen of the other.
*/
#include "Arduino.h"

uint16_t Generate_private_key() {
	uint16_t r = 0;
	for (int i = 0; i < 16; ++i) { 
		int bit = analogRead(1); // analogreads from pin 1 and stores it to bit.
		bit = bit & 1; // Reduces the int bit to just the least significant bit.
		r = r * 2 + bit;
		delay(50); // Delays 50ms so the voltage on the pin can fluctuate.
	}
	return r;
}

uint32_t get_input() {
	int buffer[11]; // Buffer for holding the current interger read from Serial.read()
	int counter = 0; // A counter for which slot to put the buffer.
	while (buffer[counter] != -35) {  // Continues running until user has pressed a enter (return).
		while (Serial.available() > 0) {
			buffer[counter] = Serial.read() - 48;
			if (buffer[counter] == -35) {
				continue; // Break if user has pressed enter (return).
			}
			else {
				Serial.print(buffer[counter]);
				counter = counter + 1;
			}
		}
	}
	unsigned long adder = 0;
	unsigned long ul = 0;
	for (int i = 0; i < counter; ++i) {
		adder = buffer[i];
		int j = counter - 1;
		j = j - i;
		for (j; j > 0; --j) {
			adder = adder * 10;
		}
		ul = ul + adder;
	}
	return ul;
}

/* Calculates (a**b) mod m
a: base 0<=a<2^32, 32 bits long
b: exponent 0<=, 32 bits long
m: modulus, 16 bits long (0<m<2^16)
Running time: O(1) [constant running]
*/
uint32_t fast_pow_mod(uint32_t a, uint32_t b, uint32_t m) {
	uint32_t result = 1 % m; // will store the result at the end
	uint32_t p = a % m; // will store a^{2^i} % m for i=0,1,...
	for (int i = 0; i<32; ++i) {
		if ((b & (1ul << i)) != 0) { // check i-th bit of b
			result = (result*p) % m;
		}
		p = (p*p) % m;
	}
	return result;
}


char Setup() { // Setup function
	init();
	Serial.begin(9600);
	Serial3.begin(9600);

	uint32_t prime = 19211;
	uint32_t generator = 6;
	uint32_t private_key = Generate_private_key();
	uint32_t public_key = fast_pow_mod(generator, private_key, prime);
	// key exchange
	Serial.print("Hey this is the key you were looking for: ");
	Serial.println(public_key);
	Serial.print("Can you please enter the key you received from your partner: ");
	uint32_t partners_public_key = get_input();
	uint32_t shared_secret = fast_pow_mod(partners_public_key, private_key, prime); // Computes shared_secret
	char secret_key = shared_secret; // lowest 8 bits of the shared_secret to secret_key
	return secret_key;
}


int main() {
	char secret_key = Setup();

	while (true) {
		// PC keystroke -> other Arduino
		if (Serial.available()>0) {
			char c = Serial.read();
			Serial.println((int)c);
			c = c ^ secret_key; // Encrypt data
			Serial3.write(c); // sending byte to the "other" Arduino
		}
		// Other Arduino to PC screen
		if (Serial3.available()>0) {
			char c = Serial3.read();
			c = c ^ secret_key; // Decrypt data 
			Serial.write(c); // Show byte on screen as character
		}
	}
	Serial3.end();
	Serial.end();
	return 0;
}