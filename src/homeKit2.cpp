#include <Arduino.h>

#include "homeKit2.h"
#include "configManager.h"
#include "WiFiManager.h"
#include "hardware.h"
#include "logging.h"
#include "homekit2helper.h"

#include <math.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <arduino_homekit_server.h>

homeKit2 homeKit2::instance;

extern "C" homekit_server_config_t config;

extern "C" homekit_characteristic_t chaName;
extern "C" homekit_characteristic_t chaSerial;

extern "C" homekit_characteristic_t chaOutlet;
extern "C" homekit_characteristic_t chaOutletInUse;

extern "C" homekit_characteristic_t chaVoltage;
extern "C" homekit_characteristic_t chaCurrent;
extern "C" homekit_characteristic_t chaActivePower;
extern "C" homekit_characteristic_t chaApparantPower;
extern "C" homekit_characteristic_t chaEnergy;

extern "C" homekit_characteristic_t chaWifiIPAddress;
extern "C" homekit_characteristic_t chaWifiRssi;
extern "C" homekit_characteristic_t chaReportSendInterval;
extern "C" homekit_characteristic_t chaReportSendWattsAbsolute;
extern "C" homekit_characteristic_t chaReportSendWattsPercentage;

extern "C" homekit_characteristic_t chaMaxPower;
extern "C" homekit_characteristic_t chaMaxPowerHold;

void homeKit2::begin()
{
    config.password_callback = &homeKit2::updatePassword;

    serialNumber = String(ESP.getChipId(), HEX);
    serialNumber.toUpperCase();

    localIP = WifiManager::instance.LocalIP().toString();

    config::instance.addConfigSaveCallback(std::bind(&homeKit2::onConfigChange, this));
    hardware::instance.relayChangeCallback.addConfigSaveCallback(std::bind(&homeKit2::notifyRelaychange, this));
    hardware::instance.activePowerChangeCallback.addConfigSaveCallback(std::bind(&homeKit2::checkPowerChanged, this));

    chaReportSendInterval.setter = onReportSendIntervalChange;
    chaOutlet.setter = onRelayChange;
    chaReportSendWattsAbsolute.setter = onReportWattageThresholdChange;
    chaReportSendWattsPercentage.setter = onReportWattagePercentThresholdChange;
    chaMaxPower.setter = onMaxPowerChange;
    chaMaxPowerHold.setter = onMaxPowerHoldChange;

    // update values
    updateChaValue(chaSerial, serialNumber.c_str());
    updateAccessoryName();
    updateChaValue<uint32_t>(chaReportSendInterval, config::instance.data.reportSendInterval / 1000);
    updateChaValue(chaReportSendWattsAbsolute, config::instance.data.wattageThreshold);
    updateChaValue(chaReportSendWattsPercentage, config::instance.data.wattagePercentThreshold);
    updateChaValue(chaMaxPower, config::instance.data.maxPower);
    updateChaValue<uint16_t>(chaMaxPowerHold, config::instance.data.maxPowerHold / 1000);
    updateChaValue(chaWifiIPAddress, localIP.c_str());
    updateChaValue<int>(chaWifiRssi, WifiManager::instance.RSSI());
    updateChaValue(chaOutlet, hardware::instance.isRelayOn());
    updateChaValue(chaOutletInUse, hardware::instance.anyPower());

    updateChaValue(chaVoltage, hardware::instance.getVoltage());
    updateChaValue(chaCurrent, hardware::instance.getCurrent());
    updateChaValue(chaActivePower, hardware::instance.getActivePower());
    updateChaValue(chaApparantPower, hardware::instance.getApparentPower());
    updateChaValue(chaEnergy, hardware::instance.getEnergy());

    config.on_event = onHomeKitStateChange;
    arduino_homekit_setup(&config);

    LOG_INFO(F("HomeKit Server Running"));

    // send all notifications in case they changed
    notifyConfigValueChanges();
    notifyIPAddressChange();
    notifyWifiRssiChange();
    notifyRelaychange();
    notifyOutletInUse();
    notifyPowerReport();
}

void homeKit2::onConfigChange()
{
    if ((accessoryName != config::instance.data.hostName) && (!config::instance.data.hostName.isEmpty()))
    {
        updateAccessoryName();
    }

    notifyConfigValueChanges();
}

void homeKit2::onHomeKitStateChange(homekit_event_t event)
{
    homeKit2::instance.homeKitStateChanged.callChangeListeners();
}

void homeKit2::updateAccessoryName()
{
    accessoryName = config::instance.data.hostName;
    if (accessoryName.isEmpty())
    {
        accessoryName = F("Sonoff S31");
    }
    updateChaValue(chaName, accessoryName.c_str());
} 

void homeKit2::notifyConfigValueChanges()
{
    notifyChaValue<uint32_t>(chaReportSendInterval, config::instance.data.reportSendInterval / 1000);
    notifyChaValue(chaReportSendWattsAbsolute, config::instance.data.wattageThreshold);
    notifyChaValue(chaReportSendWattsPercentage, config::instance.data.wattagePercentThreshold);
    notifyChaValue(chaMaxPower, config::instance.data.maxPower);
    notifyChaValue<uint16_t>(chaMaxPowerHold, config::instance.data.maxPowerHold / 1000);
}

void homeKit2::notifyIPAddressChange()
{
    localIP = WifiManager::instance.LocalIP().toString();
    updateChaValue(chaWifiIPAddress, localIP.c_str());
    homekit_characteristic_notify(&chaWifiIPAddress, chaWifiIPAddress.value);
}

void homeKit2::notifyWifiRssiChange()
{
    notifyChaValue<int>(chaWifiRssi, WifiManager::instance.RSSI());
}

void homeKit2::loop()
{
    const auto now = millis();
    if ((now - lastCheckedForNonEvents > config::instance.data.reportSendInterval))
    {
        if (localIP != WifiManager::instance.LocalIP().toString())
        {
            notifyIPAddressChange();
        }
        const auto delta = std::abs(chaWifiRssi.value.int_value - WifiManager::instance.RSSI());
        if (delta > 2)
        {
            notifyWifiRssiChange();
        }
        lastCheckedForNonEvents = now;
    }

    if (now - lastPowerReport > config::instance.data.reportSendInterval)
    {
        notifyPowerReport();
    }

    arduino_homekit_loop();
}

bool homeKit2::isPaired()
{
    auto server = arduino_homekit_get_running_server();
    if (server)
    {
        return server->paired;
    }
    return false;
}

void homeKit2::updatePassword(const char *password)
{
    homeKit2::instance.password = password;
}

void homeKit2::onReportSendIntervalChange(const homekit_value_t value)
{
    if (value.format == homekit_format_uint32)
    {
        updateChaValue(chaReportSendInterval, value.uint32_value);
        config::instance.data.reportSendInterval = value.uint32_value * 1000;
        config::instance.save();
    }
}

void homeKit2::onRelayChange(const homekit_value_t value)
{
    if (value.format == homekit_format_bool)
    {
        hardware::instance.setRelayState(value.bool_value);
        updateChaValue(chaOutlet, hardware::instance.isRelayOn());
    }
}

void homeKit2::onReportWattageThresholdChange(const homekit_value_t value)
{
    if (value.format == homekit_format_uint16)
    {
        updateChaValue(chaReportSendWattsAbsolute, value.uint16_value);
        config::instance.data.wattageThreshold = value.uint16_value;
        config::instance.save();
    }
}

void homeKit2::onReportWattagePercentThresholdChange(const homekit_value_t value)
{
    if (value.format == homekit_format_uint8)
    {
        updateChaValue(chaReportSendWattsPercentage, value.uint8_value);
        config::instance.data.wattagePercentThreshold = value.uint8_value;
        config::instance.save();
    }
}

void homeKit2::onMaxPowerChange(const homekit_value_t value)
{
    if (value.format == homekit_format_uint16)
    {
        updateChaValue(chaMaxPower, value.uint16_value);
        config::instance.data.maxPower = value.uint16_value;
        config::instance.save();
    }
}

void homeKit2::onMaxPowerHoldChange(const homekit_value_t value)
{
    if (value.format == homekit_format_uint16)
    {
        updateChaValue(chaMaxPowerHold, value.uint16_value);
        config::instance.data.maxPowerHold = value.uint16_value * 1000;
        config::instance.save();
    }
}

void homeKit2::notifyRelaychange()
{
    notifyChaValue(chaOutlet, hardware::instance.isRelayOn());
}

void homeKit2::checkPowerChanged()
{
    notifyOutletInUse();

    const auto prevValue = chaActivePower.value.float_value;
    const auto powerNow = hardware::instance.getActivePower();

    const auto changeInPower = std::abs(prevValue - powerNow);
    const bool shouldSend = (changeInPower >= config::instance.data.wattageThreshold) ||
                            (changeInPower * 100 >= prevValue * config::instance.data.wattageThreshold);

    if (shouldSend)
    {
        notifyPowerReport();
    }
}

void homeKit2::notifyOutletInUse()
{
    const bool currentOutletInUse = hardware::instance.getActivePower() > 0;
    notifyChaValue(chaOutletInUse, currentOutletInUse);
}

void homeKit2::notifyPowerReport()
{
    lastPowerReport = millis();
    notifyChaValue<double>(chaVoltage, hardware::instance.getVoltage());
    notifyChaValue<double>(chaCurrent, hardware::instance.getCurrent());
    notifyChaValue<double>(chaActivePower, hardware::instance.getActivePower());
    notifyChaValue<double>(chaApparantPower, hardware::instance.getApparentPower());
    notifyChaValue<double>(chaEnergy, hardware::instance.getEnergy());
}

int homeKit2::getConnectedClientsCount()
{
    return arduino_homekit_connected_clients_count();
}

///////////////////////////////////////////////////////////

bool read_storage(uint32 srcAddress, byte *desAddress, uint32 size)
{
    LOG_TRACE(F("Reading HomeKit data from ") << srcAddress << F(" size ") << size);
    const auto &data = config::instance.data.homeKitPairData;
    if (data.size() < (srcAddress + size))
    {
        LOG_TRACE(F("Returning empty for ") << srcAddress << F(" size ") << size);
        memset(desAddress, 0xFF, size);
    }
    else
    {
        memcpy(desAddress, data.data() + srcAddress, size);
    }
    return true;
}

bool write_storage(uint32 desAddress, byte *srcAddress, uint32 size)
{
    LOG_TRACE(F("Writing HomeKit data at ") << desAddress << F(" size ") << size);
    auto &data = config::instance.data.homeKitPairData;
    if ((desAddress + size) > data.size())
    {
        data.resize(desAddress + size);
    }

    memcpy(data.data() + desAddress, srcAddress, size);
    config::instance.save();
    return true;
}

bool reset_storage()
{
    config::instance.data.homeKitPairData.resize(0);
    config::instance.save();
    return true;
}
