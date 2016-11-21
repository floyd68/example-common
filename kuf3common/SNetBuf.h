#pragma once

#define NETWORK_RECV_BUF_SIZE 16384

#include <atomic>
#include <vector>

// Circular buf
// m_nHead : start of data
// m_nTail : start of empty
// m_nHead == m_nTail : empty
// m_nTail == (m_nHead - 1 + NETWORK_RECV_BUF_SIZE) % NETWORK_RECV_BUF_SIZE : full of data
class SNetBuf
{
public:
	SNetBuf() : m_nHead(0), m_nTail(0) {}

	typedef std::size_t size;
	typedef std::pair <void*, size> buf_desc;
	typedef std::vector <buf_desc> vecBuf;

	void clear()
	{
		m_nHead = m_nTail = 0;
	}
	vecBuf get_empty_buf()
	{
		vecBuf bufs;

		if (m_nTail < m_nHead)
		{
			bufs.push_back(buf_desc(&m_data[m_nTail], m_nHead - m_nTail - 1));
		}
		else
		{
			bufs.push_back(buf_desc(&m_data[m_nTail], NETWORK_RECV_BUF_SIZE - m_nTail));
			if (m_nHead == 0)
				bufs[0].second -= 1;
			else if (m_nHead != 1)
				bufs.push_back(buf_desc(&m_data[0], m_nHead - 1));
		}
		return bufs;
	}
	vecBuf get_read_buf()
	{
		vecBuf bufs;

		if (m_nHead <= m_nTail)
		{
			bufs.push_back(buf_desc(&m_data[m_nHead], m_nTail - m_nHead));
		}
		else
		{
			bufs.push_back(buf_desc(&m_data[m_nHead], NETWORK_RECV_BUF_SIZE - m_nHead));
			bufs.push_back(buf_desc(&m_data[0], m_nTail));
		}
		return bufs;
	}
	size available_data()
	{
		return (m_nTail + NETWORK_RECV_BUF_SIZE - m_nHead) % NETWORK_RECV_BUF_SIZE;
	}
	size available_space()
	{
		return (m_nHead + NETWORK_RECV_BUF_SIZE - m_nTail - 1) % NETWORK_RECV_BUF_SIZE;
	}
	void commit_recv(size sz)
	{
		if (sz > available_space())
			throw std::range_error("not available space");
		m_nTail = (m_nTail + sz) % NETWORK_RECV_BUF_SIZE;
	}
	void push_data(const void* p, size sz)
	{
		if (sz > available_space())
			throw std::range_error("not available space");

		memcpy(&m_data[m_nTail], p, sz);
		m_nTail = (m_nTail + sz) % NETWORK_RECV_BUF_SIZE;
	}
	void pop_data(size sz)
	{
		if (sz > available_data())
			throw std::range_error("not available data");
		m_nHead = (m_nHead + sz) % NETWORK_RECV_BUF_SIZE;

		// if (m_nHead == m_nTail)
		//	m_nHead = m_nTail = 0;
	}
	void read_at(size index, void* p, size sz)
	{
		if (index + sz > available_data())
			throw std::range_error("not available data");

		size pos = (m_nHead + index) % NETWORK_RECV_BUF_SIZE;
		if (pos + sz > NETWORK_RECV_BUF_SIZE)
		{
			size szToRead = NETWORK_RECV_BUF_SIZE - pos;
			memcpy(p, &m_data[pos], szToRead);
			sz -= szToRead;
			p = static_cast<char*>(p)+szToRead;
			pos = 0;
		}
		if (sz > 0)
			memcpy(p, &m_data[pos], sz);
	}

	// private:
	std::atomic <size> m_nHead;
	std::atomic <size> m_nTail;
	unsigned char m_data[NETWORK_RECV_BUF_SIZE];
};

