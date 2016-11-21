#pragma once

#include <fstream>
#include <xstring>
#include <filesystem> // 디렉토리 및 파일 제어
#include <mutex>

using namespace std;
namespace fs = experimental::filesystem; // 이녀석... 무조건 utf16만 쓴다. -_-

class ofstream_file
{
public:
	ofstream_file(const wchar_t* directory_name, const wchar_t* file_name);
	virtual ~ofstream_file();

	bool open();
	void close();
	void append_line(const wchar_t* str);
	void backup(); // must close file before open.
	uintmax_t get_file_size(); // return value - byte

	ofstream_file() = delete;
	ofstream_file operator = (ofstream_file& arg) = delete;
private:

	wstring _name; // file name
	wstring _full_name; // This value contains file name and file extension.
	basic_fstream<wchar_t, char_traits<wchar_t> > _write;

	int _last_file_no; // Increases after backup. used to file renaming.
	bool _is_opened;

	mutex _mutex;
	fs::path _directory_path;

	void create_new_name(__out wstring& new_name);
};
