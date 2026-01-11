#pragma once

namespace util
{
    // https://blog.bearcats.nl/perfect-sleep-function/
    // https://github.com/blat-blatnik/Snippets/blob/main/precise_sleep.c
    // 
    // The PERFECT sleeping function for Windows.
    // 
    // - Sleep times accurate to 1 microsecond
    // - Low CPU usage
    // - Runs on Windows Vista and up
    void preciseSleep(double seconds);
    void initPreciseSleep();
}
