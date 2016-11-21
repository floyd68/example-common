#pragma once

#include "ofstream_file.h"
#include <queue>
#include <thread>
#include <atomic>

enum class log_level : int {
	off = 1,
	info,
	warn,
	debug,
	error,
	critical,
	all,
};

class logger : public ofstream_file
{
public:
	logger(const wchar_t* directory_name, const wchar_t* file_name);
	virtual ~logger();

	void set_storage_size(__int8 megabyte = 3); // 3MB

	void set_log_flag(log_level lev);
	void write(log_level lev, const wchar_t* func, const int line, const wchar_t* log_txt, ...);

private:
	void put(log_level lev, const wchar_t* func, const int line, const wchar_t* log_txt);

	void process();
	void write();
	void insert(const wstring& str);

	logger(const logger& p) = delete;
	logger& operator=(const logger& p) = delete;

	uintmax_t _storage_size;
	queue<wstring> _log_container;

	unsigned short _log_flag;

	// therad variable
	mutex _mutex_queue;
	atomic<int> _queue_size;
	thread _thread;
	bool _exit_thread;
};

