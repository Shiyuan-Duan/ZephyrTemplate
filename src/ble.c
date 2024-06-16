

// ble.c
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include "ble.h"
#include <max30003.h>

#define PRIO K_HIGHEST_APPLICATION_THREAD_PRIO

LOG_MODULE_REGISTER(BLE);

static bool button_state;
static bool notify_mysensor_enabled;
static char dbg_message[1024]="Hello world\n\0";
static struct ble_cb cb;

const struct device *max30003_dev = DEVICE_DT_GET(DT_NODELABEL(max30003));

static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;
	
	printk("Read button triggered!\n");
	
	// max30003_print_cnfg(max30003_dev);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, 1024);
	// return 0;
}

static void ble_ccc_exg_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}


BT_GATT_SERVICE_DEFINE(
	exg_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_EXG),
	BT_GATT_CHARACTERISTIC(BT_UUID_EXG_BUTTON, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_button, NULL, &dbg_message),
	BT_GATT_CHARACTERISTIC(BT_UUID_EXG_SENSOR, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL,
			       NULL, NULL),

	BT_GATT_CCC(ble_ccc_exg_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	// BT_GATT_CHARACTERISTIC(BT_UUID_EXG_LED, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL, write_led, NULL)
);

int init_ble_service(struct ble_cb *callbacks){
	cb.dbg_cb = callbacks->dbg_cb;
	return 0;
}

static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL); /* Set to NULL for undirected advertising */

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_EXG_VAL),
};

static void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
		return;
	}

	printk("Connected\n");
	ssize_t mtu = bt_gatt_get_mtu(conn);
	printk("MTU: %d\n", mtu);


}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);


}
struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

int stream_sensor_data(uint32_t *sensor_value, ssize_t size)
{
	if (!notify_mysensor_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &exg_service.attrs[3], sensor_value, size);
}



int ble_main(void)
{	

	int err;
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}

	bt_conn_cb_register(&connection_callbacks);

	LOG_INF("Bluetooth initialized\n");
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}

	// stream_sensor_data(1);

	LOG_INF("Advertising successfully started with prioirty: %d\n", PRIO);
	return 0;
}

K_THREAD_DEFINE(ble_t, 4096, ble_main, NULL, NULL, NULL, PRIO, 0, 0);