#pragma once

#include "Button2.h"
#include "CSE7766.h"
#include "changeCallback.h"

enum class LedState
{
    On,
    Off,
    FastBlink,
    SlowBlink
};

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

    void setLedState(LedState ledState) {}

    static hardware instance;

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
    const int ButtonPin = 0; // Sonoff On/Off button
    const int RelayPin = 12; // Sonoff relay
    const int LedPin = 13;   // Sonoff LED
    const int CSE7766Rx = 1;

    double voltage{0};       // V
    double current{0};       // A
    double activePower{0};   // W
    double apparentPower{0}; // VA
    double energy{0}; // KWh
    double powerFactor{0};

    Button2 button;
    CSE7766 powerChip;
    uint64_t lastRead{0};
    uint64_t lastEnergySaved{0};

    static float roundPlaces(double val, int places);
    void buttonClicked(Button2 &btn);
    void powerChipUpdate();

    typedef double (CSE7766::*getDataFtn)();

    void checkChanged(getDataFtn getFtn, double &existingValue,
                      uint8_t decimalPaces, const changeCallBack &changeCallback);
};