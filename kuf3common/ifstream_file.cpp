#include "stdafx.h"
#include <iostream>
#include <fileapi.h>
#include "ifstream_file.h"

ifstream_file::ifstream_file(const wchar_t* directory_name)
	: _directory_name(directory_name),
	  _directory_path(fs::current_path() / directory_name)
{
}


ifstream_file::~ifstream_file()
{
}

bool ifstream_file::read_all(__out vec_files& files)
{
	WIN32_FIND_DATA find_data;
	wstring file_type = _directory_path / L"/*.txt";
	auto handle = FindFirstFile(file_type.c_str(), &find_data);
	if (INVALID_HANDLE_VALUE == handle)
	{
		return false;
	}

	vec_file file;
	do
	{
		force_read_file(find_data.cFileName, file);
		files.emplace(find_data.cFileName, file);
		file.clear();
	} while (FindNextFile(handle, &find_data));

	return true;
}

void ifstream_file::force_read_file(const wchar_t* file_name, __out vec_file& file)
{
	if (_read.is_open())
	{
		_read.close();
	}

	read_file(file_name, file);
}

bool ifstream_file::read_file(const wchar_t* file_name, __out vec_file& file)
{
	if (open(file_name) == false)
		return false;

	wstring t;
	while (_read.eof() == false)
	{
		_read >> t;
		file.push_back(t);
	}

	return true;
}


bool ifstream_file::open(const wchar_t* file_name)
{
	if (file_name == nullptr)
		return false;

	_read.open(_directory_path / file_name, ios_base::app);
	return _read.is_open();
}