#ifndef __C_SYMBOL_V2_2_H__
#define __C_SYMBOL_V2_2_H__

/*
 *    Simple symbol(or id) type based on string table 
 *
 *           2005.11.25 by Young-Hyun Joo
 *
 *        ver.2 on 2006.6.14 by Young-Hyun Joo
 */

#include <string.h>
#include <stddef.h>
#include <deque>
#include "c_hash.h"

struct c_symbol_dummy_lock
{
	void lock() {}
	void unlock() {}
};

template< typename SYMSPACE, typename LOCK = c_symbol_dummy_lock >		// SYMSPACE : dummy type indicating symbol namespace 
struct c_symbol
{
	class symi;

	// case sensitive version

	class sym
	{
	public:

		sym()															{ m_pStr = nullptr; }
		explicit sym( const char* pStr, bool bCopyStrOnAdd = true )		{ m_pStr = c_symbol::manager().get( pStr, bCopyStrOnAdd, -1 ); }
		sym( const char* pStr, size_t len )								{ m_pStr = c_symbol::manager().get( pStr, true, int(len) ); }
		sym( const char* pStr, int len )								{ m_pStr = c_symbol::manager().get( pStr, true, len ); }
		explicit sym( const std::string & str )							{ m_pStr = c_symbol::manager().get( str.c_str(), true, int(str.size()) ); }
		sym( const sym& asym )											{ m_pStr = asym.m_pStr; }
		sym& operator = ( const sym& asym )								{ m_pStr = asym.m_pStr; return *this; }

		operator uintptr_t() const										{ return uintptr_t( m_pStr ); }
		const char* c_str() const										{ return m_pStr ? m_pStr : ""; }

#if defined(__ORBIS__) || ( defined(_MSC_VER) && _MSC_VER >= 1800 )
		bool operator == (std::nullptr_t t) const { static_assert(false, "Can not compare c_symbol with nullptr"); }
		bool operator != (std::nullptr_t t) const { static_assert(false, "Can not compare c_symbol with nullptr"); }
#endif

		bool operator == ( const char* pStr ) const						{ return m_pStr == pStr || (m_pStr && pStr && !strcmp( m_pStr, pStr )); }
		bool operator != ( const char* pStr ) const						{ return m_pStr != pStr && (!m_pStr || !pStr || strcmp( m_pStr, pStr )); }
		bool operator == ( int n ) const								{ assert( n == 0 ); return m_pStr == nullptr; }
		bool operator != ( int n ) const								{ assert( n == 0 ); return m_pStr != nullptr; }

		bool operator == ( const sym& asym ) const						{ return uintptr_t(m_pStr) == uintptr_t(asym); }
		bool operator != ( const sym& asym ) const						{ return uintptr_t(m_pStr) != uintptr_t(asym); }
		bool operator == ( const symi& asym ) const						{ return uintptr_t(m_pStr) == uintptr_t(asym) || (m_pStr && !stricmp( m_pStr, asym.c_str() )); }
		bool operator != ( const symi& asym ) const						{ return uintptr_t(m_pStr) != uintptr_t(asym) && (!m_pStr || stricmp( m_pStr, asym.c_str() )); }
		
		static void ClearTable( bool bPreserveConstSym = false )		{ c_symbol::manager().clear( bPreserveConstSym ); }
		static int Count()												{ return c_symbol::manager().m_table.size(); }

		bool TestAndSet( const char* pStr )								{ m_pStr = c_symbol::manager().lookUp( pStr, -1 ); return pStr == nullptr || *pStr == 0 || m_pStr; }
		bool TestAndSet( const char* pStr, size_t len )					{ m_pStr = c_symbol::manager().lookUp( pStr, int(len) ); return pStr == nullptr || *pStr == 0 || m_pStr; }
		bool TestAndSet( const char* pStr, int len )					{ m_pStr = c_symbol::manager().lookUp( pStr, len ); return pStr == nullptr || *pStr == 0 || m_pStr; }

	private:
		operator std::string() const;

		const char* m_pStr;
	};

	class csym : public sym
	{
	public:
		
		csym( const char* pStr ) : sym( pStr, false ) {}
	};


	// case insensitive version

	class symi
	{
	public:

		symi()															{ m_pStr = nullptr; }
		explicit symi( const char* pStr, bool bCopyStrOnAdd = true )	{ m_pStr = c_symbol::manageri().get( pStr, bCopyStrOnAdd, -1 ); }
		symi( const char* pStr, int len )								{ m_pStr = c_symbol::manageri().get( pStr, true, len ); }
		symi( const char* pStr, size_t len )							{ m_pStr = c_symbol::manageri().get( pStr, true, int(len) ); }
		explicit symi( const std::string & str )						{ m_pStr = c_symbol::manageri().get( str.c_str(), true, int(str.size()) ); }
		symi( const symi& asym )										{ m_pStr = asym.m_pStr; }
		symi& operator = ( const symi& asym )							{ m_pStr = asym.m_pStr; return *this; }

		operator uintptr_t() const										{ return uintptr_t( m_pStr ); }
		const char* c_str() const										{ return m_pStr ? m_pStr : ""; }

#if defined(__ORBIS__) || ( defined(_MSC_VER) && _MSC_VER >= 1800 )
		bool operator == (std::nullptr_t t) const { static_assert(false, "Can not compare c_symbol with nullptr"); }
		bool operator != (std::nullptr_t t) const { static_assert(false, "Can not compare c_symbol with nullptr"); }
#endif

		bool operator == ( const char* pStr ) const						{ return m_pStr == pStr || (m_pStr && pStr && !stricmp( m_pStr, pStr )); }
		bool operator != ( const char* pStr ) const						{ return m_pStr != pStr && (!m_pStr || !pStr || stricmp( m_pStr, pStr )); }
		bool operator == ( int n ) const								{ assert( n == 0 ); return m_pStr == nullptr; }
		bool operator != ( int n ) const								{ assert( n == 0 ); return m_pStr != nullptr; }

		bool operator == ( const symi& asym ) const						{ return uintptr_t(m_pStr) == uintptr_t(asym); }
		bool operator != ( const symi& asym ) const						{ return uintptr_t(m_pStr) != uintptr_t(asym); }
		bool operator == ( const sym& asym ) const						{ return uintptr_t(m_pStr) == uintptr_t(asym) || (m_pStr && !stricmp( m_pStr, asym.c_str() )); }
		bool operator != ( const sym& asym ) const						{ return uintptr_t(m_pStr) != uintptr_t(asym) && (!m_pStr || stricmp( m_pStr, asym.c_str() )); }
		
		static void ClearTable( bool bPreserveConstSym = false )		{ c_symbol::manageri().clear( bPreserveConstSym ); }
		static int Count()												{ return c_symbol::manageri().m_table.size(); }

		bool TestAndSet( const char* pStr )								{ m_pStr = c_symbol::manageri().lookUp( pStr, -1 ); return pStr == nullptr || *pStr == 0 || m_pStr; }
		bool TestAndSet( const char* pStr, size_t len )					{ m_pStr = c_symbol::manageri().lookUp( pStr, int(len) ); return pStr == nullptr || *pStr == 0 || m_pStr; }
		bool TestAndSet( const char* pStr, int len )					{ m_pStr = c_symbol::manageri().lookUp( pStr, len ); return pStr == nullptr || *pStr == 0 || m_pStr; }

	private:
		operator std::string() const;

		const char* m_pStr;

		static inline unsigned repr( char c )                            { return c >= 'A' && c <= 'Z' ? c - ('A'-'a') : c; }
		static inline int stricmp( const char* p, const char* q )
		{
			for (; *p && repr(*p) == repr(*q); ++p, ++q ) {}
			return int(repr(*q)) - int(repr(*p));
		}
	};

	class csymi : public symi
	{
	public:
		
		csymi( const char* pStr ) : symi( pStr, false ) {}
	};

private:

	friend class sym;
	friend class symi;

	template< typename HashPr >
	struct symManager
	{
		void clear( bool bPreserveConstSym )
		{
			m_cs.lock();

			if ( bPreserveConstSym )
			{
				std::deque< std::string >::iterator it, ite;
				for ( it = m_holder.begin(), ite = m_holder.end(); it != ite; ++it )
					m_table.remove( it->c_str() );
			}
			else
				m_table.clear();

			m_holder.clear();
			m_cs.unlock();
		}

		const char* lookUp( const char* pStr, int len )
		{
			if ( pStr == nullptr || *pStr == 0 )
				return nullptr;

			m_cs.lock();

			if ( len > 0 ) {

				std::string key( pStr, len );
				if ( !m_table.lookup( key.c_str(), pStr ) )
					pStr = nullptr;

			} else {

				if ( !m_table.lookup( pStr, pStr ) )
					pStr = nullptr;
			}

			m_cs.unlock();
			return pStr;
		}

		const char* get( const char* pStr, bool bCopyStr, int len )
		{
			if ( pStr == nullptr || *pStr == 0 )
				return nullptr;

			m_cs.lock();

			if ( len > 0 ) {

				std::string key( pStr, len );
				if ( m_table.lookup( key.c_str(), pStr ) )
				{
					m_cs.unlock();
					return pStr;
				}

				m_holder.push_back( key );
				pStr = m_holder.back().c_str();

			} else {

				if ( m_table.lookup( pStr, pStr ) )
				{
					m_cs.unlock();
					return pStr;
				}

				if ( bCopyStr ) {
					m_holder.push_back( std::string( pStr ) );
					pStr = m_holder.back().c_str();
				}
			}

			m_table.add( pStr, pStr );

			m_cs.unlock();
			return pStr;
		}

		c_hash< const char*, HashPr > m_table;
		std::deque< std::string > m_holder;
		LOCK m_cs;
	};

	static symManager< c_hash_def::hashPr_pchar >& manager()			
	{ 
		static symManager< c_hash_def::hashPr_pchar > mgr; 
		return mgr; 
	}
	
	static symManager< c_hash_def::hashPr_pchar_nocase >& manageri() 	
	{ 
		static symManager< c_hash_def::hashPr_pchar_nocase > mgr; 
		return mgr; 
	}
};

template< typename SYMSPACE > inline bool operator == ( const char* pStr, const typename c_symbol< SYMSPACE >::sym& asym )	
{ return uintptr_t(pStr) == uintptr_t(asym) || (pStr && !strcmp( pStr, asym.c_str() )); }

template< typename SYMSPACE > inline bool operator != ( const char* pStr, const typename c_symbol< SYMSPACE >::sym& asym )	
{ return uintptr_t(pStr) != uintptr_t(asym) && (!pStr || strcmp( pStr, asym.c_str() )); }

template< typename SYMSPACE > inline bool operator == ( const char* pStr, const typename c_symbol< SYMSPACE >::symi& asym )	
{ return uintptr_t(pStr) == uintptr_t(asym) || (pStr && !stricmp( pStr, asym.c_str() )); }

template< typename SYMSPACE > inline bool operator != ( const char* pStr, const typename c_symbol< SYMSPACE >::symi& asym )	
{ return uintptr_t(pStr) != uintptr_t(asym) && (!pStr || stricmp( pStr, asym.c_str() )); }

template< typename S1, typename S2 > inline bool operator == ( const typename c_symbol<S1>::sym& asym1, const typename c_symbol<S2>::sym& asym2 )
{ return uintptr_t(asym1) == uintptr_t(asym2) || !strcmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator != ( const typename c_symbol<S1>::sym& asym1, const typename c_symbol<S2>::sym& asym2 )
{ return uintptr_t(asym1) != uintptr_t(asym2) && strcmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator == ( const typename c_symbol<S1>::sym& asym1, const typename c_symbol<S2>::symi& asym2 )
{ return uintptr_t(asym1) == uintptr_t(asym2) || !stricmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator != ( const typename c_symbol<S1>::sym& asym1, const typename c_symbol<S2>::symi& asym2 )
{ return uintptr_t(asym1) != uintptr_t(asym2) && stricmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator == ( const typename c_symbol<S1>::symi& asym1, const typename c_symbol<S2>::sym& asym2 )
{ return uintptr_t(asym1) == uintptr_t(asym2) || !stricmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator != ( const typename c_symbol<S1>::symi& asym1, const typename c_symbol<S2>::sym& asym2 )
{ return uintptr_t(asym1) != uintptr_t(asym2) && stricmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator == ( const typename c_symbol<S1>::symi& asym1, const typename c_symbol<S2>::symi& asym2 )
{ return uintptr_t(asym1) == uintptr_t(asym2) || !stricmp( asym1.c_str(), asym2.c_str() ); }

template< typename S1, typename S2 > inline bool operator != ( const typename c_symbol<S1>::symi& asym1, const typename c_symbol<S2>::symi& asym2 )
{ return uintptr_t(asym1) != uintptr_t(asym2) && stricmp( asym1.c_str(), asym2.c_str() ); }

#endif
