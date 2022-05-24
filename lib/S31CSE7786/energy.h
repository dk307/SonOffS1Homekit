#pragma once

#include <Arduino.h>

// Base units are 32 bit since we are the fastest with them.

struct Ws
{
    Ws();
    Ws(uint32_t);
    uint32_t value;
};

struct Wh
{
    Wh();
    Wh(Ws);
    Wh(uint32_t);
    uint32_t value;
};

struct KWh
{
    KWh();
    KWh(Ws);
    KWh(Wh);
    KWh(uint32_t);
    uint32_t value;
};

struct Energy
{
    constexpr static uint32_t KwhMultiplier = 3600000ul;
    constexpr static uint32_t KwhLimit = ((1ul << 31ul) / KwhMultiplier);

    Energy() = default;

    // TODO: while we accept ws >= the kwh conversion limit,
    // should this be dealt with on the unit level?
    explicit Energy(double);
    explicit Energy(KWh, Ws);
    explicit Energy(KWh);
    explicit Energy(Wh);
    explicit Energy(Ws);

    // Sets internal counters to zero
    void reset();

    // Check whether we have *any* energy recorded. Can be zero:
    // - on cold boot
    // - on overflow
    // - when we call `reset()`
    explicit operator bool() const;

    // Generic conversion as-is
    double asDouble() const;
    String asString() const;

    // Convert back to input unit, with overflow mechanics when kwh values goes over 32 bit
    Ws asWs() const;

    // Generic sensors output energy in joules / watt-second
    Energy &operator+=(Ws);
    Energy operator+(Ws);

    // But sometimes we want to accept asDouble() value back
    Energy &operator=(double);

    // We are storing a kind-of integral and fractional parts
    // Using watt-second to avoid loosing precision, we don't expect these to be accessed directly
    KWh kwh;
    Ws ws;
};