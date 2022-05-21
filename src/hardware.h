#pragma once

#include <Button2.h>
#include <S31CSE7766.h>
#include <Timer.h>
#include "changeCallback.h"
#include <memory>

class hardware
{
public:
    void begin();
    void loop();

    void setRelayState(bool on);
    bool isRelayOn();

    double getVoltage() const { return voltage; }
    double getCurrent() const { return current; }
    double getActivePower() const { return activePower; }
    double getApparentPower() const { return apparentPower; }
    double getEnergy() const { return energy; }
    double getPowerFactor() const { return powerFactor; }

    static hardware instance;

    void setLedDefaultState();

    changeCallBack relayChangeCallback;
    changeCallBack voltageChangeCallback;
    changeCallBack currentChangeCallback;
    changeCallBack activePowerChangeCallback;
    changeCallBack apparentPowerChangeCallback;
    changeCallBack energyChangeCallback;
    changeCallBack powerFactorChangeCallback;

    static const int VoltageRoundPlaces = 0;
    static const int CurrentRoundPlaces = 3;
    static const int ActivePowerRoundPlaces = 0;
    static const int ApparentPowerRoundPlaces = 0;
    static const int EnergyPowerRoundPlaces = 2;
    static const int PowerFactorRoundPaces = 2;

private:
    enum class LedState : uint8_t
    {
        On,
        Off,
        PowerOver,
    };

    const int ButtonPin = 0; // Sonoff On/Off button
    const int RelayPin = 12; // Sonoff relay
    const int LedPin = 13;   // Sonoff LED

    double voltage{0};       // V
    double current{0};       // A
    double activePower{0};   // W
    double apparentPower{0}; // VA
    double energy{0};        // KWh
    double powerFactor{0};
    
    LedState ledState;
    uint64_t ledOldChange{0};
    Button2 button;

    std::unique_ptr<CSE7766> powerChip;
    uint64_t lastRtcEnergySaved{0};
    StartStopTimer overPowerTimer;

    static float roundPlaces(double val, int places);
    void buttonClicked(Button2 &btn);
    void powerChipUpdate();
    void setLedState(LedState ledState);
    void ledUpdate();

    typedef double (CSE7766::*getDataFtn)() const;

    void checkChanged(getDataFtn getFtn, double &existingValue,
                      uint8_t decimalPaces, const changeCallBack &changeCallback);
};