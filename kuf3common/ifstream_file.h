#pragma once

#include <fstream>
#include <xstring>
#include <filesystem> // 디렉토리 및 파일 제어
#include <map>

using namespace std;
namespace fs = experimental::filesystem; // 이녀석... 무조건 utf16만 쓴다. -_-

using vec_file = vector<wstring>;
using file_name = wstring;
using vec_files = map<file_name, vec_file>;

class ifstream_file
{
public:
	ifstream_file(const wchar_t* directory_name);
	virtual ~ifstream_file();

	bool read_all(__out vec_files& files); // 디렉토리에 있는 모든 파일 읽어줌
	void force_read_file(const wchar_t* file_name, __out vec_file& file); // 파일 열려 있으면 무조건 닫고 다시 열음.
	bool read_file(const wchar_t* file_name, __out vec_file& file); // 파일 열려 있으면 false

private:
	bool open(const wchar_t* file_name);

	basic_ifstream<wchar_t, char_traits<wchar_t> > _read;
	wstring _directory_name;

	fs::path _directory_path;
};

