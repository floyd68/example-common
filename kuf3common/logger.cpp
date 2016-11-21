#include "stdafx.h"
#include "logger.h"
#include "time_util_utc.h"

const int _1MB = 1048576;

using namespace _system;

logger::logger(const wchar_t* directory_name, const wchar_t* file_name)
	: ofstream_file(directory_name, file_name),
		_storage_size(_1MB), _exit_thread(false), _queue_size(0), _log_flag(0)
{
	_thread = thread{ &logger::process, this };
}

logger::~logger()
{
	_exit_thread = true;
	if (_thread.joinable()) // need test
	{
		_thread.join();
	}

	write();
}

void logger::set_storage_size(__int8 megabyte)
{
	_storage_size = megabyte * _1MB;
	return;
}

void logger::set_log_flag(log_level lev)
{
	// todo ...
}

void logger::write(log_level lev, const wchar_t* func, const int line, const wchar_t* log_txt, ...)
{
	va_list args;
	va_start(args, log_txt);
	wchar_t buffer[1024];
	_vsnwprintf(buffer, 1024, log_txt, args);
	put(lev, func, line, buffer);
	va_end(args);
}

void logger::put(log_level lev, const wchar_t* func, const int line, const wchar_t* log_txt)
{
	auto log_str = [&]()->wchar_t* {
		switch (lev)
		{
			case log_level::critical:	return L"critical";
			case log_level::error:		return L"error";
			case log_level::debug:		return L"debug";
			case log_level::warn:		return L"warn";
			case log_level::info:		return L"info";
		}
		return L"[?]";
	};

	wstring logtxt(log_txt);

	wchar_t location[128] = { 0, };
	wsprintfW(location, L"[%s][%s][%d][%s] ", log_str(), get_current_date_by_HH_MM_SS_MIL().c_str(), line, func);

	logtxt.insert(0, location);
	insert(logtxt.c_str());
	++_queue_size;
}


void logger::insert(const wstring& str)
{
	_mutex_queue.lock();
	_log_container.push(str);
	_mutex_queue.unlock();
	return;
}

void logger::process()
{
	while (true)
	{
		if (_mutex_queue.try_lock())
		{
			if (_log_container.empty() == false)
			{
				__super::append_line(_log_container.front().c_str());
				_log_container.pop();
				--_queue_size;
			}

			_mutex_queue.unlock();
		}
		else
		{
			auto size =  __super::get_file_size();
			if (_storage_size < size)
			{
				__super::backup();
			}
			if (_queue_size.load() > 1000)
				write();
		}

		if (_exit_thread)
			break;
	}

	return;
}

void logger::write()
{
	_mutex_queue.lock();
	while (_log_container.empty() == false)
	{
		__super::append_line(_log_container.front().c_str());
		_log_container.pop();
	}
	_queue_size = 0;
	_mutex_queue.unlock();
}

