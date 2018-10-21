/*
 * Based on: Miroslaw Kardas (Mirekk36) library  and G4lvanix git
 * Created by: HorochovPL
 */

#include <avr/io.h>
#include <util/twi.h>	//definitions of i2c states
#include <stdint.h>

#include "i2c.h"


/*
 * Typical call is i2c_init(100) for 100 KHz baud rate
 * Returns error value (ok=0)
 *
 */
err_t i2c_init(uint16_t bitrateX100K) {
	/*
	 * TODO: Maybe tinker with prescaler even if TWBR < 255
	 * Check if i2c isn't live
	 *
	 *
	 * ((F_CPU / SCL_FREQ)/2 - 8 ) /  PRESC = TWBR
	 * (F_CPU / SCL_FREQ)/2 < 8 means underflow. For value 8 TWBR is still 0.
	 */
	int16_t bitrate = F_CPU / 2000 / bitrateX100K;
	TWSR &= ~((1<<TWPS0)|(1<<TWPS1));	//disable prescaler
	if (bitrate < 9) {			//handle underflow
		TWBR = 0;
		return _i2c_slowerBitrateUnderflow;
	}

	bitrate-= 8;

	if (bitrate > 255) {			//handle overflow
		//Use the smallest prescaler to be more accurate.
		bitrate>>=2;	//div 4
		if(bitrate<255) {	//if presc 4 will do
			TWSR|= (1<<TWPS0);
			TWBR = bitrate;
			return _i2c_attachedPrescaler;
		}

		bitrate>>=2;	//div 16
		if (bitrate < 255) {	//if presc 16 will do
			TWSR |= (1 << TWPS1);
			TWBR = bitrate;
			return _i2c_attachedPrescaler;
		}

		bitrate >>= 2;	//div 64
		TWSR |= (1 << TWPS1)|(1<<TWPS0);
		if (bitrate < 255) {	//if presc 64 will do..
			TWBR = bitrate;
			return _i2c_attachedPrescaler;
		}

		TWBR=255;					//..or not.
		return _i2c_slowerBitrateOverflow;	//Is it even possible? :D
	}

	TWBR=bitrate;
	return _i2c_ok;

}


err_t i2c_start(uint8_t devAddr) {
	TWCR = 0;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	while (!(TWCR&(1<<TWINT)));
	if ((TWSR & 0xF8) != TW_START) return _i2c_badStart;

	// start transmission of devaddr
	TWDR = devAddr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ((twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK)) return _i2c_slaveAddrNotAcking;

	return _i2c_ok;
}


void i2c_stop(void) {
	if(!(TWCR&(1<<TWSTA))) 	return;	//do not perform double-stop
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	while ( (TWCR&(1<<TWSTO)));
//	if ((TWSR & 0xF8) != TW_SR_STOP) return _i2c_badStop;		//haven't tested it yet
}

err_t i2c_write(uint8_t bajt) {
	TWDR = bajt;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ( !(TWCR&(1<<TWINT)));

	if ((TWSR & 0xF8) != TW_MT_DATA_ACK) return _i2c_slaveDataNotAcking;
	return _i2c_ok;

}

/*
 * Check for ((TWSR & 0xF8) != TW_MR_DATA_ACK)
 * But the last one should be different, TW_MR_DATA_NACK
 */
uint8_t i2c_read(uint8_t ack) {
	TWCR = (1<<TWINT)|(ack<<TWEA)|(1<<TWEN);
	while ( !(TWCR & (1<<TWINT)));
	return TWDR;
}

err_t i2c_write_buf(uint8_t devAddr, uint8_t adr, uint16_t len, uint8_t *buf) {
	uint8_t err;
	i2c_stop();	//just in case
	//If operation failed will not continue
	if ((err = i2c_start(devAddr)) == _i2c_ok)
		if ((err = i2c_write(adr))	== _i2c_ok)
			while (len--)
				if ((err = i2c_write(*buf++)) != _i2c_ok) break;
	//But still will close bus
	i2c_stop();
	return err; //ret error, otherwise ok
}

err_t i2c_write_single(uint8_t devAddr, uint8_t adr, uint8_t data) {
	uint8_t err;
	i2c_stop();	//just in case
	if ((err = i2c_start(devAddr)) == _i2c_ok)
		if ((err = i2c_write(adr)) == _i2c_ok)
			err = i2c_write(data);
	i2c_stop();
	return err;
}

err_t i2c_read_buf(uint8_t devAddr, uint8_t adr, uint16_t len, uint8_t *buf) {
	uint8_t err;
	i2c_stop();	//just in case
	if ((err = i2c_start(devAddr)) == _i2c_ok)
		if ((err = i2c_write(adr)) == _i2c_ok)
			if ((err = i2c_start(devAddr + 1)) == _i2c_ok)
				while (len--) {
					*buf++ = i2c_read(len ? ACK : NACK);
					if (len)
						if ((TWSR & 0xF8) != TW_MR_DATA_ACK) {	//Externally check for error
							err = _i2c_slaveDataNotAcking;
							break;
						}
				}
	i2c_stop();
	return err;
}
