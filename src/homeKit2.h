#pragma once

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "changeCallback.h"

class homeKit2
{
public:
    void begin();
    void loop();
    bool isPaired();

    static homeKit2 instance;

    const String &getPassword()
    {
        return password;
    }
    int getConnectedClientsCount();

    changeCallBack homeKitStateChanged;

private:
    homeKit2(){};

    static void updatePassword(const char *password);

    void notifyConfigValueChanges();
    void notifyIPAddressChange();
    void notifyWifiRssiChange();
    void updateAccessoryName();

    void onConfigChange();
    static void onReportSendIntervalChange(const homekit_value_t);
    static void onReportWattageThresholdChange(const homekit_value_t);
    static void onReportWattagePercentThresholdChange(const homekit_value_t);
    static void onMaxPowerChange(const homekit_value_t);
    static void onMaxPowerHoldChange(const homekit_value_t);

    static void onHomeKitStateChange(homekit_event_t event);

    static void onRelayChange(const homekit_value_t);
    void notifyRelaychange();
    void notifyOutletInUse();

    void notifyPowerReport();

    void checkPowerChanged();

    String accessoryName;
    String password;
    String serialNumber;
    String localIP;

    uint64_t lastCheckedForNonEvents{0};
    uint64_t lastPowerReport{0};
};
