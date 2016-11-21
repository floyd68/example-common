#pragma once

#define NETWORK_BUF_SIZE 2048
#define MAX_STRING_LENGTH 512

#include <type_traits>
#include <string>
#include <stdexcept>
#include "SNetBuf.h"
#include <assert.h>

class SMsgSend;
class SMsgRecv;

enum class KUF3PACKET : unsigned short;

class NetStreamable
{
public:
	virtual bool WriteTo(SMsgSend& msg) const = 0;
	virtual bool ReadFrom(const SMsgRecv& msg) = 0;
};

class SMsgSend
{
public:
	SMsgSend(unsigned short id)
		: m_usID( *reinterpret_cast<unsigned short *>(&m_data[2]) )
		, m_usSize( *reinterpret_cast<unsigned short *>(&m_data[0]) )
	{
		ID(id);
	}
	SMsgSend(KUF3PACKET id);

	SMsgSend(SMsgSend& other)
		: m_usID( *reinterpret_cast<unsigned short *>(&m_data[2]) )
		, m_usSize( *reinterpret_cast<unsigned short *>(&m_data[0]) )
	{
		memcpy(&m_data[0], &other.m_data[0], other.m_usSize + 4);
	}
	SMsgSend& operator = (SMsgSend& other)
	{
		memcpy(&m_data[0], &other.m_data[0], other.m_usSize + 4);
	}

	SMsgSend& ID(unsigned short id)
	{
		m_usID = id;
		m_usSize = 0;
		return *this;
	}
	void Write(const void* p, unsigned short sz)
	{
		if (m_usSize + 4 + sz >= NETWORK_BUF_SIZE)
			throw std::length_error("Net Buffer overflow");
		memcpy(&m_data[m_usSize + 4], p, sz);
		m_usSize += sz;
	}
	template <typename T,
		typename = std::enable_if< std::is_pod<T>::value && !std::is_pointer<T>::value>::type>
	SMsgSend& operator << (const T& value)
	{
		Write(&value, sizeof(T));
		return *this;
	}
	SMsgSend& operator << (const NetStreamable& value)
	{
		value.WriteTo(*this);
		return *this;
	}
	SMsgSend& operator << (const std::string& str)
	{
		auto sz = str.length();
		if (sz > MAX_STRING_LENGTH)
			sz = MAX_STRING_LENGTH;

		unsigned short usStringLen = static_cast<unsigned short>(sz);
		*this << usStringLen;

		Write( str.c_str(), static_cast<unsigned short>(sz) );
		return *this;
	}
	SMsgSend& operator << (const char* szMessage)
	{
		auto sz = strlen(szMessage);

		if (sz > MAX_STRING_LENGTH)
			sz = MAX_STRING_LENGTH;

		unsigned short usStringLen = static_cast<unsigned short>(sz);
		*this << usStringLen;

		Write( szMessage, static_cast<unsigned short>(sz));
		return *this;
	}

	const unsigned char* GetData() const { return m_data; }
	unsigned short GetSize() const { return m_usSize + 4; }
	unsigned short GetusID() const { return m_usID; }

	void SetData(const unsigned char* pData, unsigned short usSize)
	{
		memcpy(m_data, pData, usSize + 4);
	}

private:
	unsigned short& m_usSize;
	unsigned short& m_usID;
	unsigned char m_data[NETWORK_BUF_SIZE];
};


class SMsgRecv
{
public:
	SMsgRecv( SNetBuf* pBuf );

	SMsgRecv(SMsgRecv& other) = delete;
	SMsgRecv& operator = (SMsgRecv& other) = delete;

	void Read(void* p, unsigned short sz) const
	{
		if (m_usRead + sz - 4 > m_usSize)
			throw std::length_error("Read overflow");
		m_pBuf->read_at(m_usRead, p, sz);
		m_usRead += sz;
	}
	template <typename T,
		typename = std::enable_if< std::is_pod<T>::value && !std::is_pointer<T>::value>::type>
		const SMsgRecv& operator >> (T& value) const
	{
		Read(&value, sizeof(T));

		return *this;
	}
	const SMsgRecv& operator >> (NetStreamable& value) const
	{
		value.ReadFrom(*this);
		return *this;
	}
	const SMsgRecv& operator >> (std::string& str) const
	{
		unsigned short sz;

		*this >> sz;
		if ( sz > MAX_STRING_LENGTH)
			throw std::length_error("string too long");

		char* szTemp = new char[sz + 1];
		Read( szTemp, sz);
		szTemp[sz] = 0;

		str = szTemp;

		return *this;
	}
	void done_msg() const
	{
		assert(is_valid());
		m_pBuf->pop_data(4 + m_usSize);
	}
	bool is_valid() const
	{
		return (m_pBuf->available_data() >= (unsigned short)(m_usSize + (unsigned short)4) );
	}

	const SNetBuf* GetBuf() const { return m_pBuf; }
	unsigned short GetSize() const { return m_usSize; }
	unsigned short GetID() const { return m_usID; }

	SMsgSend ConvertSendMsg() const
	{
		SMsgSend msg(m_usID);

		unsigned char* szTemp = new unsigned char[m_usSize + 4];
		m_pBuf->read_at(0, szTemp, m_usSize + 4);

		msg.SetData(szTemp, m_usSize);

		return msg;
	}

private:
	unsigned short m_usSize;
	unsigned short m_usID;
	mutable unsigned short m_usRead;
	SNetBuf* m_pBuf;
};