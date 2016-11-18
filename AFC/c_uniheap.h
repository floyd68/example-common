#ifndef _UNISIZE_HEAP_V2_2_INCLUDED_
#define _UNISIZE_HEAP_V2_2_INCLUDED_

/*
 *     Fast & low overhead fixed unit size heap, suitable for small object allocation
 *
 *          1998.12.16 by Young-Hyun Joo (v1)
 *
 *          2006.10.4 by Young-Hyun Joo (v2), with optimization ideas from 
 *                                                     Loki small object pool and XMemoryPool
 */

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>		 // for memset

// ----------------------------------------------------------------

namespace c_uniheap_detail
{
	template< bool bByte > struct CalcIndexType { typedef unsigned char type; };
	template<> struct CalcIndexType< false > { typedef unsigned short type; };
}

template< int UNIT_SIZE, unsigned short BLOCK_SIZE = 128, bool bSAFE_INDEX = false >
class c_uniheap 
{
	struct TBlock;

	// UNIT_SIZE == 1 이고 BLOCK_SIZE < 256 인 경우는 1 byte index 를, 아니면 2 byte index 를 사용한다

	typedef typename c_uniheap_detail::CalcIndexType< UNIT_SIZE == 1 && BLOCK_SIZE < 256 >::type TIndex;
	enum { USIZE = (UNIT_SIZE == 1 && sizeof(TIndex) == sizeof(short)) ? sizeof(short) : UNIT_SIZE };
	enum { ULONG_BITS = 8*sizeof(unsigned int) }; 

public:

	c_uniheap() 
	{
		assert( UNIT_SIZE > 0 );

		m_pFreeBlock = new TBlock();
		m_blocks.push_back( m_pFreeBlock );
	}

	~c_uniheap()
	{
		typename std::vector< TBlock* >::iterator it;
		for ( it = m_blocks.begin(); it != m_blocks.end(); it++ ) 
			delete *it;
	}

	void clear()
	{
		typename std::vector< TBlock* >::iterator it;
		for ( it = m_blocks.begin()+1; it != m_blocks.end(); it++ ) 
			delete *it;

		m_blocks.resize( 1 );
		m_pFreeBlock = m_blocks.front();
		m_pFreeBlock->clear();
	}

	void swap( c_uniheap< UNIT_SIZE, BLOCK_SIZE, bSAFE_INDEX >& aHeap )
	{
		std::swap( m_pFreeBlock, aHeap.m_pFreeBlock );
		m_blocks.swap( aHeap.m_blocks );
	}

	int capacity() const
	{
		return BLOCK_SIZE * m_blocks.size();
	}

	int count() const
	{
		int c = 0;
		typename std::vector< TBlock* >::const_iterator it;
		for ( it = m_blocks.begin(); it != m_blocks.end(); it++ ) 
			c += (*it)->count;
		return c;
	}

	void* alloc()
	{
		TBlock* pFreeBlock = find_free_block();

		return pFreeBlock ? alloc_from_block( pFreeBlock ) : nullptr;
	}

	bool free( void* ptr )
	{
		int index = find_ptr_block_index( ptr );
		if ( index < int(m_blocks.size()) ) 
		{
			TBlock* pBlock = m_blocks[ index ];

			assert( (reinterpret_cast<char*>(ptr) - pBlock->buf) % USIZE == 0 );

			if ( m_pFreeBlock != pBlock && pBlock->firstFree == BLOCK_SIZE )
			{
				pBlock->pNextFreeBlock = m_pFreeBlock->pNextFreeBlock;
				m_pFreeBlock->pNextFreeBlock = pBlock;
			}

			free_from_block( ptr, pBlock );

			if ( bSAFE_INDEX && index != int(m_blocks.size())-1 )
				return true;

			if ( m_pFreeBlock != pBlock && pBlock->count == 0 && m_blocks.size() > 1 ) 
			{
				for ( TBlock* p = m_pFreeBlock; p != nullptr; p = p->pNextFreeBlock )
				{
					if ( p->pNextFreeBlock == pBlock ) 
					{
						p->pNextFreeBlock = pBlock->pNextFreeBlock;
						break;
					}
				}

				delete pBlock;
				m_blocks.erase( m_blocks.begin() + index );
			}

			return true;
		}

		// error : ptr is not valid!!!
		return false;
	}

	int get_index( void* ptr ) const
	{
		assert( bSAFE_INDEX );
		int index = find_ptr_block_index( ptr );
		if ( index >= int(m_blocks.size()) ) 
			return -1;

		return index * BLOCK_SIZE + (reinterpret_cast< char* >( ptr ) - m_blocks[ index ]->buf) / UNIT_SIZE;
	}

	void* get_ptr( int index ) const
	{
		assert( bSAFE_INDEX );
		if ( index < 0 || index >= BLOCK_SIZE * m_blocks.size() )
			return nullptr;

		return m_blocks[ index / BLOCK_SIZE ]->buf + (index % BLOCK_SIZE) * UNIT_SIZE;
	}

	struct cursor
	{
		TBlock* const* 	pLastBlock;
		int				lastIndex;
		unsigned int	flagOccupied[ (BLOCK_SIZE + ULONG_BITS-1) / ULONG_BITS ];
	};

	void* get_first_unit( cursor& cr ) const
	{
		size_t c = m_blocks.size();
		for ( cr.pLastBlock = &m_blocks.front(); c--; cr.pLastBlock++ ) 
		{
			make_occupied_flag( *cr.pLastBlock, cr.flagOccupied );
			cr.lastIndex = get_next_occupied_index_from_block( *cr.pLastBlock, cr.flagOccupied, 0 );
			if ( cr.lastIndex != -1 ) 
				return (*cr.pLastBlock)->buf + cr.lastIndex * USIZE;
		}

		return nullptr;
	}

	void* get_next_unit( cursor& cr ) const
	{
		if ( cr.pLastBlock - &m_blocks.front() == int(m_blocks.size()) ) 
			return nullptr;

		cr.lastIndex = get_next_occupied_index_from_block( *cr.pLastBlock, cr.flagOccupied, cr.lastIndex+1 );
		if ( cr.lastIndex != -1 ) 
			return (*cr.pLastBlock)->buf + cr.lastIndex * USIZE;

		while ( ++cr.pLastBlock - &m_blocks.front() != int(m_blocks.size()) ) 
		{
			if ( (*cr.pLastBlock)->count == 0 )
				continue;

			make_occupied_flag( *cr.pLastBlock, cr.flagOccupied );
			cr.lastIndex = get_next_occupied_index_from_block( *cr.pLastBlock, cr.flagOccupied, 0 );
			if ( cr.lastIndex != -1 ) 
				return (*cr.pLastBlock)->buf + cr.lastIndex * USIZE;
		}

		return nullptr;
	}

private:

	c_uniheap( const c_uniheap& );
	c_uniheap& operator = ( const c_uniheap& );

	struct TBlock 
	{
		char			buf[ BLOCK_SIZE * USIZE ];
		unsigned short	firstFree;
		unsigned short	count;
		TBlock*			pNextFreeBlock;

		TBlock() { clear(); }

		void clear()
		{
			firstFree = 0;
			count = 0;
			for ( int i = 0; i < int(BLOCK_SIZE); i++ ) 
				*reinterpret_cast< TIndex* >( buf + i*USIZE ) = TIndex(i+1);
			pNextFreeBlock = nullptr;
		}
	};

	TBlock* find_free_block()
	{
		while ( m_pFreeBlock )
		{
			if ( m_pFreeBlock->count < BLOCK_SIZE )
				return m_pFreeBlock;

			TBlock* pBlock = m_pFreeBlock->pNextFreeBlock;
			m_pFreeBlock->pNextFreeBlock = nullptr;
			m_pFreeBlock = pBlock;
		}

		// all blocks are full, so we must make a new empty block and allocate from it.

		m_pFreeBlock = new TBlock();
		if ( m_pFreeBlock )
			m_blocks.insert( std::lower_bound( m_blocks.begin(), m_blocks.end(), m_pFreeBlock ), m_pFreeBlock );

		return m_pFreeBlock;
	}

	static inline bool ptr_in_range( void* ptr, char* base, int len )
	{
		return base <= ptr && ptr < base + len;
	}

	int find_ptr_block_index( void *ptr ) const
	{
		int begin = 0, end = int(m_blocks.size());

		// binary search for block

		while ( begin != end )
		{
			int mid = ( begin + end ) / 2;
			if( mid == begin ) 
				break;

			if ( m_blocks[ mid ]->buf <= ptr ) 
				begin = mid;
			else
				end = mid;
		}

		if ( begin < int(m_blocks.size()) && ptr_in_range( ptr, m_blocks[ begin ]->buf, BLOCK_SIZE * USIZE ) )
			return begin;

		return int(m_blocks.size());
	}

	void* alloc_from_block( TBlock* block ) 
	{
		int firstFree = block->firstFree;

		assert( firstFree >= 0 && firstFree < BLOCK_SIZE );

		void* ptr = block->buf + firstFree * USIZE;
		block->firstFree = *reinterpret_cast< TIndex* >( ptr );

		// memory marking for debugging

		assert( (memset( ptr, 0xcd, USIZE ), block->count < BLOCK_SIZE) );

		++block->count;
		return ptr;
	}

	void free_from_block( void* ptr, TBlock* block )
	{
		int index = int(reinterpret_cast<char*>(ptr) - block->buf) / USIZE;

		*reinterpret_cast< TIndex* >( block->buf + index * USIZE ) = TIndex(block->firstFree);
		block->firstFree = (unsigned short)index;
		--block->count;

		// memory marking for debugging

		assert( (memset( reinterpret_cast<char*>(ptr) + sizeof(TIndex), 0xdd, USIZE - sizeof(TIndex) ), block->count >= 0) );
	}

	void make_occupied_flag( const TBlock* block, unsigned int* flagOccupied ) const
	{
		memset( flagOccupied, ~0, (BLOCK_SIZE + ULONG_BITS-1) / ULONG_BITS * sizeof(unsigned int) );
		for ( int index = block->firstFree; index < BLOCK_SIZE; 
			  index = *reinterpret_cast< const TIndex* >( block->buf + index * USIZE ) )
		{
			flagOccupied[ index / ULONG_BITS ] &= ~(1 << (index % ULONG_BITS));
		}
	}

	int get_next_occupied_index_from_block( const TBlock* block, unsigned int* flagOccupied, int start_index ) const
	{
		block;
		flagOccupied += start_index / ULONG_BITS;
		unsigned int mask = 1 << (start_index % ULONG_BITS);

		for (; start_index < BLOCK_SIZE; start_index++ )
		{
			if ( *flagOccupied == 0 ) 
				start_index += ULONG_BITS-1, mask = 1U << (ULONG_BITS-1);
			else if ( *flagOccupied & mask )
				return start_index;

			mask *= 2;
			if ( mask == 0 )
				mask = 1, ++flagOccupied;
		}
		return -1;
	}

	std::vector< TBlock* >	m_blocks;
	TBlock*					m_pFreeBlock;
};


// ----------------------------------------------------------------

#ifdef C_OBJHEAP_NO_EXCEPTION
	#define C_OBJHEAP_TRY	if (true)
	#define C_OBJHEAP_CATCH	else
	#define C_OBJHEAP_THROW		
#else
	#define C_OBJHEAP_TRY	try
	#define C_OBJHEAP_CATCH	catch(...)
	#define C_OBJHEAP_THROW	throw
#endif

template< typename T, unsigned short BLOCK_SIZE = 128, int ALIGN = 1, bool bSAFE_INDEX = false >
class c_objheap
{
public:

	~c_objheap()
	{
		cursor cr;
		for ( T* p = get_first_unit( cr ); p != nullptr; p = get_next_unit( cr ) )
			p->T::~T();
	}

	void clear()
	{
		cursor cr;
		for ( T* p = get_first_unit( cr ); p != nullptr; p = get_next_unit( cr ) )
			p->T::~T();
		m_heap.clear();
	}

	void swap( c_objheap< T, BLOCK_SIZE, ALIGN, bSAFE_INDEX >& aHeap ) 
	{
		m_heap.swap( aHeap.m_heap );
	}

	int capacity() const			{ return m_heap.capacity(); }
	int count() const				{ return m_heap.count(); }

	void free( T* p )
	{
		p->T::~T();
		m_heap.free( p );
	}

	T* alloc()
	{
		T* p = reinterpret_cast< T* >( m_heap.alloc() );
		if ( p )
		{
			C_OBJHEAP_TRY { new (p) T; }
			C_OBJHEAP_CATCH { free(p); C_OBJHEAP_THROW; }
		}
		return p;
	}

	template< typename T1 >
	T* alloc( const T1& t1 )
	{
		T* p = reinterpret_cast< T* >( m_heap.alloc() );
		if ( p )
		{
			C_OBJHEAP_TRY { new (p) T(t1); }
			C_OBJHEAP_CATCH	{ free(p); C_OBJHEAP_THROW; }
		}
		return p;
	}

	template< typename T1, typename T2 >
	T* alloc( const T1& t1, const T2& t2 )
	{
		T* p = reinterpret_cast< T* >( m_heap.alloc() );
		if ( p )
		{
			C_OBJHEAP_TRY { new (p) T(t1,t2); }
			C_OBJHEAP_CATCH	{ free(p); C_OBJHEAP_THROW; }
		}
		return p;
	}

	template< typename T1, typename T2, typename T3 >
	T* alloc( const T1& t1, const T2& t2, const T3& t3 )
	{
		T* p = reinterpret_cast< T* >( m_heap.alloc() );
		if ( p )
		{
			C_OBJHEAP_TRY { new (p) T(t1,t2,t3); }
			C_OBJHEAP_CATCH	{ free(p); C_OBJHEAP_THROW; }
		}
		return p;
	}

	template< typename T1, typename T2, typename T3, typename T4 >
	T* alloc( const T1& t1, const T2& t2, const T3& t3, const T4& t4 )
	{
		T* p = reinterpret_cast< T* >( m_heap.alloc() );
		if ( p )
		{
			C_OBJHEAP_TRY { new (p) T(t1,t2,t3,t4); }
			C_OBJHEAP_CATCH	{ free(p); C_OBJHEAP_THROW; }
		}
		return p;
	}

	template< typename T1, typename T2, typename T3, typename T4, typename T5 >
	T* alloc( const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5 )
	{
		T* p = reinterpret_cast< T* >( m_heap.alloc() );
		if ( p )
		{
			C_OBJHEAP_TRY { new (p) T(t1,t2,t3,t4,t5); }
			C_OBJHEAP_CATCH	{ free(p); C_OBJHEAP_THROW; }
		}
		return p;
	}

private:

	template< int U, int A > struct CalcAlignLess	{ enum { val = (U > A/2) ? A : CalcAlignLess< U, A/2 >::val }; };
	template< int U > struct CalcAlignLess< U, 1 >	{ enum { val = U }; };
	template< int U, int A > struct CalcAlign		{ enum { val = (U >= A) ? (U + A-1) / A * A : CalcAlignLess< U, A >::val }; };

	c_uniheap< CalcAlign< sizeof(T), ALIGN >::val, BLOCK_SIZE, bSAFE_INDEX > m_heap;

public:

	typedef typename c_uniheap< CalcAlign< sizeof(T), ALIGN >::val, BLOCK_SIZE, bSAFE_INDEX >::cursor cursor;

	T* get_first_unit( cursor& cr )	const	{ return reinterpret_cast< T* >( m_heap.get_first_unit( cr ) ); }
	T* get_next_unit( cursor& cr ) const	{ return reinterpret_cast< T* >( m_heap.get_next_unit( cr ) ); }

	int get_index( T* ptr ) const			{ return m_heap.get_index( ptr ); }
	void* get_ptr( int index ) const		{ return reinterpret_cast< T* >( m_heap.get_ptr( index ) ); }
};

#endif