#include <Arduino.h>

#include <LittleFS.h>
#include <base64.h> // from esphap
#include <MD5Builder.h>
#include <user_interface.h>
#include "logging.h"

#include "configManager.h"

// Base address of USER RTC memory
// https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map#memmory-mapped-io-registers
#define RTCMEM_ADDR_BASE (0x60001200)
#define RTCMEM_OFFSET 32u
#define RTCMEM_ADDR (RTCMEM_ADDR_BASE + (RTCMEM_OFFSET * 4u))
#define RTCMEM_BLOCKS 96u
#define RTCMEM_MAGIC 0xA4535574 // Change this when modifying RtcmemData

static const char RtcConfigFilePath[] PROGMEM = "/rtc.bin";
static const char ConfigFilePath[] PROGMEM = "/conf.json";
static const char ConfigChecksumFilePath[] PROGMEM = "/confchksum.json";
static const char HostNameId[] PROGMEM = "hostname";
static const char WebUserNameId[] PROGMEM = "webusername";
static const char WebPasswordId[] PROGMEM = "webpassword";
static const char HomeKitPairDataId[] PROGMEM = "homekitpairdata";

static const char MaxPowerId[] PROGMEM = "maxpower";
static const char MaxPowerHoldId[] PROGMEM = "maxpowerhold";
static const char ReportSendIntervalId[] PROGMEM = "reportsendinterval";
static const char WattageThresholdId[] PROGMEM = "wattagethreshold";
static const char WattagePercentThresholdId[] PROGMEM = "wattagepercentthreshold";
static const char PowerPercentThresholdId[] PROGMEM = "powerpercentthreshold";
static const char CurrentCalibrationRatioId[] PROGMEM = "currentcalibrationratio";
static const char VoltageCalibrationRatioId[] PROGMEM = "voltagecalibrationratio";
static const char PowerCalibrationRatioId[] PROGMEM = "powercalibrationratio";

config __attribute__((init_priority(101))) config::instance;

template <class... T>
String config::md5Hash(T &&...data)
{
    MD5Builder hashBuilder;
    hashBuilder.begin();
    hashBuilder.add(data...);
    hashBuilder.calculate();
    return hashBuilder.toString();
}

template <class... T>
size_t config::writeToFile(const String &fileName, T &&...contents)
{
    File file = LittleFS.open(fileName, "w");
    if (!file)
    {
        return 0;
    }

    const auto bytesWritten = file.write(contents...);
    file.close();
    return bytesWritten;
}

config::config() : Rtcmem(reinterpret_cast<volatile config::RtcmemData *>(RTCMEM_ADDR))
{
}

void config::erase()
{
    Rtcmem->magic = 0;
    LittleFS.remove(FPSTR(RtcConfigFilePath));
    LittleFS.remove(FPSTR(ConfigChecksumFilePath));
    LittleFS.remove(FPSTR(ConfigFilePath));
}

bool config::begin()
{
    static_assert(sizeof(config::RtcmemData) <= (RTCMEM_BLOCKS * 4u), "RTCMEM struct is too big");

    rtcmemSetup();

    const auto configData = readFile(FPSTR(ConfigFilePath));

    if (configData.isEmpty())
    {
        LOG_INFO(F("No stored config found"));
        reset();
        return false;
    }

    DynamicJsonDocument jsonDocument(2048);
    if (!deserializeToJson(configData.c_str(), jsonDocument))
    {
        reset();
        return false;
    }

    // read checksum from file
    const auto readChecksum = readFile(FPSTR(ConfigChecksumFilePath));
    const auto checksum = md5Hash(configData);

    if (!checksum.equalsIgnoreCase(readChecksum))
    {
        LOG_ERROR(F("Config data checksum mismatch"));
        reset();
        return false;
    }

    data.hostName = jsonDocument[FPSTR(HostNameId)].as<String>();
    data.webUserName = jsonDocument[FPSTR(WebUserNameId)].as<String>();
    data.webPassword = jsonDocument[FPSTR(WebPasswordId)].as<String>();

    data.reportSendInterval = jsonDocument[FPSTR(ReportSendIntervalId)].as<uint64_t>();
    data.wattageThreshold = jsonDocument[FPSTR(WattageThresholdId)].as<uint16_t>();
    data.wattagePercentThreshold = jsonDocument[FPSTR(WattagePercentThresholdId)].as<uint8_t>();

    data.maxPower = jsonDocument[FPSTR(MaxPowerId)].as<uint16_t>();
    data.maxPowerHold = jsonDocument[FPSTR(MaxPowerHoldId)].as<uint16_t>();

    data.voltageCalibrationRatio = jsonDocument[FPSTR(VoltageCalibrationRatioId)].as<float>();
    data.currentCalibrationRatio = jsonDocument[FPSTR(CurrentCalibrationRatioId)].as<float>();
    data.powerCalibrationRatio = jsonDocument[FPSTR(PowerCalibrationRatioId)].as<float>();

    const auto encodedHomeKitData = jsonDocument[FPSTR(HomeKitPairDataId)].as<String>();

    const auto size = base64_decoded_size(reinterpret_cast<const unsigned char *>(encodedHomeKitData.c_str()),
                                          encodedHomeKitData.length());
    data.homeKitPairData.resize(size);

    base64_decode_(reinterpret_cast<const unsigned char *>(encodedHomeKitData.c_str()),
                   encodedHomeKitData.length(), data.homeKitPairData.data());

    data.homeKitPairData.shrink_to_fit();

    LOG_DEBUG(F("Loaded Config from file"));

    return true;
}

void config::reset()
{
    data.setDefaults();
    requestSave = true;
}

void config::save()
{
    LOG_INFO(F("Saving configuration"));

    DynamicJsonDocument jsonDocument(2048);

    jsonDocument[FPSTR(HostNameId)] = data.hostName.c_str();
    jsonDocument[FPSTR(WebUserNameId)] = data.webUserName.c_str();
    jsonDocument[FPSTR(WebPasswordId)] = data.webPassword.c_str();

    const auto requiredSize = base64_encoded_size(data.homeKitPairData.data(), data.homeKitPairData.size());
    const auto encodedData = std::make_unique<unsigned char[]>(requiredSize + 1);
    base64_encode_(data.homeKitPairData.data(), data.homeKitPairData.size(), encodedData.get());

    jsonDocument[FPSTR(HomeKitPairDataId)] = encodedData.get();
    jsonDocument[FPSTR(ReportSendIntervalId)] = data.reportSendInterval;

    jsonDocument[FPSTR(MaxPowerId)] = data.maxPower;
    jsonDocument[FPSTR(MaxPowerHoldId)] = data.maxPowerHold;

    jsonDocument[FPSTR(WattageThresholdId)] = data.wattageThreshold;
    jsonDocument[FPSTR(WattagePercentThresholdId)] = data.wattagePercentThreshold;

    jsonDocument[FPSTR(VoltageCalibrationRatioId)] = data.voltageCalibrationRatio;
    jsonDocument[FPSTR(CurrentCalibrationRatioId)] = data.currentCalibrationRatio;
    jsonDocument[FPSTR(PowerCalibrationRatioId)] = data.powerCalibrationRatio;

    String json;
    serializeJson(jsonDocument, json);

    if (writeToFile(FPSTR(ConfigFilePath), json.c_str(), json.length()) == json.length())
    {
        const auto checksum = md5Hash(json);
        if (writeToFile(FPSTR(ConfigChecksumFilePath), checksum.c_str(), checksum.length()) != checksum.length())
        {
            LOG_ERROR(F("Failed to write config checksum file"));
        }
    }
    else
    {
        LOG_ERROR(F("Failed to write config file"));
    }

    LOG_INFO(F("Saving Configuration done"));
    callChangeListeners();
}

void config::loop()
{
    const auto now = millis();
    const int rtcSaveInterval = 60 * 60 * 1000; // 1hr
    if ((now - lastRtcSavedToFash >= rtcSaveInterval) || requestSave)
    {
        tryWriteRtcMemoryToFlash();
        lastRtcSavedToFash = now;
    }

    if (requestSave)
    {
        requestSave = false;
        save();
    }
}

String config::readFile(const String &fileName)
{
    File file = LittleFS.open(fileName, "r");
    if (!file)
    {
        return String();
    }

    const auto json = file.readString();
    file.close();
    return json;
}

String config::getAllConfigAsJson()
{
    loop(); // save if needed
    return readFile(FPSTR(ConfigFilePath));
}

bool config::restoreAllConfigAsJson(const std::vector<uint8_t> &json, const String &hashMd5)
{
    DynamicJsonDocument jsonDocument(2048);
    if (!deserializeToJson(json, jsonDocument))
    {
        return false;
    }

    const auto expectedMd5 = md5Hash(json.data(), json.size());
    if (!expectedMd5.equalsIgnoreCase(hashMd5))
    {
        LOG_ERROR(F("Uploaded Md5 for config does not match. File md5:") << expectedMd5);
        return false;
    }

    if (writeToFile(FPSTR(ConfigFilePath), json.data(), json.size()) != json.size())
    {
        return false;
    }

    if (writeToFile(FPSTR(ConfigChecksumFilePath), hashMd5.c_str(), hashMd5.length()) != hashMd5.length())
    {
        return false;
    }
    return true;
}

template <class T>
bool config::deserializeToJson(const T &data, DynamicJsonDocument &jsonDocument)
{
    DeserializationError error = deserializeJson(jsonDocument, data);

    // Test if parsing succeeds.
    if (error)
    {
        LOG_ERROR(F("deserializeJson for config failed: ") << error.f_str());
        return false;
    }
    return true;
}

void config::setRelayState(bool state)
{
    Rtcmem->relay = state;
    tryWriteRtcMemoryToFlash();
}

bool config::getRelayState() const
{
    return Rtcmem->relay;
}

void config::setEnergyState(const Energy &state)
{
    Rtcmem->energy.kwh = state.kwh.value;
    Rtcmem->energy.ws = state.ws.value;
}

Energy config::getEnergyState() const
{
    return Energy(Rtcmem->energy.kwh, Rtcmem->energy.ws);
}

void config::rtcmemSetup()
{
    bool rtcmemStatus = false;
    const auto resetInfo = ESP.getResetInfoPtr();
    switch (resetInfo->reason)
    {
    case REASON_EXT_SYS_RST:
    case REASON_WDT_RST:
    case REASON_EXCEPTION_RST:
        break;

    case REASON_DEFAULT_RST:
        rtcmemStatus = tryReadRtcMemoryFromFlash();
        break;
    default:
        rtcmemStatus = RTCMEM_MAGIC == Rtcmem->magic;
        break;
    }

    if (!rtcmemStatus)
    {
        auto ptr = reinterpret_cast<volatile uint32_t *>(RTCMEM_ADDR);
        const auto end = ptr + RTCMEM_BLOCKS;
        do
        {
            *ptr = 0;
        } while (++ptr != end);

        Rtcmem->magic = RTCMEM_MAGIC;
    }
    else
    {
        LOG_DEBUG(F("Using Rtc Memory values"));
        copyRtcMemory(const_cast<RtcmemData *>(Rtcmem), &lastSavedToFlash);
    }
}

void config::tryWriteRtcMemoryToFlash()
{
    RtcmemData copy;
    copyRtcMemory(const_cast<RtcmemData *>(Rtcmem), &copy);
    if (writeToFile(FPSTR(RtcConfigFilePath), reinterpret_cast<uint8_t *>(&copy), sizeof(copy)) == sizeof(copy))
    {
        LOG_INFO(F("Saved Rtc Memory to Flash"));
        copyRtcMemory(&copy, &lastSavedToFlash);
    }
}

bool config::tryReadRtcMemoryFromFlash()
{
    File f = LittleFS.open(FPSTR(RtcConfigFilePath), "r");
    if (f)
    {
        RtcmemData copy;
        if (f.size() == sizeof(copy))
        {
            const auto bytesRead = f.readBytes(reinterpret_cast<char *>(&copy), sizeof(copy));
            f.close();

            copyRtcMemory(&copy, const_cast<RtcmemData *>(Rtcmem));
            return (bytesRead == sizeof(copy)) && (Rtcmem->magic == RTCMEM_MAGIC);
        }
    }
    return false;
}

void config::copyRtcMemory(const RtcmemData *source, RtcmemData *dest)
{
    dest->magic = source->magic;
    dest->relay = source->relay;
    dest->energy.kwh = source->energy.kwh;
    dest->energy.ws = source->energy.ws;
}