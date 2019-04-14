//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I2C

bool I2C_RawRead(uint8_t ACK, uint8_t *data);
void InitI2C();
bool I2C_RawStart(uint8_t deviceAddr, bool Direction);
bool I2C_RawWrite(uint8_t write_data);
bool I2C_RawStop(void);
bool I2C_ReadByte(uint8_t address, uint8_t reg, uint8_t *data);
bool I2C_ReadBytes(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size);
bool I2C_WriteByte(uint8_t address, uint8_t reg, uint8_t data);
bool I2C_WriteBytes(uint8_t address, uint8_t reg, uint8_t data[], uint8_t size);
bool I2C_StartMaster();
bool I2C_StopMaster();
bool I2C_StartSlave();
bool I2C_StopSlave();


typedef union {
	struct {
		uint8_t BUSSTATE:2;
		uint8_t BUSERR:1;
		uint8_t ARBLOST:1;
		uint8_t RXACK:1;
		uint8_t CLKHOLD:1;
		uint8_t WIF:1;
		uint8_t RIF:1;
	};
	
	uint8_t all;
} MStatus;

#define MASTER_ENABLE	1
#define MASTER_DISABLE	0
#define SLAVE_ENABLE	1
#define SLAVE_DISABLE	0

#define I2C_READ 1
#define I2C_WRITE 0
#define I2C_WAIT_TIMEOUT	0xFF

#define RETURN_OK			0
#define RETURN_FAILED		1
#define RETURN_NO_SLAVE		2
#define RETURN_BUS_ERROR	3
#define RETURN_BUS_ARBLOST	4
#define RETURN_BUS_BUSY		5
