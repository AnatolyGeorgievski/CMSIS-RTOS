/* r3_device */
#include "atomic.h"
#include "r3_tree.h"
#include "r3_asn.h"

enum _BACnetObjectType {
ANALOG_INPUT = 0,
ANALOG_OUTPUT = 1,
ANALOG_VALUE = 2,
BINARY_INPUT = 3,
BINARY_OUTPUT = 4,
BINARY_VALUE = 5,
CALENDAR = 6,
COMMAND = 7,
DEVICE = 8,
EVENT_ENROLLMENT = 9,
BACnetObjectType_FILE = 10,
GROUP = 11,
LOOP = 12,
MULTI_STATE_INPUT = 13,
MULTI_STATE_OUTPUT = 14,
NOTIFICATION_CLASS = 15,
PROGRAM = 16,
SCHEDULE = 17,
AVERAGING = 18,
MULTI_STATE_VALUE = 19,
TREND_LOG = 20,
LIFE_SAFETY_POINT = 21,
LIFE_SAFETY_ZONE = 22,
ACCUMULATOR = 23,
PULSE_CONVERTER = 24,
EVENT_LOG = 25,
GLOBAL_GROUP = 26,
TREND_LOG_MULTIPLE = 27,
LOAD_CONTROL = 28,
STRUCTURED_VIEW = 29,
ACCESS_DOOR = 30,
TIMER = 31,
ACCESS_CREDENTIAL = 32,
ACCESS_POINT = 33,
ACCESS_RIGHTS = 34,
ACCESS_USER = 35,
ACCESS_ZONE = 36,
CREDENTIAL_DATA_INPUT = 37,
NETWORK_SECURITY = 38,
BITSTRING_VALUE = 39,
CHARACTERSTRING_VALUE = 40,
DATE_PATTERN_VALUE = 41,
DATE_VALUE = 42,
DATETIME_PATTERN_VALUE = 43,
DATETIME_VALUE = 44,
INTEGER_VALUE = 45,
LARGE_ANALOG_VALUE = 46,
OCTETSTRING_VALUE = 47,
POSITIVE_INTEGER_VALUE = 48,
TIME_PATTERN_VALUE = 49,
TIME_VALUE = 50,
NOTIFICATION_FORWARDER = 51,
ALERT_ENROLLMENT = 52,
CHANNEL = 53,
LIGHTING_OUTPUT = 54,
BINARY_LIGHTING_OUTPUT = 55,
NETWORK_PORT = 56,
ELEVATOR_GROUP = 57,
ESCALATOR = 58,
LIFT = 59,
};


#define T(name) {name, sizeof(name)/sizeof(ParamSpec_t), sizeof(struct _BACnet##name)}
const DeviceObjectClass device_object_classes[] = {
[ANALOG_INPUT]  = T(AnalogInputObject),
[ANALOG_OUTPUT] = T(AnalogOutputObject),
[ANALOG_VALUE]  = T(AnalogValueObject),
[BINARY_INPUT]  = T(BinaryInputObject),
[BINARY_OUTPUT] = T(BinaryOutputObject),
[BINARY_VALUE]  = T(BinaryValueObject),
[COMMAND]       = T(CommandObject),
//[SCHEDULE]      = T(CommandObject),
};
#undef T


typedef struct _Device Device_t;
struct _Device {
	struct _Object instance;//!< то что отличает каждый объект
	struct _NetworkPort ports[1];//число портов на плате
};

struct _Device Device_tree_config = {
	ports = 
	[0]={"port0", arg=USART1, .init = MODBUS_MASTER, .bind={oid=, prop_id=, .array_index=3, }},
	
};
obj = device_tree_lookup(MODBUS_MASTER)
void* value = obj->init(USART1);
if (obj->service) service_run(obj->service, value);
if (obj->bind) {
	void** property_ref = r3_object_reference(bind->oid, bind->property_id, bind->array_index);
	if (property_ref) *property_ref = value;
}



static tree_t *device_tree=NULL;
static tree_t *object_classes=NULL;

/*! добавить описание класса в систему */
void device_class_add(uint32_t key, const DeviceObjectClass *obj_class)
{
	tree_t *leaf = g_slice_alloc(sizeof(tree_t));
	tree_init(leaf, key, obj_class);
    leaf = tree_insert(&object_classes, leaf);
}

static void __attribute__((constructor)) device_tree_init()
{
	//device_class_add(ANALOG_INPUT, const DeviceObjectClass *obj_class);

	// создать объект
	tree_t *leaf = g_slice_alloc(sizeof(tree_t));
	tree_init(leaf, (DEVICE<<22|1), &device);
    leaf = tree_insert(&device_tree, leaf);

}
