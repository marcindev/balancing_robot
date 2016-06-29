#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>

struct Settings
{
    struct Pid
    {
        enum PidParam
        {
            proportional = 0,
            integral,
            derivative
        };

        struct PidEntry
        {


            PidEntry() : min(0.0), max(10.0), interval(0.1) { }
            float min, max, interval;
        };

        Pid();

        std::map<PidParam, PidEntry> pidSettings;
    };

    Pid pid;

};

#endif // SETTINGS_H
