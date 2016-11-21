#include "stdafx.h"
#include "SMsg.h"
#include <assert.h>

SMsgSend::SMsgSend(KUF3PACKET id)
	: SMsgSend(static_cast<unsigned short>(id)) {}

SMsgRecv::SMsgRecv(SNetBuf* pBuf)
	: m_pBuf(pBuf), m_usRead(4)
{
	m_pBuf->read_at(0, &m_usSize, sizeof(unsigned short));
	m_pBuf->read_at(2, &m_usID, sizeof(unsigned short));
}