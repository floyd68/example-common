#include "stdafx.h"
#include "str_util.h"
#include "ofstream_file.h"
#include "time_util_utc.h"
#include <iostream>

#include <Shlwapi.h>  // 파일 존재 유무 체크 헤더
#pragma comment(lib, "Shlwapi.lib")

using namespace std;
using namespace _system;

ofstream_file::ofstream_file(const wchar_t* directory_name, const wchar_t* file_name) : _is_opened(false), _last_file_no(0)
{
#ifdef _UNICODE
	_name = wstring(file_name);
	wstring _directory_name = wstring(directory_name);

	_write.imbue(locale("kor"));
#else
	_name = M2W(file_name);
	wstring _directory_name = M2W(directory_name);
	_write.imbue(locale("kor"));
#endif
	_directory_path = fs::current_path();
	if (false == _directory_name.empty())
	{
		_directory_path = _directory_path / _directory_name.c_str();
	}
	
	fs::create_directories(_directory_path);
}

ofstream_file::~ofstream_file()
{
	close();
	backup();
	fs::remove(_directory_path / _full_name.c_str());
}

bool ofstream_file::open()
{
	_mutex.lock();
	if (_write.is_open() == false)
	{
		_full_name = _name + L".txt";
		_write.open(_directory_path / _full_name.c_str(), ios_base::app);
		_is_opened = true;
	}
	_mutex.unlock();

	if (_write.fail()) {
		wcout << L"[error] ofstream_file::open() unable to open file for appending. file_path[" << _name.c_str() << L"]" << endl;
		return false;
	}

	return true;
}

void ofstream_file::close()
{
	_is_opened = false;
	_mutex.lock();
	if (_write.is_open())
	{
		_write.close();
	}
	_mutex.unlock();
}

void ofstream_file::backup()
{
	close();
	wstring new_name;
	create_new_name(__out new_name);
	fs::rename(_directory_path / _full_name.c_str(), _directory_path / new_name.c_str());
}

void ofstream_file::create_new_name(__out wstring& new_name)
{
	auto cur_date = get_current_date_by_YY_MM_DD();
	while (true)
	{
		new_name = _system::format(_T("_%s_%04d.txt"), cur_date.c_str(), _last_file_no).c_str();
		new_name.insert(new_name.begin(), _name.begin(), _name.end());
		if (false == fs::exists(_directory_path / new_name.c_str()))
		{
			break;
		}

		++_last_file_no;
	}
}

void ofstream_file::append_line(const wchar_t* str)
{
	_mutex.lock();
	_write << str << endl;
	_mutex.unlock();
}

uintmax_t ofstream_file::get_file_size()
{
	uintmax_t size = 0;
	try {
		size = fs::file_size(_directory_path / _full_name.c_str());
	}
	catch (exception& e)
	{
		wcout << "--- ofstream_file::get_file_size() exception ---" << endl;
		wcout << _directory_path.c_str() << _full_name.c_str() << endl;
		cerr << e.what() << endl;
		wcout << "--- ofstream_file::get_file_size() exception end ---" << endl;
		
		open();
		return fs::file_size(_directory_path / _full_name.c_str());
	}
	return size;
}