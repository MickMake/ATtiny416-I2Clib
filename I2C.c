//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I2C

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include "I2C.h"
#include "ups.h"


#define I2C_STATE_IS_HIGH(x) ((TWI0.MSTATUS & (x)) == (x))
#define I2C_STATE_IS_LOW(x) ((TWI0.MSTATUS & (x)) != (x))
#define I2C_BUS_STATE (TWI0.MSTATUS & TWI_BUSSTATE_gm)

#define I2C_BUS_NOT_IDLE I2C_STATE_IS_LOW(TWI_BUSSTATE_IDLE_gc)
#define I2C_BUS_IDLE I2C_STATE_IS_HIGH(TWI_BUSSTATE_IDLE_gc)

#define I2C_BUS_NOT_BUSY I2C_STATE_IS_LOW(TWI_BUSSTATE_BUSY_gc)
#define I2C_BUS_BUSY I2C_STATE_IS_HIGH(TWI_BUSSTATE_BUSY_gc)

#define I2C_BUS_NOT_OWNER I2C_STATE_IS_LOW(TWI_BUSSTATE_OWNER_gc)
#define I2C_BUS_OWNER I2C_STATE_IS_HIGH(TWI_BUSSTATE_OWNER_gc)

#define I2C_NOT_CLOCKHOLD I2C_STATE_IS_LOW(TWI_CLKHOLD_bm)
#define I2C_CLOCKHOLD I2C_STATE_IS_HIGH(TWI_CLKHOLD_bm)

#define I2C_NOT_BUSERR I2C_STATE_IS_LOW(TWI_BUSERR_bm)
#define I2C_BUSERR I2C_STATE_IS_HIGH(TWI_BUSERR_bm)

#define I2C_NOT_ARBLOST I2C_STATE_IS_LOW(TWI_ARBLOST_bm)
#define I2C_ARBLOST I2C_STATE_IS_HIGH(TWI_ARBLOST_bm)


uint8_t SetBaudRate(uint16_t baud) {

//((((float)F_CPU / (float)F_SCL)) - 10 )
	
}


void InitI2C()
{

	I2C_StartMaster();
}


bool I2C_StartMaster() {

	PORTB.DIRSET = I2C_SCL;

	TWI0.CTRLA = TWI_SDAHOLD_500NS_gc | TWI_SDASETUP_8CYC_gc; // Use max SDA setup and hold times to be safe
	//TWI0.MBAUD = (uint8_t)TWI0_BAUD(100000);	        // set MBAUD register for 100kHz
	TWI0.MBAUD = 35;
	//TWI0.MBAUD = 0x01;
	TWI0.MCTRLA = 1 << TWI_ENABLE_bp			/* Enable TWI Master: enabled */
	| 0 << TWI_QCEN_bp					/* Quick Command Enable: disabled */
	| 0 << TWI_RIEN_bp					/* Read Interrupt Enable: disabled */
	| 1 << TWI_SMEN_bp					/* Smart Mode Enable: enabled */
	| TWI_TIMEOUT_DISABLED_gc				/* Bus Timeout Disabled */
	| 0 << TWI_WIEN_bp;					/* Write Interrupt Enable: disabled */

	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc ;		        //Force TWI state machine into IDLE state
	TWI0.MSTATUS = (TWI_RIF_bm | TWI_WIF_bm) ;
	TWI0.MCTRLB = TWI_FLUSH_bm ;				/* Purge MADDR and MDATA */
	
	return(RETURN_OK);
}


bool I2C_StopMaster() {

	TWI0.MCTRLA &= ~(TWI_ENABLE_bm);
	PORTB.DIRSET &= ~(I2C_SCL);

	return(RETURN_OK);
}


bool I2C_StartSlave() {

/*
	TWI0.CTRLA = TWI_SDAHOLD_500NS_gc | TWI_SDASETUP_8CYC_gc; // Use max SDA setup and hold times to be safe
	//TWI0.MBAUD = (uint8_t)TWI0_BAUD(100000);	        // set MBAUD register for 100kHz
	TWI0.MBAUD = 35;
	//TWI0.MBAUD = 0x01;
	TWI0.MCTRLA = 0 << TWI_ENABLE_bp			/ Enable TWI Master: enabled /
	| 0 << TWI_QCEN_bp					/ Quick Command Enable: disabled /
	| 0 << TWI_RIEN_bp					/ Read Interrupt Enable: disabled /
	| 1 << TWI_SMEN_bp					/ Smart Mode Enable: enabled /
	| TWI_TIMEOUT_DISABLED_gc				/ Bus Timeout Disabled /
	| 0 << TWI_WIEN_bp;					/ Write Interrupt Enable: disabled /

	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc ;		        //Force TWI state machine into IDLE state
	TWI0.MSTATUS = (TWI_RIF_bm | TWI_WIF_bm) ;
	TWI0.MCTRLB = TWI_FLUSH_bm ;				/ Purge MADDR and MDATA /
*/

	return(RETURN_OK);
}


bool I2C_WriteByte(uint8_t address, uint8_t reg, uint8_t data) {

	if (I2C_BUS_NOT_BUSY) {
		if (I2C_RawStart(address, I2C_WRITE)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawWrite(reg)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawWrite(data)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		I2C_RawStop();
	}

	return(RETURN_OK);
}


bool I2C_WriteBytes(uint8_t address, uint8_t reg, uint8_t data[], uint8_t size) {
	if (I2C_BUS_NOT_BUSY) {
		uint8_t length = reg + size;
		for (; reg < length; reg++, data++) {
			if (I2C_WriteByte(address, reg, *data)) {
				return(RETURN_FAILED);
			}
		}
	}

	return(RETURN_OK);
}


bool I2C_ReadByte(uint8_t address, uint8_t reg, uint8_t *data) {

	if (I2C_BUS_NOT_BUSY) {
		if (I2C_RawStart(address, I2C_WRITE)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawWrite(reg)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawStart(address, I2C_READ)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawRead(0, data)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		I2C_RawStop();
	}

	return(RETURN_OK);
}


bool I2C_ReadBytes(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size) {

	if (I2C_BUS_NOT_BUSY) {
		if (I2C_RawStart(address, I2C_WRITE)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawWrite(reg)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if (I2C_RawStart(address, I2C_READ)) {
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		for (uint8_t index = 0x00; index < size; index++) {
			// For the last packet received we want to send a NACK to stop it from sending any more.
			// (index + 1 == size) ? 1 : 0
			if (I2C_RawRead((index + 1 == size) ? 0 : 1, data++)) {
				I2C_RawStop();
				return(RETURN_FAILED);
			}
		}

		I2C_RawStop();
	}

	return(RETURN_OK);
}

/*
bool I2C_ReadBytes2(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size) {
	if (I2C_BUS_NOT_BUSY) {
		for (uint8_t index = 0x00; index < size; index++) {
			I2C_ReadByte(address, reg + index, data++);
		}
	}

	return(RETURN_OK);
}
*/


bool I2C_RawStart(uint8_t deviceAddr, bool Direction)
{
	volatile uint8_t timeout;

	deviceAddr = (deviceAddr << 1) | Direction; // TRUE - read, FALSE - write

	if (I2C_BUS_NOT_BUSY)
	{
		for (volatile uint16_t arbLoop = 0x04; arbLoop > 0; arbLoop--) {
			TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);
			TWI0.MADDR = deviceAddr;
			if (Direction) {
				// addressRead
				for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_RIF_bm) && (timeout > 0); timeout--) {
				}
			} else {

				// addressWrite
				for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_WIF_bm) && (timeout > 0); timeout--) {
				}
			}

			if (I2C_ARBLOST) {
				// Arbitration lost - keep retrying.
				continue;
			}

			if (I2C_BUSERR) {
				// Bus error - abort.
				return(RETURN_FAILED);		// RETURN_BUS_ERROR);
			}

			if (I2C_STATE_IS_LOW(TWI_RXACK_bm)) {
				// Slave responding.
				return(RETURN_OK);		// RETURN_OK);
			} else {
				// Slave not responding - abort.
				return(RETURN_FAILED);		// RETURN_NO_SLAVE);
			}
		}
	} else {
		return(RETURN_FAILED);		// RETURN_BUS_BUSY);
	}

	return(RETURN_FAILED);		// RETURN_BUS_ARBLOST);
}


bool I2C_RawRead(uint8_t ACK, uint8_t *data)							// ACK=1 send ACK ; ACK=0 send NACK
{
	volatile uint8_t timeout;

	if (I2C_BUS_OWNER)
	{
		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--) {
			// Wait until we have a clock hold.
		}
		if (timeout == 0) {
			// Even though we own the bus, we aren't holding the clock.
			return(RETURN_FAILED);
		}

		TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);

		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_RIF_bm) && (timeout > 0); timeout--) {
			// Wait until RIF set
		}
		if (timeout == 0) {
			// Timeout on waiting for RIF signal.
			return(RETURN_FAILED);
		}

		*data = TWI0.MDATA;
		if	(ACK == 1)	{TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);}		// si ACK=1 mise à 0 ACKACT => action send ack
		else			{TWI0.MCTRLB |= (1 << TWI_ACKACT_bp);	}	// sinon (ACK=0) => mise à 1 ACKACT => nack préparé pour actionstop

//		if (I2C_ARBLOST) {
//			// Arbitration lost - keep retrying.
//			continue;
//		}

		if (I2C_BUSERR) {
			// Bus error - abort.
			return(RETURN_FAILED);		// RETURN_BUS_ERROR);
		}

		if (I2C_STATE_IS_LOW(TWI_RXACK_bm)) {
			// Slave responding.
			return(RETURN_OK);		// RETURN_OK);
		} else {
			// Slave not responding - abort.
			return(RETURN_FAILED);		// RETURN_NO_SLAVE);
		}
	}

	return(RETURN_FAILED);
}


bool I2C_RawWrite(uint8_t write_data)
{
	volatile uint8_t timeout;

	if (I2C_BUS_OWNER)
	{
		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--) {
			// Wait until we have a clock hold.
		}
		if (timeout == 0) {
			// Even though we own the bus, we aren't holding the clock.
			return(RETURN_FAILED);
		}

		TWI0.MDATA = write_data;
		// while (!((TWI0.MSTATUS & TWI_WIF_bm) | (TWI0.MSTATUS & TWI_RXACK_bm))) ;		//Wait until WIF set and RXACK cleared

		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_WIF_bm) && (timeout > 0); timeout--) {
		}

//		if (I2C_ARBLOST) {
//			// Arbitration lost - keep retrying.
//			continue;
//		}

		if (I2C_BUSERR) {
			// Bus error - abort.
			return(RETURN_FAILED);		// RETURN_BUS_ERROR);
		}

		if (I2C_STATE_IS_LOW(TWI_RXACK_bm)) {
			// Slave responding.
			return(RETURN_OK);		// RETURN_OK);
		} else {
			// Slave not responding - abort.
			return(RETURN_FAILED);		// RETURN_NO_SLAVE);
		}
	}

	return(RETURN_FAILED);
}


bool I2C_RawStop(void)
{
	volatile uint8_t timeout;

	for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--) {
		// Wait until we have a clock hold.
	}
	if (timeout == 0) {
		// Even though we own the bus, we aren't holding the clock.
		return(RETURN_FAILED);
	}

	//TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
	TWI0.MCTRLB = TWI_MCMD_STOP_gc;
	for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--) {
		// Wait until we have a clock hold.
	}
	if (I2C_BUSERR) {
		// A bit of a fudge - I'm getting bus errors for some reason.
		// Doesn't seem to affect anything though.
		TWI0.MSTATUS = TWI_BUSERR_bm;
	}

	return(RETURN_OK);
}


void I2C_RawForceStop(void)
{

	I2C_RawStop();
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;	//Force TWI state machine into IDLE state
	TWI0.MCTRLB |= TWI_FLUSH_bm;			// Purge MADDR and MDATA
}


/*
uint8_t CheckReadState() {
	MStatus status;
	uint8_t returnValue = RETURN_WAIT;
	
	while(returnValue == RETURN_WAIT) {
		status.all = TWI0.MSTATUS;

		// Bus error.
		if (status.WIF && status.ARBLOST) {
			twi_send_stop();
			returnValue = RETURN_BUS_ERROR;
		}

		// Bus error.
		if (status.WIF && status.ARBLOST) {
			twi_send_stop();
			returnValue = RETURN_BUS_ERROR;
		}

		// No slave found.
		if (status.WIF && status.RXACK) {
			twi_send_stop();
			returnValue = RETURN_NO_SLAVE;
		}

		// Got an RXACK from the slave.
		if (status.RIF && !status.RXACK) {
			returnValue = RETURN_OK;
		}

		// Got an RXACK from the slave.
		if (status.RIF && !status.RXACK) {
			returnValue = RETURN_OK;
		}
	}

	return(returnValue);
}
*/
