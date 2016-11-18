#ifndef __C_HASH_H__
#define __C_HASH_H__

/*
 *      An Associative Array Implementation based on chained hash, 
 *                    with memory & speed optimization using c_uniheap
 *
 *          2001.10.9 by Young-Hyun Joo
 *
 *        Last Modified on 2012.5.22 by Young-Hyun Joo
 */

#include "c_uniheap.h"
#include <string>

namespace c_hash_detail
{
	template< typename Key, typename hashPr, bool > struct CompKey {
		static inline bool call( unsigned hashKey1, const Key& key1, unsigned hashKey2, const Key& key2 ) {
			return hashKey1 == hashKey2 ? hashPr::isLess( key1, key2 ) : hashKey1 < hashKey2;
		}
	};
	template< typename Key, typename hashPr > struct CompKey< Key, hashPr, true > {
		static inline bool call( unsigned hashKey1, const Key&, unsigned hashKey2, const Key& ) {
			return hashKey1 < hashKey2;
		}
	};
}

template< typename T, typename hashPr, int BLOCK_SIZE = 128, int ALIGN = 4 >
class c_hash
{
public:

	typedef typename hashPr::Key Key;

	c_hash( unsigned slot_count = 50 )
	{
		m_nSlotCount = slot_count;
		m_nCount = 0;
		m_slots = new Slot[ m_nSlotCount ];
		memset( m_slots, 0, m_nSlotCount * sizeof(Slot) );
	}

	struct Initializer 
	{
		Key key;
		T value;
	};

	c_hash( const Initializer initArray[], unsigned N )
	{
		m_nSlotCount = N + 50;
		m_nCount = 0;
		m_slots = new Slot[ m_nSlotCount ];
		memset( m_slots, 0, m_nSlotCount * sizeof(Slot) );
		for ( unsigned i = 0; i < N; ++i )
			add( initArray[i].key, initArray[i].value );
	}

	~c_hash()
	{
		clear();		
		delete[] m_slots;
	}

	c_hash( const c_hash& src )
	{
		m_nSlotCount = src.m_nSlotCount;
		m_nCount = 0;
		m_slots = new Slot[m_nSlotCount];
		memset( m_slots, 0, m_nSlotCount * sizeof(Slot) );
		src.for_each( *this );
	}

	c_hash( c_hash&& src )
	{
		m_nCount = src.m_nCount;			src.m_nCount = 0;
		m_nSlotCount = src.m_nSlotCount;	src.m_nSlotCount = 0;
		m_slots = src.m_slots;				src.m_slots = nullptr;
		m_nodePool.swap( src.m_nodePool );
	}

	template< int _BLOCK_SIZE, int _ALIGN >
	c_hash( const c_hash<T,hashPr,_BLOCK_SIZE,_ALIGN>& src )
	{
		m_nSlotCount = src.m_nSlotCount;
		m_nCount = 0;
		m_slots = new Slot[m_nSlotCount];
		memset( m_slots, 0, m_nSlotCount * sizeof(Slot) );
		src.for_each( *this );
	}

	void swap( c_hash& src )
	{
		std::swap( m_nCount, src.m_nCount );
		std::swap( m_nSlotCount, src.m_nSlotCount );
		std::swap( m_slots, src.m_slots );
		m_nodePool.swap( src.m_nodePool );
	}

	template< int _BLOCK_SIZE, int _ALIGN >
	c_hash& operator = ( const c_hash<T,hashPr,_BLOCK_SIZE,_ALIGN>& src )
	{
		if ( this != &src )
		{
			clear();
			src.for_each( *this );
		}
		return *this;
	}

	c_hash& operator = ( c_hash&& src )
	{
		swap( src );
		return *this;
	}

	size_t size() const		
	{ 
		return m_nCount; 
	}

	T* lookup( const Key& key ) const
	{
		unsigned hashKey = hashPr::getHashKey( key );
		Slot& slot = m_slots[ hashKey % m_nSlotCount ];
		if ( slot == nullptr ) 
			return nullptr;
		
		Node* node = getNode( slot, hashKey, key );
		return node ? &node->value : nullptr;
	}

	bool lookup( const Key& key, T& value ) const
	{
		const T* p = lookup( key );
		if ( p == nullptr ) 
			return false;
		value = *p;
		return true;
	}

	bool has( const Key& key ) const
	{
		return lookup( key ) != nullptr;
	}

	void clear()
	{
		if ( m_slots )
			memset( m_slots, 0, m_nSlotCount * sizeof(Slot) );
		m_nodePool.clear();
		m_nCount = 0;
	}

	T* add( const Key& key, const T& value )
	{
		unsigned hashKey = hashPr::getHashKey( key );
		Node* node = addNode( m_slots[ hashKey % m_nSlotCount ], hashKey, key, &value );
		return (node == nullptr) ? nullptr : &node->value;
	}

	T* modify( const Key& key, const T& value )
	{
		unsigned hashKey = hashPr::getHashKey( key );
		Slot& slot = m_slots[ hashKey % m_nSlotCount ];
		if( slot == nullptr )
			return nullptr;

		Node* node = getNode( slot, hashKey, key );
		if ( node )
		{
			node->value = value;
			return &node->value;
		}
		return nullptr;
	}

	bool remove( const Key& key )
	{
		unsigned hashKey = hashPr::getHashKey( key );
		Slot& slot = m_slots[ hashKey % m_nSlotCount ];
		if ( slot == nullptr ) 
			return false;

		return removeNode( slot, hashKey, key );
	}

	T& operator [] ( const Key& key )
	{
		unsigned hashKey = hashPr::getHashKey( key );
		
		while ( true )
		{
			Slot& slot = m_slots[ hashKey % m_nSlotCount ];
			if ( slot != nullptr ) 
			{
				Node* node = getNode( slot, hashKey, key );
				if ( node )
					return node->value;
			}

			addNode( slot, hashKey, key, nullptr );
		}
	}

	template< typename Pr >
	int for_each( Pr& pred ) const
	{
		int count = 0;
		bool bTerminate = false;
		for ( unsigned index = 0; index < m_nSlotCount; ++index )
		{
			for ( Node* pNode = m_slots[ index ]; pNode != nullptr; pNode = pNode->pNext )
			{
				pred( pNode->key, pNode->value, bTerminate );
				++count;

				if ( bTerminate )
					return count;
			}
		}
		return count;
	}

	template< typename Pr >
	int for_each( const Pr& pred ) const
	{
		return for_each( const_cast< Pr& >( pred ) );
	}

private:

	struct Node
	{
		Node( unsigned h, const Key& k, Node* pNext ) : hashKey( h ), key( k ), pNext( pNext )  {}
		Node( const T& v, unsigned h, const Key& k, Node* pNext ) : value( v ), hashKey( h ), key( k ), pNext( pNext )  {}

		unsigned hashKey;
		T value;
		Key key;
		Node* pNext;
	};

	typedef Node* Slot;
	
	static inline bool isKeyLess( unsigned hashKey1, const Key& key1, unsigned hashKey2, const Key& key2 )
	{
		return c_hash_detail::CompKey< Key, hashPr, hashPr::uniqueHashKey >::call( hashKey1, key1, hashKey2, key2 );
	}

	Node* findLowerBound( Node* pNode, unsigned hashKey, const Key& key, Node*& pPrev ) const
	{
		Node* p = pPrev;
		while ( pNode != nullptr )
		{
			if ( !isKeyLess( pNode->hashKey, pNode->key, hashKey, key ) )
				break;
			p = pNode;
			pNode = pNode->pNext;
		}
		pPrev = p;
		return pNode;
	}

	bool removeNode( Slot& slot, unsigned hashKey, const Key& key )
	{
		Node* pPrev = nullptr;
		Node* pNode = findLowerBound( slot, hashKey, key, pPrev );
		if ( pNode == nullptr || isKeyLess( hashKey, key, pNode->hashKey, pNode->key ) ) 
			return false;

		if ( pPrev == nullptr )
			slot = pNode->pNext;
		else
			pPrev->pNext = pNode->pNext;

		m_nodePool.free( pNode );
		--m_nCount;
		if ( m_nCount * 8 + 50 < m_nSlotCount )
			rehash( m_nSlotCount / 4 + 1 );
		return true;
	}

	Node* addNode( Slot& slot, unsigned hashKey, const Key& key, const T* pValue )
	{
		Node* pPrev = nullptr;
		Node* pNode = findLowerBound( slot, hashKey, key, pPrev );
		if ( pNode != nullptr && !isKeyLess( hashKey, key, pNode->hashKey, pNode->key ) )
			return nullptr;

		Node* node = pValue != nullptr ? m_nodePool.alloc( *pValue, hashKey, key, pNode )
									: m_nodePool.alloc( hashKey, key, pNode );
		if ( pPrev == nullptr )
			slot = node;
		else
			pPrev->pNext = node;

		++m_nCount;
		if ( m_nCount > m_nSlotCount * 4 )
			rehash( m_nSlotCount * 8 + 1 );
		return node;
	}

	Node* getNode( Slot& slot, unsigned hashKey, const Key& key ) const
	{	
		Node* pPrev = nullptr;
		Node* pNode = findLowerBound( slot, hashKey, key, pPrev );

		if ( pNode == nullptr || isKeyLess( hashKey, key, pNode->hashKey, pNode->key ) )
			return nullptr;

		return pNode;
	}

	void rehash( unsigned nNewSlotCount )
	{
		if ( m_nSlotCount == nNewSlotCount )
			return;

		Slot* newSlots = new Slot[ nNewSlotCount ];
		memset( newSlots, 0, nNewSlotCount * sizeof(Slot) );

		for ( unsigned index = 0; index < m_nSlotCount; ++index )
		{
			for ( Node* pNode = m_slots[ index ]; pNode != nullptr; )
			{
				Slot& slot = newSlots[ pNode->hashKey % nNewSlotCount ];
				Node* pPrev = nullptr;
				Node* pNext = findLowerBound( slot, pNode->hashKey, pNode->key, pPrev );
				if ( pPrev )
					pPrev->pNext = pNode;
				else
					slot = pNode;
				pPrev = pNode->pNext;
				pNode->pNext = pNext;
				pNode = pPrev;
			}
		}
		delete [] m_slots;
		m_slots = newSlots;
		m_nSlotCount = nNewSlotCount;
	}

	bool operator () ( const Key& key, const T& value ) 
	{
		add( key, value );
		return false;
	}

	size_t m_nCount;
	unsigned m_nSlotCount;
	Slot* m_slots;
	c_objheap< Node, BLOCK_SIZE, ALIGN > m_nodePool;
};

namespace c_hash_def
{
	template< typename T >
	struct hashPr_basic
	{
		typedef T Key;
		enum { uniqueHashKey = sizeof(T) <= sizeof(unsigned) };

		static inline unsigned getHashKey( Key key )
		{
			return unsigned( key );
		}

		static inline bool isLess( Key key1, Key key2 )
		{
			return key1 < key2;
		}
	};

	// Robert Jenkins' 32 bit integer hash function
	// sonee - 20120917
	template< typename T >
	struct hashPr_RobertJenkins32BitInteger
	{
		typedef T Key;
		enum { uniqueHashKey = sizeof(T) <= sizeof(unsigned) };

		static inline unsigned getHashKey( Key key )
		{
			key = (key+0x7ed55d16) + (key<<12);
			key = (key^0xc761c23c) ^ (key>>19);
			key = (key+0x165667b1) + (key<<5);
			key = (key+0xd3a2646c) ^ (key<<9);
			key = (key+0xfd7046c5) + (key<<3);
			key = (key^0xb55a4f09) ^ (key>>16);

			return key;
		}

		static inline bool isLess( Key key1, Key key2 )
		{
			return key1 < key2;
		}
	};

	template< typename T >
	struct hashPr_raw
	{
		typedef T Key;
		enum { uniqueHashKey = false };

		static inline unsigned getHashKey( const Key& key )
		{
			unsigned hashKey = 0;
			const unsigned char* p = reinterpret_cast< const unsigned char* >( &key );
			for ( int i = 0; i < sizeof(Key); ++i )
				hashKey = hashKey * 32 - hashKey + p[i];
			return hashKey;
		}

		static inline bool isLess( const Key& key1, const Key& key2 )
		{
			return ::memcmp( &key1, &key2, sizeof(Key) ) < 0;
		}
	};

	template< typename CHAR, typename comp_traits >
	struct hashPr_sz
	{
		typedef const CHAR* Key;
		enum { uniqueHashKey = false };
		
		static inline unsigned getHashKey( const Key& key )
		{
			unsigned hashKey = 0;
			for ( const CHAR *str = key; *str; str++ ) 
				hashKey = hashKey * 32 - hashKey + comp_traits::repr( *str );
			return hashKey;
		}

		static inline bool isLess( const Key& k1, const Key& k2 )
		{
			return comp_traits::compare( k1, k2 ) < 0;
		}
	};

	template< typename STRING, typename comp_traits >
	struct hashPr_str
	{
		typedef STRING Key;
		enum { uniqueHashKey = false };

		template< typename CHAR >
		static inline unsigned _getHashKey( const CHAR* pStr )
		{
			unsigned hashKey = 0;
			for ( ; *pStr; pStr++ ) 
				hashKey = hashKey * 32 - hashKey + comp_traits::repr( *pStr );
			return hashKey;
		}

		static inline unsigned getHashKey( const Key& keystr )
		{
			return _getHashKey( keystr.c_str() );
		}

		static inline bool isLess( const Key& str1, const Key& str2 )
		{
			return comp_traits::compare( str1.c_str(), str2.c_str() ) < 0;
		}
	};

	struct case_tr
	{
		template< typename CH >
		static inline int tstrcmp( const CH* p, const CH* q )
		{
			for (; *p && *p == *q; ++p, ++q ) {}
			return int(*q) - int(*p);
		}

		static inline int compare( const char* p, const char* q )        { return tstrcmp( p, q ); }
		static inline int compare( const wchar_t* p, const wchar_t* q )  { return tstrcmp( p, q ); }
		static inline unsigned repr( unsigned c )                        { return c; }
	};

	struct nocase_tr
	{
		template< typename CH >
		static inline int tstricmp( const CH* p, const CH* q )
		{
			for (; *p && repr(*p) == repr(*q); ++p, ++q ) {}
			return int(repr(*q)) - int(repr(*p));
		}

		static inline int compare( const char* p, const char* q )        { return tstricmp( p, q ); }
		static inline int compare( const wchar_t* p, const wchar_t* q )  { return tstricmp( p, q ); }
		static inline unsigned repr( char c )                            { return c >= 'A' && c <= 'Z' ? c - ('A'-'a') : c; }
		static inline unsigned repr( wchar_t c )                         { return c >= L'A' && c <= L'Z' ? c - (L'A'-L'a') : c; }
	};

	typedef hashPr_basic< int >                    hashPr_int;
	typedef hashPr_sz< char, case_tr >             hashPr_pchar;
	typedef hashPr_sz< char, nocase_tr >           hashPr_pchar_nocase;
	typedef hashPr_sz< wchar_t, case_tr >          hashPr_pwchar;
	typedef hashPr_sz< wchar_t, nocase_tr >        hashPr_pwchar_nocase;
	typedef hashPr_str< std::string, case_tr >     hashPr_string;
	typedef hashPr_str< std::string, nocase_tr >   hashPr_string_nocase;
	typedef hashPr_str< std::wstring, case_tr >    hashPr_wstring;
	typedef hashPr_str< std::wstring, nocase_tr >  hashPr_wstring_nocase;
}

#endif
