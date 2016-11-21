#pragma once

#include <time.h>
#include <cwchar>
#include <xstring>
#include <tchar.h>

namespace _system {
	const int one_minute_second = 60;
	const int one_hour_second = 3600;
	const int one_day_second = 86400;

	inline clock_t get_clock()
	{
		return ::clock();
	}
	
	inline __time64_t get_time()
	{
		return _time64(NULL);
	}

	inline void get_tm(tm* t)
	{
		auto base = get_time();
		_localtime64_s(t, &base);
		t->tm_year += 1900;
		t->tm_mon += 1;
	}

	inline double elapsed_time(const clock_t start_msec, const clock_t finish_msec)
	{
		return finish_msec - start_msec / CLOCKS_PER_SEC;
	}

	inline double elapsed_time(const __time64_t start_sec, const __time64_t finish_sec)
	{
		return _difftime64(finish_sec, start_sec);
	}

	inline bool elapsed_time_utc(_Out_ _timespec64* const time, const int base_sec)
	{
		return 0 != _timespec64_get(time, base_sec);
	}

	inline wstring get_current_date_by_YY_MM_DD()
	{
		tm cur_date;
		get_tm(&cur_date);
		wchar_t buf[32] = { 0, };
		swprintf_s(buf, 32, L"%04d-%02d-%02d", cur_date.tm_year, cur_date.tm_mon, cur_date.tm_mday); // _stprintf_s
		return wstring(buf);
	}

	inline wstring get_current_date_by_HH_MM_SS_MIL()
	{
		SYSTEMTIME cur_date;
		GetLocalTime(&cur_date);
		wchar_t buf[32] = { 0, };
		swprintf_s(buf, 32, L"%02d:%02d:%02d:%03d", cur_date.wHour, cur_date.wMinute, cur_date.wSecond, cur_date.wMilliseconds);
		return wstring(buf);
	}

	inline bool converwstring(_Out_ TCHAR* const buffer, size_t number_of_elements, __time64_t* const base)
	{
		return EINVAL != _tctime_s(buffer, number_of_elements, base);
	}

	inline bool converwstring(_Out_ TCHAR* const buffer, size_t number_of_elements, tm* const base)
	{
		return EINVAL != _tasctime_s(buffer, number_of_elements, base);
	}

	inline bool convert_tm_local(_Out_ tm* const out_t, const __time64_t base)
	{
		return EINVAL != localtime_s(out_t, &base);
	}

	inline bool convert_tm(_Out_ tm* const out_t, const __time64_t base)
	{
		return EINVAL != gmtime_s(out_t, &base);
	}

	inline __time64_t convert_time_t(_Inout_ tm* const base)
	{
		return _mktime64(base);
	}

	inline time_t add_day(_Inout_ tm* const tm_time, int day)
	{
		return _mktime64(tm_time) + (day * one_day_second);
	}
	inline time_t add_hour(_Inout_ tm* const tm_time, int hour)
	{
		return _mktime64(tm_time) + (hour * one_hour_second);
	}
	inline time_t add_minute(_Inout_ tm* const tm_time, int minute)
	{
		return _mktime64(tm_time) + (minute * one_minute_second);
	}
	inline time_t add_second(_Inout_ tm* const tm_time, int second)
	{
		return _mktime64(tm_time) + second;
	}
	inline time_t sub_day(_Inout_ tm* const tm_time, int day)
	{
		return _mktime64(tm_time) + (day * one_day_second);
	}
	inline time_t sub_hour(_Inout_ tm* const tm_time, int hour)
	{
		return _mktime64(tm_time) + (hour * one_hour_second);
	}
	inline time_t sub_minute(_Inout_ tm* const tm_time, int minute)
	{
		return _mktime64(tm_time) + (minute * one_minute_second);
	}
	inline time_t sub_second(_Inout_ tm* const tm_time, int second)
	{
		return _mktime64(tm_time) + second;
	}

} /* system*/