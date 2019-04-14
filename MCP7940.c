//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MCP7940

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include "MCP7940.h"
#include "I2C.h"

// Reduce memory footprint - keep it global.
volatile rtcDateTime DateTime;
volatile rtcControl Control;
volatile rtcAlarm Alarm0;
volatile rtcAlarm Alarm1;
volatile rtcPower PowerDown;
volatile rtcPower PowerUp;


uint8_t ByteToBcd(uint8_t byteDecimal) {
	
	return((byteDecimal / 10) << 4 | (byteDecimal % 10));
}


uint8_t BcdToByte(uint8_t byteBCD) {
	uint8_t byteMSB = 0;
	uint8_t byteLSB = 0;
	byteMSB = (byteBCD & 0b11110000) >> 4;
	byteLSB = (byteBCD & 0b00001111);
	return((byteMSB*10) + byteLSB);
}


void InitRTC() {

	// First check the ST bit.
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_SECOND, &DateTime.Second.all);
	if (DateTime.Second.Enable == 0) {
		// Enable clock.
		DateTime.Second.Enable = 0x01;
		I2C_WriteByte(MCP7940_ADDR, RTC_REG_SECOND, DateTime.Second.all);
	}

}


bool GetState() {

	// Fetch all stats.
	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_SECOND, &(DateTime.all[RTC_REG_SECOND]), RTC_REG_DATETIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_CONTROL, &(Control.all[RTC_REG_CONTROL]), RTC_REG_CONTROL_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_POWERDN, &(PowerDown.all[RTC_REG_POWERDN]), RTC_REG_POWER_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_POWERUP, &(PowerDown.all[RTC_REG_POWERUP]), RTC_REG_POWER_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

/*
	volatile uint8_t foo[0x20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t index = 0x00;
	foo[index++] = DateTime.Second.Enable;
	foo[index++] = DateTime.WeekDay.Battery;
	foo[index++] = DateTime.WeekDay.PowerFail;
	foo[index++] = DateTime.WeekDay.Running;
	
	foo[index++] = Control.Output.all;		// 4
	foo[index++] = Control.OscTrim.all;
	
	foo[index++] = PowerDown.Minute.all;		// 6
	foo[index++] = PowerDown.Hour.all;
	foo[index++] = PowerDown.Day.all;
	foo[index++] = PowerDown.Month.all;
	
	foo[index++] = PowerUp.Minute.all;			// 10
	foo[index++] = PowerUp.Hour.all;
	foo[index++] = PowerUp.Day.all;
	foo[index++] = PowerUp.Month.all;
	foo[index++] = DateTime.WeekDay.Battery;
*/

	return(RETURN_OK);
}


bool GetDateTime(uint8_t *Year, uint8_t *Month, uint8_t *Day, uint8_t *WeekDay, uint8_t *Hour, uint8_t *Minute, uint8_t *Second) {

	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_SECOND, &(DateTime.all[RTC_REG_SECOND]), RTC_REG_DATETIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	if (DateTime.Hour.HourFormat) {
		// Make sure we have the time format in 24 hour - makes things easier.
		// So we'll need to convert from 12 hour to 24 hour.
		DateTime.Hour.HourFormat = 0;
		if (DateTime.Hour.AMPM) {
			DateTime.Hour.bcd = ByteToBcd(BcdToByte(DateTime.Hour.bcdAMPM) + 12);
		} else {
			DateTime.Hour.bcd = DateTime.Hour.bcdAMPM;
		}
	}

	*Year = BcdToByte(DateTime.Year.bcd);
	*Month = BcdToByte(DateTime.Month.bcd);
	*Day = BcdToByte(DateTime.Day.bcd);
	*WeekDay = BcdToByte(DateTime.WeekDay.bcd);
	*Hour = BcdToByte(DateTime.Hour.bcd);
	*Minute = BcdToByte(DateTime.Minute.bcd);
	*Second = BcdToByte(DateTime.Second.bcd);

	return(RETURN_OK);
}


bool SetDateTime(uint8_t Year, uint8_t Month, uint8_t Day, uint8_t WeekDay, uint8_t Hour, uint8_t Minute, uint8_t Second) {

	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_SECOND, &(DateTime.all[RTC_REG_SECOND]), RTC_REG_DATETIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	if (DateTime.Hour.HourFormat) {
		// Make sure we have the time format in 24 hour - makes things easier.
		DateTime.Hour.HourFormat = 0;
	}

	DateTime.Year.bcd = ByteToBcd(Year);
	DateTime.Month.bcd = ByteToBcd(Month);
	DateTime.Day.bcd = ByteToBcd(Day);
	DateTime.WeekDay.bcd = ByteToBcd(WeekDay);
	DateTime.Hour.bcd = ByteToBcd(Hour);
	DateTime.Minute.bcd = ByteToBcd(Minute);
	DateTime.Second.bcd = ByteToBcd(Second);
	if (I2C_WriteBytes(MCP7940_ADDR, RTC_REG_SECOND, &(DateTime.all[RTC_REG_SECOND]), RTC_REG_DATETIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


bool GetTime(uint8_t *Hour, uint8_t *Minute, uint8_t *Second) {

	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_TIME, &(DateTime.all[RTC_REG_TIME]), RTC_REG_TIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	*Second = BcdToByte(DateTime.Second.bcd);
	*Minute = BcdToByte(DateTime.Minute.bcd);
	*Hour = BcdToByte(DateTime.Hour.bcd);

	return(RETURN_OK);
}


bool SetTime(uint8_t Hour, uint8_t Minute, uint8_t Second) {

	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_TIME, &(DateTime.all[RTC_REG_TIME]), RTC_REG_TIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Hour.bcd = ByteToBcd(Hour);
	DateTime.Minute.bcd = ByteToBcd(Minute);
	DateTime.Second.bcd = ByteToBcd(Second);
	if (I2C_WriteBytes(MCP7940_ADDR, RTC_REG_TIME, &(DateTime.all[RTC_REG_TIME]), RTC_REG_TIME_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


bool GetDate(uint8_t *Year, uint8_t *Month, uint8_t *Day) {

	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_DATE, &(DateTime.all[RTC_REG_DATE]), RTC_REG_DATE_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	*Day = BcdToByte(DateTime.Day.bcd);
	*Month = BcdToByte(DateTime.Month.bcd);
	*Year = BcdToByte(DateTime.Year.bcd);

	return(RETURN_OK);
}


bool SetDate(uint8_t Year, uint8_t Month, uint8_t Day) {

	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadBytes(MCP7940_ADDR, RTC_REG_DATE, &(DateTime.all[RTC_REG_DATE]), RTC_REG_DATE_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Year.bcd = ByteToBcd(Year);
	DateTime.Month.bcd = ByteToBcd(Month);
	DateTime.Day.bcd = ByteToBcd(Day);
	if (I2C_WriteBytes(MCP7940_ADDR, RTC_REG_DATE, &(DateTime.all[RTC_REG_DATE]), RTC_REG_DATE_COUNT) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetSeconds() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_SECOND, &DateTime.Second.all);

	return(BcdToByte(DateTime.Second.bcd));
}


bool SetSeconds(uint8_t Seconds) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_SECOND, &DateTime.Second.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Second.bcd = ByteToBcd(Seconds);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_SECOND, DateTime.Second.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetMinutes() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_MINUTE, &DateTime.Minute.all);

	return(BcdToByte(DateTime.Minute.bcd));
}


bool SetMinutes(uint8_t Minutes) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_MINUTE, &DateTime.Minute.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Minute.bcd = ByteToBcd(Minutes);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_MINUTE, DateTime.Minute.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetHours() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_HOUR, &DateTime.Hour.all);

	return(BcdToByte(DateTime.Hour.bcd));
}


bool SetHours(uint8_t Hours) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_HOUR, &DateTime.Hour.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Hour.bcd = ByteToBcd(Hours);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_HOUR, DateTime.Hour.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetWeekDay() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_WEEKDAY, &DateTime.WeekDay.all);

	return(BcdToByte(DateTime.WeekDay.bcd));
}


bool SetWeekDay(uint8_t WeekDay) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_WEEKDAY, &DateTime.WeekDay.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.WeekDay.bcd = ByteToBcd(WeekDay);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_WEEKDAY, DateTime.WeekDay.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetDay() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_DAY, &DateTime.Day.all);

	return(BcdToByte(DateTime.Day.bcd));
}


bool SetDay(uint8_t Day) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_DAY, &DateTime.Day.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Day.bcd = ByteToBcd(Day);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_DAY, DateTime.Day.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetMonth() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_MONTH, &DateTime.Month.all);

	return(BcdToByte(DateTime.Month.bcd));
}


bool SetMonth(uint8_t Month) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_MONTH, &DateTime.Month.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Month.bcd = ByteToBcd(Month);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_MONTH, DateTime.Month.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}


uint8_t GetYear() {
	I2C_ReadByte(MCP7940_ADDR, RTC_REG_YEAR, &DateTime.Year.all);

	return(BcdToByte(DateTime.Year.bcd));
}


bool SetYear(uint8_t Year) {
	
	// Read first, to avoid over-writing extra information contained in registers.
	if (I2C_ReadByte(MCP7940_ADDR, RTC_REG_YEAR, &DateTime.Year.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	DateTime.Year.bcd = ByteToBcd(Year);
	if (I2C_WriteByte(MCP7940_ADDR, RTC_REG_YEAR, DateTime.Year.all) == RETURN_FAILED) {
		return(RETURN_FAILED);
	}

	return(RETURN_OK);
}

