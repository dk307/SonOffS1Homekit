#pragma once
#include <Arduino.h>

class StartStopTimer
{
private:
    enum class status_t
    {
        STOPPED,
        RUNNING
    };

public:
    uint32_t startIfNotRunning()
    {
        const auto now = millis();
        if (status != status_t::RUNNING)
        {
            started = now;  
            status = status_t::RUNNING;
            return 0;
        }
        else
        {
            return now - started;
        }
    }

    void stop()
    {
        started = 0;
        status = status_t::STOPPED;
    }

private:
    uint32_t started{0};
    status_t status{status_t::STOPPED};
};