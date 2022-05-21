#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#define HOMEKIT_CUSTOM_UUID(value) (value "-03e9-4157-b099-54f4a4944163")
#define HOMEKIT_ELGATO_UUID(value) (value "-079E-48FF-8F27-9C2605A29F52")

#define XSTRINFGY(s) STRINFGY(s)
#define STRINFGY(s) #s

#define HOMEKIT_SERVICE_CUSTOM_WIFI HOMEKIT_CUSTOM_UUID("F0000000")
#define HOMEKIT_SERVICE_CUSTOM_SETUP HOMEKIT_CUSTOM_UUID("F0000001")
#define HOMEKIT_SERVICE_CUSTOM_REPORT HOMEKIT_CUSTOM_UUID("F0000002")

#define HOMEKIT_CHARACTERISTIC_CUSTOM_IP_ADDR HOMEKIT_CUSTOM_UUID("00000001")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_IP_ADDR(_value, ...)               \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_IP_ADDR,                               \
    .description = "Wifi IP Address",                                            \
    .format = homekit_format_string,                                             \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .value = HOMEKIT_STRING_(_value),                                            \
    ##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_WIFI_RSSI HOMEKIT_CUSTOM_UUID("00000002")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_WIFI_RSSI(_value, ...)             \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_WIFI_RSSI,                             \
    .description = "Wifi RSSI",                                                  \
    .format = homekit_format_int,                                                \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .min_value = (float[]){-100},                                                \
    .max_value = (float[]){0},                                                   \
    .min_step = (float[]){1},                                                    \
    .value = HOMEKIT_INT_(_value),                                               \
    ##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_REPORT_SEND_INTERVAL HOMEKIT_CUSTOM_UUID("00000003")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_REPORT_SEND_INTERVAL(_value, ...)                                     \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_REPORT_SEND_INTERVAL,                                                     \
    .description = "Report Send Interval",                                                                          \
    .format = homekit_format_uint64,                                                                                \
    .unit = homekit_unit_seconds,                                                                                   \
    .permissions = homekit_permissions_paired_read | homekit_permissions_paired_write | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                                                      \
    .max_value = (float[]){604800},                                                                                 \
    .min_step = (float[]){1},                                                                                       \
    .value = HOMEKIT_UINT64_(_value),                                                                               \
    ##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_MAX_POWER_HOLD HOMEKIT_CUSTOM_UUID("00000004")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_MAX_POWER_HOLD(_value, ...)                                     \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_MAX_POWER_HOLD,                                                           \
    .description = "Max Power Hold Window",                                                                         \
    .format = homekit_format_uint64,                                                                                \
    .unit = homekit_unit_seconds,                                                                                   \
    .permissions = homekit_permissions_paired_read | homekit_permissions_paired_write | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                                                      \
    .max_value = (float[]){604800},                                                                                 \
    .min_step = (float[]){1},                                                                                       \
    .value = HOMEKIT_UINT64_(_value),                                                                               \
    ##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_MAX_POWER HOMEKIT_CUSTOM_UUID("00000005")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_MAX_POWER(_value, ...)                                     \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_MAX_POWER,                                                                \
    .description = "Max Power(Watts)",                                                                              \
    .format = homekit_format_uint64,                                                                                \
    .unit = homekit_unit_seconds,                                                                                   \
    .permissions = homekit_permissions_paired_read | homekit_permissions_paired_write | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                                                      \
    .max_value = (float[]){604800},                                                                                 \
    .min_step = (float[]){1},                                                                                       \
    .value = HOMEKIT_UINT64_(_value),                                                                               \
    ##__VA_ARGS__

#define HOMEKIT_SERVICE_VOLTAGE HOMEKIT_ELGATO_UUID("E863F10A")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_VOLTAGE(_value, ...)               \
    .type = HOMEKIT_SERVICE_VOLTAGE,                                             \
    .description = "Voltage",                                                    \
    .format = homekit_format_float,                                              \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                   \
    .max_value = (float[]){1024},                                                \
    .min_step = (float[]){1},                                                    \
    .value = HOMEKIT_FLOAT_(_value),                                             \
    ##__VA_ARGS__

#define HOMEKIT_SERVICE_CURRENT HOMEKIT_ELGATO_UUID("E863F126")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_CURRENT(_value, ...)               \
    .type = HOMEKIT_SERVICE_CURRENT,                                             \
    .description = "Current",                                                    \
    .format = homekit_format_float,                                              \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                   \
    .max_value = (float[]){100},                                                 \
    .min_step = (float[]){0.001},                                                \
    .value = HOMEKIT_FLOAT_(_value),                                             \
    ##__VA_ARGS__

#define HOMEKIT_SERVICE_ACTIVEPOWER HOMEKIT_ELGATO_UUID("E863F10D")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_ACTIVEPOWER(_value, ...)           \
    .type = HOMEKIT_SERVICE_ACTIVEPOWER,                                         \
    .description = "Active Power",                                               \
    .format = homekit_format_float,                                              \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                   \
    .max_value = (float[]){5000},                                                \
    .min_step = (float[]){1},                                                    \
    .value = HOMEKIT_FLOAT_(_value),                                             \
    ##__VA_ARGS__

#define HOMEKIT_SERVICE_APPARENTPOWER HOMEKIT_ELGATO_UUID("E863F110")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_APPARENTPOWER(_value, ...)         \
    .type = HOMEKIT_SERVICE_APPARENTPOWER,                                       \
    .description = "Apparent Power",                                             \
    .format = homekit_format_float,                                              \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                   \
    .max_value = (float[]){5000},                                                \
    .min_step = (float[]){1},                                                    \
    .value = HOMEKIT_FLOAT_(_value),                                             \
    ##__VA_ARGS__

#define HOMEKIT_SERVICE_ENERGY HOMEKIT_ELGATO_UUID("E863F10C")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_ENERGY(_value, ...)                \
    .type = HOMEKIT_SERVICE_ENERGY,                                              \
    .description = "Energy",                                                     \
    .format = homekit_format_float,                                              \
    .permissions = homekit_permissions_paired_read | homekit_permissions_notify, \
    .min_value = (float[]){0},                                                   \
    .max_value = (float[]){999999},                                              \
    .min_step = (float[]){0.001},                                                \
    .value = HOMEKIT_FLOAT_(_value),                                             \
    ##__VA_ARGS__

homekit_characteristic_t chaVoltage = HOMEKIT_CHARACTERISTIC_(CUSTOM_VOLTAGE, 0, .id = 201);
homekit_characteristic_t chaCurrent = HOMEKIT_CHARACTERISTIC_(CUSTOM_CURRENT, 0, .id = 202);
homekit_characteristic_t chaActivePower = HOMEKIT_CHARACTERISTIC_(CUSTOM_ACTIVEPOWER, 0, .id = 203);
homekit_characteristic_t chaApparantPower = HOMEKIT_CHARACTERISTIC_(CUSTOM_APPARENTPOWER, 0, .id = 204);
homekit_characteristic_t chaEnergy = HOMEKIT_CHARACTERISTIC_(CUSTOM_ENERGY, 0, .id = 205);

homekit_characteristic_t chaWifiIPAddress = HOMEKIT_CHARACTERISTIC_(CUSTOM_IP_ADDR, "", .id = 400);
homekit_characteristic_t chaWifiRssi = HOMEKIT_CHARACTERISTIC_(CUSTOM_WIFI_RSSI, 0, .id = 401);
homekit_characteristic_t chaReportSendInterval = HOMEKIT_CHARACTERISTIC_(CUSTOM_REPORT_SEND_INTERVAL, 0, .id = 500);
homekit_characteristic_t chaMaxPower = HOMEKIT_CHARACTERISTIC_(CUSTOM_MAX_POWER, 0, .id = 501);
homekit_characteristic_t chaMaxPowerHold = HOMEKIT_CHARACTERISTIC_(CUSTOM_MAX_POWER_HOLD, 0, .id = 502);

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_sensor, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .id=1, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Sonoff S31", .id=100),
            HOMEKIT_CHARACTERISTIC(MODEL, "Sonoff S31", .id=101),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, NULL, .id=102),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, XSTRINFGY(VERSION), .id=103),
            NULL
        }),

        HOMEKIT_SERVICE(CUSTOM_REPORT, .id=2, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &chaVoltage,
            &chaCurrent,
            &chaActivePower,
            &chaApparantPower,
            &chaEnergy,
            NULL
        }),
		
        HOMEKIT_SERVICE(CUSTOM_WIFI, .id=4, .characteristics=(homekit_characteristic_t*[]) {
            &chaWifiRssi,
            &chaWifiIPAddress,
            NULL
        }),
        
        HOMEKIT_SERVICE(CUSTOM_SETUP, .id=5, .characteristics=(homekit_characteristic_t*[]) {
            &chaReportSendInterval,
            &chaMaxPower,
            &chaMaxPowerHold,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
        .category = homekit_accessory_category_sensor,
		.accessories = accessories,
};