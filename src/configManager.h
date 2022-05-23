#pragma once
#include "changeCallBack.h"
#include <ArduinoJson.h>
#include <Energy.h>

#include <memory>

class DataStorage
{
public:
    void read(uint32_t srcAddress, uint32_t *desAddress, uint32_t size);
    void write(uint32_t desAddress, uint32_t *srcAddress, uint32_t size);
    void save();
};

struct configData
{
    String hostName;
    String webUserName;
    String webPassword;
    std::vector<uint8_t> homeKitPairData;
    uint64_t reportSendInterval;
    uint16_t wattageThreshold;
    uint8_t wattagePercentThreshold;
    uint16_t maxPower;
    uint64_t maxPowerHold;
    double voltageCalibrationRatio;
    double currentCalibrationRatio;
    double powerCalibrationRatio;

    configData()
    {
        setDefaults();
    }

    void setDefaults()
    {
        const auto defaultUserIDPassword = F("admin");
        hostName.clear();
        webUserName = defaultUserIDPassword;
        webPassword = defaultUserIDPassword;
        homeKitPairData.resize(0);
        reportSendInterval = 60 * 1000;
        
        wattageThreshold = 25;
        wattagePercentThreshold = 5;
        
        maxPower = 0;
        maxPowerHold = 10000;

        voltageCalibrationRatio = 1.0;
        currentCalibrationRatio = 1.0;
        powerCalibrationRatio = 1.0;
    }
};

class config : public changeCallBack
{
public:
    configData data;
    bool begin();
    void save();
    void reset();
    void loop();

    void erase();
    static config instance;

    String getAllConfigAsJson();

    void setRelayState(bool state);
    bool getRelayState() const;

    void setEnergyState(const Energy &state);
    Energy getEnergyState() const;

    // does not restore to memory, needs reboot
    bool restoreAllConfigAsJson(const std::vector<uint8_t> &json, const String &md5);

private:
    struct RtcmemEnergy
    {
        uint32_t kwh;
        uint32_t ws;
    };

    struct RtcmemData
    {
        uint32_t magic;
        uint32_t relay;
        RtcmemEnergy energy;
    };

    bool requestSave{false};
    volatile RtcmemData *Rtcmem;
    RtcmemData lastSavedToFlash;
    uint64_t lastRtcSavedToFash{0};

    config();
    static String readFile(const String &fileName);

    template <class... T>
    static String md5Hash(T &&...data);

    template <class... T>
    static size_t writeToFile(const String &fileName, T &&...contents);

    template <class T>
    bool deserializeToJson(const T &data, DynamicJsonDocument &jsonDocument);

    void rtcmemSetup();
    bool tryReadRtcMemoryFromFlash();
    void tryWriteRtcMemoryToFlash();
    static void copyRtcMemory(const RtcmemData *source, RtcmemData *dest);
};
