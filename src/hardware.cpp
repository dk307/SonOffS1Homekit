#include "hardware.h"
#include "configManager.h"
#include "WiFiManager.h"
#include "logging.h"

#include <math.h>

hardware hardware::instance;

void hardware::begin()
{
    pinMode(ButtonPin, INPUT); // on/off button
    pinMode(RelayPin, OUTPUT); // relay
    pinMode(LedPin, OUTPUT);   // led

    powerChip = std::make_unique<CSE7766>(config::instance.getEnergyState(),
                                          config::instance.data.voltageCalibrationRatio,
                                          config::instance.data.currentCalibrationRatio,
                                          config::instance.data.powerCalibrationRatio);

    setLedState(LedState::On);
    digitalWrite(RelayPin, config::instance.getRelayState() ? HIGH : LOW);

    button.setReleasedHandler(std::bind(&hardware::buttonClicked, this, std::placeholders::_1));
    button.begin(ButtonPin, INPUT);

    config::instance.addConfigSaveCallback([]
                                           {
        hardware::instance.setLedDefaultState();
        hardware::instance.overPowerTimer.stop(); });
}

void hardware::buttonClicked(Button2 &btn)
{
    // toggle
    setRelayState(!isRelayOn());
}

void hardware::setRelayState(bool on)
{
    LOG_INFO("Setting Relay state to " << on);
    const auto newState = on ? HIGH : LOW;
    if (digitalRead(RelayPin) != newState)
    {
        digitalWrite(RelayPin, newState);
        config::instance.setRelayState(on);
        relayChangeCallback.callChangeListeners();
    }
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
    ledUpdate();
}

void hardware::checkChanged(getDataFtn getFtn, double &existingValue,
                            uint8_t decimalPaces, const changeCallBack &changeCallback)
{
    const auto value = (*powerChip.*getFtn)();
    const auto roundValue = roundPlaces(value, decimalPaces);

    if (existingValue != roundValue)
    {
        existingValue = roundValue;
        changeCallback.callChangeListeners();
    }
}

void hardware::powerChipUpdate()
{
    if (powerChip->handle()) // only on full packet process
    {
        checkChanged(&CSE7766::getVoltage, voltage, VoltageRoundPlaces, voltageChangeCallback);
        checkChanged(&CSE7766::getCurrent, current, CurrentRoundPlaces, currentChangeCallback);
        checkChanged(&CSE7766::getActivePower, activePower, ActivePowerRoundPlaces, activePowerChangeCallback);
        checkChanged(&CSE7766::getApparentPower, apparentPower, ApparentPowerRoundPlaces, apparentPowerChangeCallback);
        checkChanged(&CSE7766::getPowerRatio, powerFactor, PowerFactorRoundPaces, powerFactorChangeCallback);
        checkChanged(&CSE7766::getEnergyKwh, energy, EnergyPowerRoundPlaces, energyChangeCallback);

        const int MaxRtcSaveInterval = 1000; // 1s
        const auto now = millis();
        if (now - lastRtcEnergySaved > MaxRtcSaveInterval)
        {
            config::instance.setEnergyState(powerChip->getEnergy());
            lastRtcEnergySaved = now;
        }
    }

    if ((config::instance.data.maxPower != 0) && (activePower >= double(config::instance.data.maxPower)))
    {
        setLedState(LedState::PowerOver);
        const auto elapsed = overPowerTimer.startIfNotRunning();
        if (elapsed >= config::instance.data.maxPowerHold)
        {
            LOG_WARNING(F("Turning off relay because it over power limit ") << activePower
                                                                            << F("Watts  for window : ") << elapsed << " ms");
            setRelayState(false);
            overPowerTimer.stop();
        }
    }
    else
    {
        setLedDefaultState();
        overPowerTimer.stop();
    }
}

void hardware::setLedState(LedState state)
{
    ledState = state;
    switch (state)
    {
    case LedState::On:
        if (ledState != LedState::On)
        {
            digitalWrite(LedPin, LOW);
            ledState = state;
        }
        break;
    case LedState::Off:
        if (ledState != LedState::Off)
        {
            digitalWrite(LedPin, HIGH);
            ledState = state;
        }
        break;
    case LedState::PowerOver:
        ledState = state;
        break;
    };
}

void hardware::setLedDefaultState()
{
    LOG_INFO(F("Set Default Led state"));
    setLedState(LedState::On);
}

void hardware::ledUpdate()
{
    if (ledState == LedState::PowerOver)
    {
        const auto now = millis();
        if ((ledOldChange + 200) < now)
        {
            ledOldChange = now;
            const float it = random(0, 255);
            analogWrite(LedPin, it);
        }
    }
}