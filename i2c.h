#ifndef I2C_i2c_H_
#define I2C_i2c_H_
/*
 * Based on: Miroslaw Kardas (Mirekk36) library and G4lvanix git
 * Created by: HorochovPL
 *
 *
 *	Linear "blocking" approach to I2C bus
 */

#define ACK 1
#define NACK 0

#ifndef _err_t
		typedef uint8_t err_t;
	#define _err_t
#endif


err_t i2c_init(uint16_t bitrateX100K);
err_t i2c_start(uint8_t devAddr);
void i2c_stop(void);
err_t i2c_write(uint8_t bajt);
uint8_t i2c_read(uint8_t ack);		//returns data, not i2cErrorStatus!

err_t i2c_write_buf( uint8_t devAddr, uint8_t adr, uint16_t len, uint8_t *buf );
err_t i2c_write_single( uint8_t devAddr, uint8_t adr, uint8_t data );
err_t i2c_read_buf(uint8_t devAddr, uint8_t adr, uint16_t len, uint8_t *buf);


enum i2cErrorStatus{
	_i2c_ok,
	_i2c_attachedPrescaler,			//it ain't error. Feel free to add/delete "=0" here.
	_i2c_slowerBitrateUnderflow,	//everything will work, but slower.
	_i2c_slowerBitrateOverflow,		//everything will work, but slower.
	_i2c_slaveAddrNotAcking,
	_i2c_slaveDataNotAcking,
	_i2c_badStart,
	_i2c_timeout

};

/*
 * Ready to use code
 * Just copy in right spot
 */

////Print all i2c devices
//	for (uint8_t i = 0; i < 128; ++i) {
//		if(!i2c_start(i<<1)) {
//			uart_stringFlash(PSTR("\r\nDev found at: 0x"));
//			uart_number(i<<1,16);
//		}
//		i2c_stop();
//	}



#endif /* I2C_i2c_H_ */
