#include "hardware.h"
#include "configManager.h"
#include "WiFiManager.h"
#include "logging.h"
#include "operations.h"
#include "homeKit2.h"

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
    button.setLongClickHandler(std::bind(&hardware::buttonLogPressed, this, std::placeholders::_1));
    button.begin(ButtonPin, INPUT);

    homeKit2::instance.homeKitStateChanged.addConfigSaveCallback([this]
                                                                 { setLedDefaultState(); });
}

void hardware::buttonClicked(Button2 &btn)
{
    // toggle
    setRelayState(!isRelayOn());
}

void hardware::buttonLogPressed(Button2 &btn)
{
    const auto time = btn.wasPressedFor();

    if (time >= 10000)
    {
        operations::instance.factoryReset();
    }
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
        overPowerTimer.stop();
    }
}

void hardware::setLedState(LedState state)
{
    switch (state)
    {
    case LedState::On:
        digitalWrite(LedPin, LOW);
        break;
    case LedState::Off:
        digitalWrite(LedPin, HIGH);
        break;
    };
}

void hardware::setLedDefaultState()
{
    setLedState(homeKit2::instance.getConnectedClientsCount() > 0 ? LedState::On : LedState::Off);
}
