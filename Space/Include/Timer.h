/*
    The File Which stores the Timer Class which is used to Print The Time taken to execute 
    any Func
*/

#pragma once

#include <chrono>
#include "Log.h"

namespace Space
{
    // The Class used to Profile Time to Find Time Taken to Execute Funcs
    class Timer
    {
    public:
        Timer(const char *Name)
            : _Name(Name)
        {
            _Start = std::chrono::high_resolution_clock::now();
        }

        ~Timer()
        {
            // If Stop already called then no need to call Stop()
            if(!_Stop)
                Stop();
        }

        void Stop()
        {
            float _Duration = 0.0f;
            Stop(_Duration);
            SP_CORE_PRINT("PROFILE: " << _Name << " : " << _Duration << " ms")
        }

        void Stop(float& _Duration)
        {
            auto _End = std::chrono::high_resolution_clock::now();

            // Casting the time into MilliSeconds
            long long Start = std::chrono::time_point_cast<std::chrono::milliseconds>(_Start).time_since_epoch().count();
            long long End = std::chrono::time_point_cast<std::chrono::milliseconds>(_End).time_since_epoch().count();

            _Duration = (End - Start) * 0.001f;
            _Stop = true;
        }
    private:
        const char *_Name;
        std::chrono::time_point<std::chrono::_V2::system_clock> _Start;
        bool _Stop = false;
    };
} // namespace Space

// Define used for simpilicty
#define SP_PROFILE_SCOPE(Name) Space::Timer Timer##__LINE__(Name);
