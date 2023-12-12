#pragma once

#include <string>

#include <AL/al.h>
#include "AL/alc.h"

//
// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
//
namespace audio
{
    bool check_al_errors(const char* file, int lineNumber);

//#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

    template<typename alFunction, typename... Params>
    auto alCallImpl(const char* filename,
        const std::uint_fast32_t line,
        alFunction function,
        Params... params)
        -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
    {
        function(std::forward<Params>(params)...);
        return check_al_errors(filename, line);
    }

    template<typename alFunction, typename ReturnType, typename... Params>
    auto alCallImpl(const char* filename,
        const std::uint_fast32_t line,
        alFunction function,
        ReturnType& returnValue,
        Params... params)
        -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool>
    {
        returnValue = function(std::forward<Params>(params)...);
        return check_al_errors(filename, line);
    }
}
