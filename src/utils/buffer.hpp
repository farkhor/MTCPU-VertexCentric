#ifndef BUFFER_CUH
#define BUFFER_CUH

#include <stdexcept>


template <typename T>
class buffer{
private:
	T* ptr;
	size_t nElems;
	void construct(size_t n){
		ptr = new T[ n ];
		nElems = n;
	}
public:
	buffer(){
		nElems = 0;
		ptr = NULL;
	}
	buffer(size_t n){
		construct(n);
	}
	~buffer(){
		if( nElems != 0 )
			delete[] ptr;
	}
	void alloc( size_t n ){
		if( nElems==0 )
			construct(n);
	}
	void free(){
		if( nElems!=0 ) {
			nElems = 0;
			delete[] ptr;
		}
	}
	T& at( size_t index ){
		if( index >= nElems )
			throw std::runtime_error( "The referred element does not exist in the buffer." );
		return ptr[index];
	}
	T& operator[]( size_t index ){	// must be faster compared to 'at'.
		return ptr[ index ];
	}
	T* get_ptr(){
		return ptr;
	}
	size_t size(){
		return nElems;
	}
	size_t sizeInBytes(){
		return nElems * sizeof( T );
	}

};

#endif	//	BUFFER_CUH
