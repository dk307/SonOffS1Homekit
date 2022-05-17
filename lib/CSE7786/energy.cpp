#include "energy.h"

// Base units
// TODO: implement through a single class and allow direct access to the ::value

KWh::KWh() : value(0)
{
}

KWh::KWh(uint32_t value) : value(value)
{
}

Ws::Ws() : value(0)
{
}

Ws::Ws(uint32_t value) : value(value)
{
}

// Generic storage. Most of the time we init this on boot with both members or start at 0 and increment with watt-second

Energy::Energy(KWh kwh, Ws ws) : kwh(kwh)
{
    *this += ws;
}

Energy::Energy(KWh kwh) : kwh(kwh),
                          ws()
{
}

Energy::Energy(Ws ws) : kwh()
{
    *this += ws;
}

Energy::Energy(double raw)
{
    *this = raw;
}

Energy &Energy::operator=(double raw)
{
    double _wh;
    kwh = modf(raw, &_wh);
    ws = _wh * 3600.0;
    return *this;
}

Energy &Energy::operator+=(Ws _ws)
{
    while (_ws.value >= KwhMultiplier)
    {
        _ws.value -= KwhMultiplier;
        ++kwh.value;
    }
    ws.value += _ws.value;
    while (ws.value >= KwhMultiplier)
    {
        ws.value -= KwhMultiplier;
        ++kwh.value;
    }
    return *this;
}

Energy Energy::operator+(Ws watt_s)
{
    Energy result(*this);
    result += watt_s;
    return result;
}

Energy::operator bool() const
{
    return (kwh.value > 0) && (ws.value > 0);
}

Ws Energy::asWs() const
{
    auto _kwh = kwh.value;
    while (_kwh >= KwhLimit)
    {
        _kwh -= KwhLimit;
    }

    return (_kwh * KwhMultiplier) + ws.value;
}

double Energy::asDouble() const
{
    return (double)kwh.value + ((double)ws.value / (double)KwhMultiplier);
}

// Format is `<kwh>+<ws>`
// Value without `+` is treated as `<ws>`
// (internally, we *can* overflow ws that is converted into kwh)
String Energy::asString() const
{
    String out;
    out.reserve(32);

    out += kwh.value;
    out += '+';
    out += ws.value;

    return out;
}

void Energy::reset()
{
    kwh.value = 0;
    ws.value = 0;
}
