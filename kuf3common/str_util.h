#pragma once


#include <vector>
#include <string>
#include <atlconv.h>
#include <codecvt>
#include <tchar.h>

#define TRIM_SPACE L" \t\n\v"
using t_string = basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> >;

namespace _system {

// Multibyte -> WideChar
inline std::wstring M2W(const std::string & str)
{
	USES_CONVERSION;
	return A2W(str.c_str());
}

inline std::wstring M2W(const char* str)
{
	USES_CONVERSION;
	return A2W(str);
}


// WideChar -> Multibyte
inline std::string W2M(const std::wstring & str)
{
	USES_CONVERSION;
	return W2A(str.c_str());
}

inline std::string W2M(const wchar_t* str)
{
	USES_CONVERSION;
	return W2A(str);
}

// UTF-8 -> WideChar
inline std::wstring UTF82W(const std::string & utf8_str)
{
	std::wstring_convert< std::codecvt_utf8< wchar_t > >	my_conv;
	return my_conv.from_bytes(utf8_str);
}

// WideChar -> UTF-8
inline std::string W2UTF8(const std::wstring & wide_str)
{
	std::wstring_convert< std::codecvt_utf8< wchar_t > >	my_conv;
	return my_conv.to_bytes(wide_str);
}

inline int to_int(const TCHAR* str)
{
	return _ttoi(str);
}

inline double to_float(const TCHAR* str)
{
	return _ttof(str);
}

inline size_t to_size_t(const TCHAR* str)
{
	return _ttoi64(str);
}

inline std::wstring format(const TCHAR* fmt, ...)
{
	TCHAR buf[256];
	va_list args;
	va_start(args, fmt);

	TCHAR* p = buf;
	size_t buf_len = sizeof(buf) / sizeof(TCHAR);
	while (_vsntprintf_s(p, buf_len, buf_len - 1, fmt, args) == -1) {
		if (p != buf) delete[] p;
		buf_len *= 2;
		p = new TCHAR[buf_len];
	}

	va_end(args);

#ifdef  UNICODE
	std::wstring strTemp(p);
#else
	std::wstring strTemp = M2W(p);
#endif

	if (p != buf) delete[] p;

	return strTemp;
}

inline std::wstring trim(std::wstring & str)
{
	if (str.length() == 0)
		return str;

	std::wstring r = str.erase(str.find_last_not_of(TRIM_SPACE) + 1);
	return r.erase(0, r.find_first_not_of(TRIM_SPACE));
}

inline static void split_with_string(const TCHAR* cmd_string, std::vector< t_string > & v_container, const TCHAR* sep_string, bool is_allow_blank = false)
{
	if (!cmd_string) return;

	const TCHAR *p = cmd_string;

	t_string strTemp;
	size_t separator_len = _tcslen(sep_string);

	while (*p)
	{
		if (_tcsncmp(p, sep_string, separator_len) == 0) {
			if (is_allow_blank || !strTemp.empty()) {
				v_container.push_back(strTemp);
			}
			p += separator_len;
			strTemp.clear();
			continue;
		}

		strTemp.append(1, *p);
		++p;
	}

	if (is_allow_blank || !strTemp.empty())
	{
		v_container.push_back(std::move(strTemp));
	}
}


inline static int replace(t_string & str, const TCHAR* from, size_t from_len, const TCHAR* to, size_t to_len)
{
	int cnt = 0;
	size_t nPos = 0;

	while (true) {
		nPos = str.find(from, nPos);
		if (nPos == t_string::npos) break;

		str.replace(nPos, from_len, to);
		nPos += to_len;

		cnt++;
	}

	return cnt;
}

inline static int replace(t_string & str, const t_string & from, const t_string & to)
{
	return replace(str, from.c_str(), from.size(), to.c_str(), to.size());
}

} /* system */
