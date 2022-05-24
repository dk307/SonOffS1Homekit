#pragma once

#include <homekit/homekit.h>
#include <math.h>

template <class T>
T updateChaValue(homekit_characteristic_t &cha, const T &value);

template <>
double updateChaValue<double>(homekit_characteristic_t &cha, const double &value)
{
    const auto prevValue = cha.value.float_value;
    if (!isnan(value))
    {
        cha.value.is_null = false;
        cha.value.float_value = value;
    }
    else
    {
        cha.value.is_null = true;
    }
    return prevValue;
}

template <>
uint64_t updateChaValue<uint64_t>(homekit_characteristic_t &cha, const uint64_t &value)
{
    const auto prevValue = cha.value.uint64_value;
    cha.value.uint64_value = value;
    return prevValue;
}

template <>
uint32_t updateChaValue<uint32_t>(homekit_characteristic_t &cha, const uint32_t &value)
{
    const auto prevValue = cha.value.uint32_value;
    cha.value.uint32_value = value;
    return prevValue;
}

template <>
uint16_t updateChaValue<uint16_t>(homekit_characteristic_t &cha, const uint16_t &value)
{
    const auto prevValue = cha.value.uint16_value;
    cha.value.uint16_value = value;
    return prevValue;
}

template <>
uint8_t updateChaValue<uint8_t>(homekit_characteristic_t &cha, const uint8_t &value)
{
    const auto prevValue = cha.value.uint8_value;
    cha.value.uint8_value = value;
    return prevValue;
}

template <>
int updateChaValue<int>(homekit_characteristic_t &cha, const int &value)
{
    const auto prevValue = cha.value.int_value;
    cha.value.int_value = value;
    return prevValue;
}

template <>
bool updateChaValue<bool>(homekit_characteristic_t &cha, const bool &value)
{
    const auto prevValue = cha.value.bool_value;
    cha.value.bool_value = value;
    return prevValue;
}

template <class T>
void notifyChaValue(homekit_characteristic_t &cha, T &&value)
{
    const auto prevValue = updateChaValue(cha, std::forward<T>(value));
    if (value != prevValue)
    {
        homekit_characteristic_notify(&cha, cha.value);
    }
}

void updateChaValue(homekit_characteristic_t &cha, const char *value)
{
    if (value)
    {
        cha.value.is_null = false;
        cha.value.string_value = const_cast<char *>(value);
    }
    else
    {
        cha.value.is_null = true;
    }
}