SECTIONS
{
    .data : ALIGN(4)
    {
        btstack_data_start = .;
        *(.bt_stack_data)
        *(.ble_db_data)
        *(.ble_sm_data)
        *(.ble_att_data)
        *(.ble_gatt_data)

		/*mesh*/
        BTSTACK_LE_HOST_MESH_DATA_START = .;
		. = (. +3) & ~ 3;
        _net_buf_pool_list = .;
        *(._net_buf_pool.static.*)

        *(.ble_mesh_data)
        *(.ble_mesh_tinycrypt_data)
        BTSTACK_LE_HOST_MESH_DATA_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_DATA_START);

        _fmna_data_begin = .;
        *(.fmna_stack_data)
        *(.fmna_plaform_data)
        *(.fmna_crypto_data)
        _fmna_data_end = .;
        btstack_fmna_data_size  = _fmna_data_end - _fmna_data_begin;
        . = ALIGN(4);

        btstack_data_end = .;

        . = ALIGN(4);
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        btstack_bss_start = .;
        *(.bt_stack_bss)
        *(.ble_db_bss)
        *(.ble_sm_bss)
        *(.ble_att_bss)
        *(.ble_gatt_bss)
        *(.btstack_pool)

		/*mesh*/
        BTSTACK_LE_HOST_MESH_BSS_START = .;
        *(.ble_mesh_bss)
        *(.ble_mesh_tinycrypt_bss)
        BTSTACK_LE_HOST_MESH_BSS_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_BSS_START);

        . = ALIGN(4);
        _fmna_bss_begin = .;
        *(.fmna_stack_bss)
        *(.fmna_plaform_bss)
        *(.fmna_crypto_bss)
        _fmna_bss_end = .;
        btstack_fmna_bss_size   = _fmna_bss_end - _fmna_bss_begin;
        . = ALIGN(4);


        btstack_bss_end = .;

        . = ALIGN(4);
    } > ram0

    .text : ALIGN(4)
    {
        btstack_code_start = .;

        . = ALIGN(4);

        a2dp_source_media_codec_begin = .;
        KEEP(*(.a2dp_source_media_codec))
            a2dp_source_media_codec_end = .;
        a2dp_sink_media_probe_begin = .;
        KEEP(*(.a2dp_sink_media_probe))
            a2dp_sink_media_probe_end = .;

        a2dp_sink_media_codec_begin = .;
        KEEP(*(.a2dp_sink_media_codec))
            a2dp_sink_media_codec_end = .;

        a2dp_event_handler_begin = .;
        KEEP(*(.a2dp_event_handler))
            a2dp_event_handler_end = .;

        sdp_record_item_begin = .;
        KEEP(*(.sdp_record_item))
            sdp_record_item_end = .;

        bt_sleep_begin = .;
        KEEP(*(.bt_sleep))
            bt_sleep_end = .;

		*(.bt_stack_const)
		*(.bt_stack_code)
        *(.ble_db_const)
        *(.ble_db_code)
        *(.ble_sm_const)
        *(.ble_sm_code)
        *(.ble_att_const)
        *(.ble_att_code)
        *(.ble_gatt_const)
        *(.ble_gatt_code)

		/*mesh*/
        BTSTACK_LE_HOST_MESH_CODE_START = .;
        *(.ble_mesh_code)
        *(.ble_mesh_tinycrypt_code)

        *(.ble_mesh_const)
        *(.ble_mesh_tinycrypt_const)
        BTSTACK_LE_HOST_MESH_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_CODE_START);

        _fmna_const_begin = .;
        *(.fmna_stack_const)
        *(.fmna_plaform_const)
        *(.fmna_crypto_const)
        _fmna_const_end = .;
        . = ALIGN(4);
        _fmna_code_begin = .;
        *(.fmna_stack_code)
        *(.fmna_plaform_code)
        *(.fmna_crypto_code)
        _fmna_code_end = .;
        btstack_fmna_const_size = _fmna_const_end - _fmna_const_begin;
        btstack_fmna_code_size  = _fmna_code_end - _fmna_code_begin;
        . = ALIGN(4);


        btstack_code_end = .;
		. = ALIGN(4);
    } > code0
}

BTSTACK_LE_HOST_MESH_RAM_TOTAL = BTSTACK_LE_HOST_MESH_DATA_SIZE + BTSTACK_LE_HOST_MESH_BSS_SIZE;
BTSTACK_LE_HOST_MESH_FLASH_TOTAL = BTSTACK_LE_HOST_MESH_CODE_SIZE;

BTSTACK_CODE_SIZE = (btstack_code_end - btstack_code_start) + (btstack_data_end - btstack_data_start);

