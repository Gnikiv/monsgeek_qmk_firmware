// Akko Widget — batería por VIA (canal "custom" = 0)
// Copyright 2026 — GPL-2.0-or-later (igual que el firmware QMK del M1 V5)
//
// Añade un manejador de valores custom de VIA al MonsGeek M1 V5 VIA:
//   id_custom_get_value, canal 0, value_id 1 -> [porcentaje de batería 0-100]
//   id_custom_get_value, canal 0, value_id 2 -> [cargando 0/1, carga completa 0/1]
//   id_custom_get_value, canal 0, value_id 3 -> [modo: 0=USB, 1-5=BT1-5, 6=2.4G]
//   id_custom_get_value, canal 0, value_id 4 -> [0x41 'A', 0x57 'W'] firma del parche
//   id_custom_set_value, canal 0, value_id 5 -> [1] enciende la alimentación de los LED
//                                               (LED_POWER_EN_PIN) y activa rgb_matrix
//
// Uso: copiar a keyboards/monsgeek/m1_v5/m1_v5_uk/ (o m1_v5_us/) y añadir
//      SRC += akko_widget_via.c   en keymaps/default/rules.mk

#include QMK_KEYBOARD_H

#ifdef VIA_ENABLE
#    include "via.h"
#    ifdef WIRELESS_ENABLE
#        include "wireless.h"
#    endif

extern bool charging_state;
extern bool bat_full_flag;

enum akko_widget_value_id {
    AW_ID_BATTERY  = 1,
    AW_ID_CHARGING = 2,
    AW_ID_DEVS     = 3,
    AW_ID_SIGNATURE = 4,
    AW_ID_LED_POWER = 5,
};

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data... ]
    uint8_t *command_id = &(data[0]);
    uint8_t *channel_id = &(data[1]);
    uint8_t *value_id   = &(data[2]);
    uint8_t *value_data = &(data[3]);

    if (*channel_id != id_custom_channel) {
        *command_id = id_unhandled;
        return;
    }

    switch (*command_id) {
        case id_custom_get_value:
            switch (*value_id) {
                case AW_ID_BATTERY:
#    ifdef WIRELESS_ENABLE
                    value_data[0] = *md_getp_bat();
#    else
                    value_data[0] = 0xFF;
#    endif
                    break;
                case AW_ID_CHARGING:
                    value_data[0] = charging_state ? 1 : 0;
                    value_data[1] = bat_full_flag ? 1 : 0;
                    break;
                case AW_ID_DEVS:
#    ifdef WIRELESS_ENABLE
                    value_data[0] = wireless_get_current_devs();
#    else
                    value_data[0] = 0;
#    endif
                    break;
                case AW_ID_SIGNATURE:
                    value_data[0] = 'A';
                    value_data[1] = 'W';
                    break;
                default:
                    *command_id = id_unhandled;
                    break;
            }
            break;
        case id_custom_set_value:
            if (*value_id == AW_ID_LED_POWER) {
#    ifdef LED_POWER_EN_PIN
                if (value_data[0]) {
                    gpio_write_pin_high(LED_POWER_EN_PIN);
#        ifdef HS_LED_BOOSTING_PIN
                    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#        endif
                    rgb_matrix_enable_noeeprom();
                } else {
                    gpio_write_pin_low(LED_POWER_EN_PIN);
                }
#    endif
            } else {
                *command_id = id_unhandled;
            }
            break;
        case id_custom_save:
            break;
        default:
            *command_id = id_unhandled;
            break;
    }
}
#endif // VIA_ENABLE
