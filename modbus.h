#ifndef R3_MODBUS
#define R3_MODBUS
#include <stdint.h>


#define MODBUS_MAX_ADU_LENGTH 260
#define MODBUS_MAX_COUNT 0x7D
#define MODBUS_LENGTH_OFFSET 4
#define MODBUS_SLAVE_ADDRESS_OFFSET 6
#define MODBUS_MBAP_HEADER_LENGTH 7
#define MODBUS_TCP_HEADER 7
#define MODBUS_RTU_HEADER 1
#define MODBUS_RTU_MAX_LENGTH 253

enum {
    ERR_ILLEGAL_FUNCTION=0x01,
    ERR_ILLEGAL_DATA_ADDRESS=0x02,
    ERR_ILLEGAL_DATA_VALUE=0x03,
    ERR_SERVER_DEVICE_FAILURE=0x04,// An unrecoverable error occurred while the server was attempting to perform the requested action.
    ERR_ACKNOWLEDGE=0x05,// Specialized use in conjunction with programming commands.
    ERR_SERVER_DEVICE_BUSY=0x06,
    ERR_MEMORY_PARITY_ERROR=0x08,
    ERR_GATEWAY_PATH_UNAVAILABLE=0x0A,
    ERR_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND=0x0B,

};

int  modbus_request_encode(uint8_t* pdu, uint8_t function_code, uint16_t address, uint16_t value);
void modbus_mbap_encode(uint8_t * pdu, uint16_t sec_id, uint16_t len);
int  modbus_mbap_decode(uint8_t * pdu, uint16_t sec_id, uint16_t len);

typedef struct _ModbusServerCtx ModbusServerCtx;
typedef struct _ModbusHostCtx ModbusHostCtx;
typedef struct _ModbusDeviceCtx ModbusDeviceCtx;
typedef struct _ModbusCtx ModbusCtx;
typedef struct _ModbusMap ModbusMap;
typedef struct _ModbusEncObj ModbusEncObj;

struct _ModbusEncObj {
    uint8_t id;// идентификатор объекта
    uint8_t len;
    uint8_t *data;
};
struct _ModbusMap {
	uint16_t reg_addr;	//!< адрес в единицах uint16_t
	uint16_t reg_length;//!< размер карты в единицах uint16_t 
	void* data;
};
struct _ModbusCtx {
//    ModbusMasterCtx* busmaster;// линия

    ModbusEncObj* mei;	//!< список атрибутов MEI
	uint16_t mei_count;	//!< число элементов в массиве MEI
    ModbusMap * map_regs_rw;
	
    ModbusMap * map_regs_ro;
	size_t map_regs_ro_num;//!< число элементов в массиве map_regs_ro
    ModbusMap * map_bits_rw;
    ModbusMap * map_bits_ro;
};


enum MMapRange {
    MODBUS_MAP_REGS_OUTPUT,
    MODBUS_MAP_REGS_INPUT,
    MODBUS_MAP_COILS_OUTPUT,
    MODBUS_MAP_COILS_INPUT,
};

#define ModbusMapDef(name, addr, size, data) \
static ModbusMap mbmap_def_##name= {data, addr, size};

int         modbus_server_state(ModbusCtx* ctx, uint8_t *pdu, int size);
ModbusCtx*  modbus_context_new ();
void        modbus_context_free(ModbusCtx* ctx);
void        modbus_context_insert_map(ModbusCtx* ctx, enum MMapRange range, ModbusMap* mmap);

/* эти определения годятся для для LITTLE_ENDIAN */
#include <sys/param.h> // GCC
#if (BYTE_ORDER==LITTLE_ENDIAN)
static inline uint16_t htons(uint16_t a){
    return __builtin_bswap16(a);
}
static inline uint16_t ntohs(uint16_t a){
    return __builtin_bswap16(a);
}
#else
#define htons(a) (a)
#define ntohs(a) (a)
#endif

#endif //R3_MODBUS
