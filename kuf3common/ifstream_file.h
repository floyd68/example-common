#pragma once

#include <fstream>
#include <xstring>
#include <filesystem> // ���丮 �� ���� ����
#include <map>

using namespace std;
namespace fs = experimental::filesystem; // �̳༮... ������ utf16�� ����. -_-

using vec_file = vector<wstring>;
using file_name = wstring;
using vec_files = map<file_name, vec_file>;

class ifstream_file
{
public:
	ifstream_file(const wchar_t* directory_name);
	virtual ~ifstream_file();

	bool read_all(__out vec_files& files); // ���丮�� �ִ� ��� ���� �о���
	void force_read_file(const wchar_t* file_name, __out vec_file& file); // ���� ���� ������ ������ �ݰ� �ٽ� ����.
	bool read_file(const wchar_t* file_name, __out vec_file& file); // ���� ���� ������ false

private:
	bool open(const wchar_t* file_name);

	basic_ifstream<wchar_t, char_traits<wchar_t> > _read;
	wstring _directory_name;

	fs::path _directory_path;
};

