// For conditions of distribution and use, see copyright notice in License.txt

#include "../Base/String.h"
#include "../Turso3DConfig.h"

#pragma once

namespace Turso3D
{

/// Low-resolution operating system timer.
class TURSO3D_API Timer
{
public:
    /// Construct. Get the starting clock value.
    Timer();
    
    /// Return elapsed milliseconds and optionally reset.
    unsigned ElapsedMSec(bool reset = false);
    /// Reset the timer.
    void Reset();
    
private:
    /// Starting clock value in milliseconds.
    unsigned startTime;
};

/// High-resolution operating system timer used in profiling.
class TURSO3D_API HiresTimer
{
public:
    /// Construct. Get the starting high-resolution clock value.
    HiresTimer();
    
    /// Return elapsed microseconds and optionally reset.
    long long ElapsedUSec(bool reset = false);
    /// Reset the timer.
    void Reset();

    /// Perform one-time initialization to check support and frequency.
    static void Initialize();
    /// Return if high-resolution timer is supported.
    static bool IsSupported() { return supported; }
    /// Return high-resolution timer frequency if supported.
    static long long Frequency() { return frequency; }

private:
    /// Starting clock value in CPU ticks.
    long long startTime;

    /// High-resolution timer support flag.
    static bool supported;
    /// High-resolution timer frequency.
    static long long frequency;
};

/// Get a date/time stamp as a string.
TURSO3D_API String TimeStamp();

}