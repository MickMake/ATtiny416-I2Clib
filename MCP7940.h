//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MCP7940

uint8_t ByteToBcd(uint8_t byteDecimal);
uint8_t BcdToByte(uint8_t byteBCD);
void InitRTC();
bool GetState();
bool GetDateTime(uint8_t *Year, uint8_t *Month, uint8_t *Day, uint8_t *WeekDay, uint8_t *Hour, uint8_t *Minute, uint8_t *Second);
bool SetDateTime(uint8_t Year, uint8_t Month, uint8_t Day, uint8_t WeekDay, uint8_t Hour, uint8_t Minute, uint8_t Second);
bool GetTime(uint8_t *Hour, uint8_t *Minute, uint8_t *Second);
bool SetTime(uint8_t Hour, uint8_t Minute, uint8_t Second);
bool GetDate(uint8_t *Year, uint8_t *Month, uint8_t *Day);
bool SetDate(uint8_t Year, uint8_t Month, uint8_t Day);
uint8_t GetSeconds();
bool SetSeconds(uint8_t Seconds);
uint8_t GetMinutes();
bool SetMinutes(uint8_t Minutes);
uint8_t GetHours();
bool SetHours(uint8_t Hours);
uint8_t GetWeekDay();
bool SetWeekDay(uint8_t WeekDay);
uint8_t GetDay();
bool SetDay(uint8_t Day);
uint8_t GetMonth();
bool SetMonth(uint8_t Month);
uint8_t GetYear();
bool SetYear(uint8_t Year);


#define MCP7940_ADDR  0x6F
    // I2C Address for the RTC


// uint8_t      timeStamp[7];         // Byte array holding a full time stamp.

/////////////////////////////////////////////////////////////////////////////////////////
// Time
#define RTC_REG_SECOND 0x00
    // Register Address: Time Second
typedef union {
	struct {
		uint8_t bcd:7;
		uint8_t Enable:1;
	};

	uint8_t all;
} rtcSeconds;


#define RTC_REG_MINUTE 0x01
    // Register Address: Time Minute
typedef union {
	struct {
		uint8_t bcd:7;
		uint8_t :1;
	};

	uint8_t all;
} rtcMinutes;


#define RTC_REG_HOUR   0x02
    // Register Address: Time Hour
typedef union {
	struct {
		uint8_t bcd:6;		// Keep things simple. Always store in 24 hour format.
		uint8_t HourFormat:1;
		uint8_t :1;
	};
	struct {
		uint8_t bcdAMPM:5;	// 12 hour format reference.
		uint8_t AMPM:1;		// 12 hour format reference.
		uint8_t :1;
		uint8_t :1;
	};

	uint8_t all;
} rtcHours;


/////////////////////////////////////////////////////////////////////////////////////////
// DOW
#define RTC_REG_WEEKDAY  0x03
    // Register Address: Date Day of Week
typedef union {
	struct {
		uint8_t bcd:3;
		uint8_t Running:1;
		uint8_t PowerFail:1;
		uint8_t Battery:1;
		uint8_t :2;
	};

	uint8_t all;
} rtcWeekDay;


/////////////////////////////////////////////////////////////////////////////////////////
// Date
#define RTC_REG_DAY   0x04
    // Register Address: Date Day
typedef union {
	struct {
		uint8_t bcd:6;
		uint8_t :2;
	};

	uint8_t all;
} rtcDay;


#define RTC_REG_MONTH 0x05
    // Register Address: Date Month
typedef union {
	struct {
		uint8_t bcd:5;
		uint8_t LeapYear:1;
		uint8_t :2;
	};

	uint8_t all;
} rtcMonth;


#define RTC_REG_YEAR   0x06
    // Register Address: Date Year
typedef union {
	struct {
		uint8_t bcd:8;
	};

	uint8_t all;
} rtcYear;


#define RTC_REG_TIME   RTC_REG_SECOND
#define RTC_REG_TIME_COUNT	 3
#define RTC_REG_DATE_COUNT	 3
#define RTC_REG_DATE   RTC_REG_DAY
#define RTC_REG_DATETIME_COUNT	 7
typedef union {
	struct {
		rtcSeconds	Second;
		rtcMinutes	Minute;
		rtcHours	Hour;
		rtcWeekDay	WeekDay;
		rtcDay		Day;
		rtcMonth	Month;
		rtcYear		Year;
	};

	uint8_t all[RTC_REG_DATETIME_COUNT];
} rtcDateTime;



/////////////////////////////////////////////////////////////////////////////////////////
// RTC control
#define RTC_PWM_1HZ			0x11
#define RTC_PWM_4096HZ		0x10
#define RTC_PWM_8192HZ		0x01
#define RTC_PWM_32768HZ		0x00
#define RTC_REG_OUTPUT	 0x07
typedef union {
	struct {
		uint8_t PwmFrequency:2;	// 00:1 Hz - 01:4096 Hz - 10:8192 Hz - 11:32768 Hz
		uint8_t PwmTrim:1;		// 1:PWM coarse trim	- 0:PWM fine trim.
		uint8_t ExternalOsc:1;	// 1:External clock in	- 0:Disable external clock.
		uint8_t Alarm0Enable:1;	// 1:Alarm 0 enabled	- 0:Alarm 0 disabled.
		uint8_t Alarm1Enable:1;	// 1:Alarm 1 enabled	- 0:Alarm 1 disabled.
		uint8_t PwmEnable:1;	// 1:PWM on MFP output	- 0:Disable PWM.
		uint8_t MFP:1;			// When using the MFP as a plain GPIO.
	};

	uint8_t all;
} rtcOutput;


#define RTC_REG_OSCTRIM   0x08
typedef union {
	struct {
		uint8_t PwmTrimValue:7;	// 00:1 Hz - 01:4096 Hz - 10:8192 Hz - 11:32768 Hz
		uint8_t Sign:1;			// When using the MFP as a plain GPIO.
	};

	uint8_t all;
} rtcOscTrim;


#define RTC_REG_CONTROL	 RTC_REG_OUTPUT
#define RTC_REG_CONTROL_COUNT	 2
typedef union {
	struct {
		rtcOutput	Output;
		rtcOscTrim	OscTrim;
	};

	uint8_t all[RTC_REG_CONTROL_COUNT];
} rtcControl;



/////////////////////////////////////////////////////////////////////////////////////////
// Reserved
#define RTC_REG_RESERVED09   0x09
typedef union {
	uint8_t all;
} rtcReserved;


/////////////////////////////////////////////////////////////////////////////////////////
// Alarms 0 & 1
#define RTC_REG_ALM0_SECOND 0x0A
    // Register Address: Time Second
#define RTC_REG_ALM1_SECOND 0x11
    // Register Address: Time Second

typedef union {
	struct {
		uint8_t bcd:7;
		uint8_t :1;
	};

	uint8_t all;
} rtcAlarmSeconds;


#define RTC_REG_ALM0_MINUTE 0x0B
    // Register Address: Time Minute
#define RTC_REG_ALM1_MINUTE 0x12
    // Register Address: Time Minute
typedef union {
	struct {
		uint8_t bcd:7;
		uint8_t :1;
	};

	uint8_t all;
} rtcAlarmMinutes;


#define RTC_REG_ALM0_HOUR   0x0C
    // Register Address: Time Hour
#define RTC_REG_ALM1_HOUR   0x13
    // Register Address: Time Hour
typedef union {
	struct {
		uint8_t bcd:6;		// Keep things simple. Always store in 24 hour format.
		uint8_t HourFormat:1;
		uint8_t :1;
	};
	struct {
		uint8_t bcdAMPM:5;	// 12 hour format reference.
		uint8_t AMPM:1;		// 12 hour format reference.
		uint8_t :1;
		uint8_t :1;
	};

	uint8_t all;
} rtcAlarmHours;


#define RTC_ALM_MATCH_SECONDS	0x00
#define RTC_ALM_MATCH_MINUTES	0x01
#define RTC_ALM_MATCH_HOURS	0x02
#define RTC_ALM_MATCH_DOW		0x03
#define RTC_ALM_MATCH_DATE		0x04
#define RTC_ALM_MATCH_ALL		0x07
#define RTC_REG_ALM0_WEEKDAY  0x0D
    // Register Address: Date Day of Week
#define RTC_REG_ALM1_WEEKDAY  0x14
    // Register Address: Date Day of Week
typedef union {
	struct {
		uint8_t bcd:3;					// BCD weekday.
		uint8_t Interrupt:1;			// 1:Alarm triggered		- 0:No alarm.
		uint8_t Mask:1;					// 000: Seconds match.
										// 001: Minutes match.
										// 010: Hours match.
										// 011: DOW match.
										// 100: Date match.
										// 111: All match.
		uint8_t Polarity:1;				// 1:Positive logic			- 0:Negative logic.
	};

	uint8_t all;
} rtcAlarmWeekDay;


#define RTC_REG_ALM0_DAY   0x0E
    // Register Address: Date Day
#define RTC_REG_ALM1_DAY   0x15
    // Register Address: Date Day
typedef union {
	struct {
		uint8_t bcd:6;
		uint8_t :2;
	};

	uint8_t all;
} rtcAlarmDay;


#define RTC_REG_ALM0_MONTH 0x0F
    // Register Address: Date Month
#define RTC_REG_ALM1_MONTH 0x16
    // Register Address: Date Month
typedef union {
	struct {
		uint8_t bcd:5;
		uint8_t :3;
	};

	uint8_t all;
} rtcAlarmMonth;


#define RTC_REG_ALARM0	 RTC_REG_ALM0_SECOND
#define RTC_REG_ALARM1	 RTC_REG_ALM1_SECOND
#define RTC_REG_ALARM_COUNT	6
typedef union {
	struct {
		rtcAlarmSeconds	Second;
		rtcAlarmMinutes	Minute;
		rtcAlarmHours	Hour;
		rtcAlarmWeekDay	WeekDay;
		rtcAlarmDay		Day;
		rtcAlarmMonth	Month;
	};

	uint8_t all[RTC_REG_ALARM_COUNT];
} rtcAlarm;



/////////////////////////////////////////////////////////////////////////////////////////
// Reserved
#define RTC_REG_RESERVED10   0x10
#define RTC_REG_RESERVED17   0x17



/////////////////////////////////////////////////////////////////////////////////////////
// Power up/down registers.
#define RTC_REG_POWERDN_MINUTE 0x18
    // Register Address: Time Minute
#define RTC_REG_POWERUP_MINUTE 0x1C
    // Register Address: Time Minute
typedef union {
	struct {
		uint8_t bcd:7;
		uint8_t :1;
	};

	uint8_t all;
} rtcPowerMinutes;


#define RTC_REG_POWERDN_HOUR   0x19
    // Register Address: Time Hour
#define RTC_REG_POWERUP_HOUR   0x1D
    // Register Address: Time Hour
typedef union {
	struct {
		uint8_t bcd:6;		// Keep things simple. Always store in 24 hour format.
		uint8_t HourFormat:1;
		uint8_t :1;
	};
	struct {
		uint8_t bcdAMPM:5;	// 12 hour format reference.
		uint8_t AMPM:1;		// 12 hour format reference.
		uint8_t :1;
		uint8_t :1;
	};

	uint8_t all;
} rtcPowerHours;


#define RTC_REG_POWERDN_DAY   0x1A
    // Register Address: Date Day
#define RTC_REG_POWERUP_DAY   0x1E
    // Register Address: Date Day
typedef union {
	struct {
		uint8_t bcd:6;
		uint8_t :2;
	};

	uint8_t all;
} rtcPowerDay;


#define RTC_REG_POWERDN_MONTH 0x1B
    // Register Address: Date Month
#define RTC_REG_POWERUP_MONTH 0x1F
    // Register Address: Date Month
typedef union {
	struct {
		uint8_t bcd:5;
		uint8_t WeekDay:3;
	};

	uint8_t all;
} rtcPowerMonth;


#define RTC_REG_POWERDN	 RTC_REG_POWERDN_MINUTE
#define RTC_REG_POWERUP	 RTC_REG_POWERUP_MINUTE
#define RTC_REG_POWER_COUNT		4
typedef union {
	struct {
		rtcPowerMinutes	Minute;
		rtcPowerHours	Hour;
		rtcPowerDay		Day;
		rtcPowerMonth	Month;
	};

	uint8_t all[RTC_REG_POWER_COUNT];
} rtcPower;



/*
#define RTCDATE_SECONDS	0
#define RTCDATE_MINUTES	1
#define RTCDATE_HOURS	2
#define RTCDATE_WEEKDAY	3
#define RTCDATE_DAY		4
#define RTCDATE_MONTH	5
#define RTCDATE_YEAR	6
#define RTCDATE_TIME	RTCDATE_SECONDS
#define RTCDATE_DATE	RTCDATE_DAY
*/
