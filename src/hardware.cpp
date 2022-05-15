#include "hardware.h"
#include "configManager.h"
#include "WiFiManager.h"
#include "logging.h"

#include <math.h>

hardware hardware::instance;

void hardware::begin()
{
    const auto ftn = [this]
    {
        LOG_DEBUG(F("Display refresh needed"));
    };

    config::instance.addConfigSaveCallback(ftn);

    pinMode(ButtonPin, INPUT); // on/off button
    pinMode(RelayPin, OUTPUT); // relay
    pinMode(LedPin, OUTPUT);   // led

    digitalWrite(LedPin, LOW); // always on
    digitalWrite(RelayPin, config::instance.data.relayOn ? HIGH : LOW);

    button.begin(ButtonPin);
    button.setReleasedHandler(std::bind(&hardware::buttonClicked, this, std::placeholders::_1));

    powerChip.setRX(CSE7766Rx);
    powerChip.begin();
}

void hardware::buttonClicked(Button2 &btn)
{
    // toggle
    setRelayState(!isRelayOn());
}

void hardware::setRelayState(bool on)
{
    const auto newState = on ? HIGH : LOW;
    digitalWrite(RelayPin, newState);

    config::instance.data.relayOn = newState == HIGH;
    config::instance.save();
}

bool hardware::isRelayOn()
{
    return digitalRead(RelayPin) == HIGH;
}

float hardware::roundPlaces(double val, int places)
{
    if (!isnan(val))
    {
        const auto expVal = pow(10, places);
        const auto result = float(uint64_t(expVal * val + 0.5)) / expVal;
        return result;
    }
    return val;
}

void hardware::loop()
{
    button.loop();
    powerChipUpdate();
}

void hardware::checkChanged(getDataFtn getFtn, double &existingValue,
                            uint8_t decimalPaces, const changeCallBack &changeCallback)
{
    const auto value = (powerChip.*getFtn)();
    const auto roundValue = roundPlaces(value, decimalPaces);

    if (existingValue != roundValue)
    {
        existingValue = roundValue;
        changeCallback.callChangeListeners();
    }
}

void hardware::powerChipUpdate()
{
    const auto now = millis();
    if (now - lastRead > config::instance.data.sensorsRefreshInterval)
    {
        powerChip.handle();

        checkChanged(&CSE7766::getVoltage, voltage, 1, voltageChangeCallback);
        checkChanged(&CSE7766::getCurrent, current, 4, currentChangeCallback);
        checkChanged(&CSE7766::getActivePower, activePower, 2, activePowerChangeCallback);
        checkChanged(&CSE7766::getApparentPower, apparentPower, 2, apparentPowerChangeCallback);
        checkChanged(&CSE7766::getReactivePower, reactivePower, 2, reactivePowerChangeCallback);
        checkChanged(&CSE7766::getEnergy, energy, 3, energyChangeCallback);
        checkChanged(&CSE7766::getPowerRatio, powerFactor, 3, powerFactorChangeCallback);

        lastRead = now;
    }
}
