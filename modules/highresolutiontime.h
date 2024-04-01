#ifndef HIGHRESOLUTIONTIME_H
#define HIGHRESOLUTIONTIME_H

#pragma once

#include <assert.h>
#include <stdint.h>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/thread/thread.hpp>

#if defined(WINDOWS) || defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
#include <sdkddkver.h>
#endif

#if defined(USING_TBB)
#include <tbb/concurrent_queue.h>
template <typename _Ty>
using concurrent_queue = tbb::concurrent_queue<_Ty>;
#elif defined(WINDOWS) || defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
#include <ppl.h>
#include <concurrent_queue.h>
template <typename _Ty>
using concurrent_queue = concurrency::concurrent_queue<_Ty>;
#endif

#ifdef USING_LOGURU
#include "loguru/loguru.hpp"
#endif

#if __cplusplus < 201703L && _MSVC_LANG < 201703L
namespace std {

// FUNCTION TEMPLATE clamp
template <class _Ty, class _Pr>
_NODISCARD constexpr const _Ty &clamp(
    const _Ty &_Val, const _Ty &_Min_val, const _Ty &_Max_val, _Pr _Pred)
{
    // returns _Val constrained to [_Min_val, _Max_val]
#if _ITERATOR_DEBUG_LEVEL == 2
    if (_DEBUG_LT_PRED(_Pred, _Max_val, _Min_val)) {
        _STL_REPORT_ERROR("invalid bounds arguments passed to std::clamp");
        return _Val;
    }
#endif // _ITERATOR_DEBUG_LEVEL == 2
    if (_DEBUG_LT_PRED(_Pred, _Max_val, _Val)) {
        return _Max_val;
    }
    if (_DEBUG_LT_PRED(_Pred, _Val, _Min_val)) {
        return _Min_val;
    }
    return _Val;
}

template <class _Ty>
_NODISCARD constexpr const _Ty &clamp(const _Ty &_Val, const _Ty &_Min_val, const _Ty &_Max_val)
{
    // returns _Val constrained to [_Min_val, _Max_val]
    return _STD clamp(_Val, _Min_val, _Max_val, less<_Ty>{});
}

} // namespace std
#endif

class not_implemented_error : public std::exception {
public:
    not_implemented_error(const std::string &name)
        : object(name)
    {
        message = "Object `" + object + "` not implemented.";
    }

    const char *what() const throw() { return message.c_str(); }

    std::string message;
    const std::string object;
};


/**
     * @brief 获取程序当前时间，单位[s]
     */
class HSTime {
public:
    static const HSTime &GetInstance()
    {
        static HSTime _time;
        return _time;
    }

    static double wall_time();

    static void sleep_for(double dt);

    const double get_start_time() const { return m_start_time; }

    const double software_time() const { return wall_time() - m_start_time; }

    /// Generate time in "%Y%m%d_%H%M%S" format.
    static std::string timestamp();

private:
    double m_start_time;

    HSTime() { m_start_time = wall_time(); }

    HSTime(const HSTime &) = delete;
};


#endif // HIGHRESOLUTIONTIME_H
