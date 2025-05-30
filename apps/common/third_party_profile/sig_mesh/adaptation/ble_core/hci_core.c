/*  Bluetooth Mesh */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "adaptation.h"
#include "bluetooth.h"
#include "ble/hci_ll.h"
#include "app_config.h"
#include "rcsp_bluetooth.h"
#include "custom_cfg.h"

#define LOG_TAG             "[MESH-hci_core]"
#define LOG_INFO_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_WARN_ENABLE
#define LOG_ERROR_ENABLE
#define LOG_DUMP_ENABLE
#include "mesh_log.h"

#if 1//RCSP_BTMATE_EN
#define ATT_LOCAL_PAYLOAD_SIZE    (200)                   //note: need >= 20
#define ATT_SEND_CBUF_SIZE        (512)                   //note: need >= 20,缓存大小，可修改
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_PAYLOAD_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));
#endif

#if ADAPTATION_COMPILE_DEBUG

int bt_pub_key_gen(struct bt_pub_key_cb *new_cb)
{
    return 0;
}

const u8_t *bt_pub_key_get(void)
{
    return NULL;
}

int bt_dh_key_gen(const u8_t remote_pk[64], bt_dh_key_cb_t cb)
{
    return 0;
}

struct bt_conn *bt_conn_ref(struct bt_conn *conn)
{
    return NULL;
}

void bt_conn_unref(struct bt_conn *conn) {}

void bt_conn_cb_register(struct bt_conn_cb *cb) {}

void hci_core_init(void) {}

#else

struct bt_hci_evt_le_p256_public_key_complete {
    u8_t status;
    u8_t *key;
} __packed;

struct bt_hci_evt_le_generate_dhkey_complete {
    u8_t status;
    u8_t dhkey[32];
} __packed;

struct bt_conn conn;
static bool is_connect;
static u8_t pub_key[64];
static struct bt_pub_key_cb *pub_key_cb;
static bt_dh_key_cb_t dh_key_cb;
static struct bt_conn_cb *callback_list;

extern void mesh_hci_init(void);
extern void resume_mesh_gatt_proxy_adv_thread(void);
extern void handle_scan_callback(uint8_t *packet, uint16_t size);
extern void mesh_can_send_now_wakeup(void);
extern void mesh_set_ble_work_state(ble_state_e state);

//read key 是否自动提高时钟频率，默认1,可通过接口 ll_mesh_auto_clock_up_enable 传0关掉
extern void ll_mesh_auto_clock_up_enable(u8 enable);

void ble_read_local_p256_public_key(void)
{
#if CMD_DIRECT_TO_BTCTRLER_TASK_EN
    extern int ll_hci_read_local_p256_pb_key(void);

    ll_hci_read_local_p256_pb_key();
#else
    extern void ll_read_local_p256_public_key(void);

    ll_read_local_p256_public_key();
#endif /* CMD_DIRECT_TO_BTCTRLER_TASK_EN */
}

void ble_generate_dh_key(const u8 *remote_pk)
{
#if 0
    extern int ll_hci_generate_dhkey(const u8 * data, u32 size);
    static u8 data[64];

    memcpy(data, remote_pk, sizeof(data));

    ll_hci_generate_dhkey(data, sizeof(data));
#else
    extern void ll_generate_dh_key(const u8 * data, u32 size);

    ll_generate_dh_key(remote_pk, 64);
#endif /* CMD_DIRECT_TO_BTCTRLER_TASK_EN */
}

int bt_pub_key_gen(struct bt_pub_key_cb *new_cb)

{
    struct bt_pub_key_cb *cb;

    if (pub_key_cb) {
        return 0;
    }

    new_cb->_next = pub_key_cb;
    pub_key_cb = new_cb;

    BT_INFO("BT_HCI_OP_LE_P256_PUBLIC_KEY");

    ble_read_local_p256_public_key();

    for (cb = pub_key_cb; cb; cb = cb->_next) {
        if (cb != new_cb) {
            cb->func(NULL);
        }
    }

    return 0;
}

const u8_t *bt_pub_key_get(void)
{
    return pub_key;
}

int bt_dh_key_gen(const u8_t remote_pk[64], bt_dh_key_cb_t cb)
{
    int err = 0;

    dh_key_cb = cb;

    /* BT_INFO("--func=%s", __FUNCTION__); */
    /* BT_INFO_HEXDUMP(remote_pk, 64); */

    ble_generate_dh_key(remote_pk);
    if (err) {
        dh_key_cb = NULL;
        return err;
    }

    return 0;
}

struct bt_conn *bt_conn_ref(struct bt_conn *conn)
{
    atomic_inc(&conn->ref);

    BT_DBG("handle %u ref %u", conn->handle, atomic_get(&conn->ref));

    return conn;
}

void bt_conn_unref(struct bt_conn *conn)
{
    atomic_dec(&conn->ref);

    BT_DBG("handle %u ref %u", conn->handle, atomic_get(&conn->ref));
}

void bt_conn_cb_register(struct bt_conn_cb *cb)
{
    callback_list = cb;
}

bool get_if_connecting(void)
{
    return is_connect;
}

void hci_set_mtu_callback(u16 mtu)
{
    conn.mtu = mtu;
}

u16 hci_get_conn_handle(void)
{
    return conn.handle;
}

static inline void hci_set_conn_run(u16 conn_handle)
{
    conn.handle = conn_handle;

    callback_list->connected(&conn, 0);

    is_connect = TRUE;
}

static inline void hci_set_disconn_run(void)
{
    callback_list->disconnected(&conn, 0);

    is_connect = FALSE;
}

static inline void le_pkey_complete(u8 *buf, u16 size)
{
    struct bt_hci_evt_le_p256_public_key_complete *evt = (void *)buf;

    BT_INFO("status: 0x%x", evt->status);
    BT_INFO_HEXDUMP(evt->key, 64);

    u8 key[64];
    sys_memcpy_swap(key, evt->key, 32);
    sys_memcpy_swap(&key[32], evt->key + 32, 32);

    struct bt_pub_key_cb *cb;

    if (!evt->status) {
        memcpy(pub_key, key, 64);
    }

    for (cb = pub_key_cb; cb; cb = cb->_next) {
        cb->func(evt->status ? NULL : key);
    }
}

static inline void le_dhkey_complete(u8 *buf, u16 size)
{
    struct bt_hci_evt_le_generate_dhkey_complete *evt = (void *)buf;

    BT_INFO("status: 0x%x", evt->status);
    BT_INFO_HEXDUMP(buf, size);

    if (dh_key_cb) {
        dh_key_cb(evt->status ? NULL : evt->dhkey);
        dh_key_cb = NULL;
    }
}

#define HCI_EVENT                                           0x04
#define HCI_DISCONNECTION_COMPLETE_EVENT                    0x05
#define HCI_LE_META_EVENT                                   0x3E
#define HCI_LE_CONNECTION_COMPLETE_EVENT                    0x01
#define HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT    0x08
#define HCI_LE_GENERATE_DHKEY_COMPLETE_EVENT                0x09

_WEAK_
u8 ble_update_get_ready_jump_flag(void)
{
    return 0;
}

void mesh_hci_event_callback(u8 packet_type, u8 channel, u8 *packet, u16 size)
{
    switch (packet_type) {
    case HCI_EVENT:
        switch (packet[0]) {
        case ATT_EVENT_CAN_SEND_NOW:
            mesh_can_send_now_wakeup();
            break;
        case HCI_DISCONNECTION_COMPLETE_EVENT:
            BT_INFO("HCI_DISCONNECTION_COMPLETE_EVENT");
            mesh_set_ble_work_state(BLE_ST_DISCONN);
#if RCSP_BTMATE_EN
            rcsp_exit();
#endif
            hci_set_disconn_run();
            if (!ble_update_get_ready_jump_flag()) {
                printf("resume_mesh_gatt_proxy_adv_thread\n");
                resume_mesh_gatt_proxy_adv_thread();
            }
            break;
        case HCI_LE_META_EVENT:
            switch (packet[2]) {
            case HCI_LE_CONNECTION_COMPLETE_EVENT:
                BT_INFO("HCI_LE_CONNECTION_COMPLETE_EVENT");
                u16 connection_handle = sys_get_le16(&packet[4]);
                hci_set_conn_run(connection_handle);
                BT_INFO("connection handle =0x%x", connection_handle);
                resume_mesh_gatt_proxy_adv_thread();
                ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, connection_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_PAYLOAD_SIZE);
#if RCSP_BTMATE_EN
                mesh_set_ble_work_state(BLE_ST_CONNECT);
#if (defined(BT_CONNECTION_VERIFY) && (0 == BT_CONNECTION_VERIFY))
                void JL_rcsp_auth_reset(void);
                JL_rcsp_auth_reset();
#endif
                //rcsp_dev_select(RCSP_BLE);
                rcsp_init();
#endif
                break;
            case HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT:
                BT_INFO("HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT");
                le_pkey_complete(packet + 3, size - 3);
                break;
            case HCI_LE_GENERATE_DHKEY_COMPLETE_EVENT:
                BT_INFO("HCI_LE_GENERATE_DHKEY_COMPLETE_EVENT");
                le_dhkey_complete(packet + 3, size - 3);
                break;
            }
            break;
        }
    }
}

void hci_core_init(void)
{
    mesh_hci_init();
}

#endif /* ADAPTATION_COMPILE_DEBUG */
