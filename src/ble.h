//ble.h

#ifndef BLE_H_
#define BLE_H_

#include <zephyr/types.h>

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
//000062c4-b99e-4141-9439-c4f9db977899
#define BT_UUID_EXG_VAL \
	BT_UUID_128_ENCODE(0x000062c4, 0xb99e, 0x4141, 0x9439, 0xc4f9db977899)

/** @brief Button Characteristic UUID. */
#define BT_UUID_EXG_BUTTON_VAL \
	BT_UUID_128_ENCODE(0x000062c5, 0xb99e, 0x4141, 0x9439, 0xc4f9db977899)

/** @brief LED Characteristic UUID. */
#define BT_UUID_EXG_LED_VAL \
	BT_UUID_128_ENCODE(0x000062c6, 0xb99e, 0x4141, 0x9439, 0xc4f9db977899)

#define BT_UUID_EXG_SENSOR_VAL \
	BT_UUID_128_ENCODE(0x000062c7, 0xb99e, 0x4141, 0x9439, 0xc4f9db977899)


#define BT_UUID_EXG           BT_UUID_DECLARE_128(BT_UUID_EXG_VAL)
#define BT_UUID_EXG_BUTTON    BT_UUID_DECLARE_128(BT_UUID_EXG_BUTTON_VAL)
#define BT_UUID_EXG_LED       BT_UUID_DECLARE_128(BT_UUID_EXG_LED_VAL)
#define BT_UUID_EXG_SENSOR	  BT_UUID_DECLARE_128(BT_UUID_EXG_SENSOR_VAL)


typedef int (*dbg_cb_t) (void);
struct ble_cb  {
	dbg_cb_t dbg_cb; 
};

int init_ble_service(struct ble_cb *callbacks);

/**
 * @}
 */

#endif /* BLE_H_ */