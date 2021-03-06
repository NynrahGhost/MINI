#pragma once
#include <string>
#include <stdexcept>
#include <unordered_map>
#include "compile_options.h"

typedef char int8;
typedef short int int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef float float32;
typedef double float64;
typedef long double float128;


#if ENCODING == 8
typedef std::basic_string<char, std::char_traits<char>, std::allocator<char>> String;
typedef char charT;
#define T(STRING) STRING

#elif ENCODING == 16
//typedef std::basic_string<char16_t, std::char_traits<char16_t>, std::allocator<char16_t>> String;
typedef std::u16string String;
typedef char16_t charT;
#define T(STRING) u##STRING

#elif ENCODING == 32
typedef std::basic_string<char32_t, std::char_traits<char32_t>, std::allocator<char32_t>> String;
typedef char32_t charT;
#define T(STRING) U##STRING

#else
#error Incorrect encoding size specified
#endif

uint64 murmurHash64(const void* key, int len, unsigned int seed);

template <typename _T>
struct Array {
	const static size_t INITIAL_CAPACITY = 16;

	_T* content;
	size_t max_index;
	size_t capacity;

	Array() {
		content = new _T[INITIAL_CAPACITY];
		max_index = -1;
		capacity = INITIAL_CAPACITY;
	}

	Array(size_t initial_capacity) {
		content = new _T[initial_capacity];
		max_index = -1;
		capacity = initial_capacity;
	}

	~Array() {
		delete[] content;
	}

	void init() {
		content = new _T[INITIAL_CAPACITY];
		max_index = -1;
		capacity = INITIAL_CAPACITY;
	}

	void init(size_t initial_capacity) {
		content = new _T[initial_capacity];
		max_index = -1;
		capacity = initial_capacity;
	}

	inline Array* resize(size_t size) {
		_T* ptr = new _T[size];
		memcpy(ptr, content, sizeof(_T) * (max_index + 1));
		delete[] content;
		content = ptr;
		return this;
	}

	inline Array* add(_T element) {
		if (++max_index != capacity)
		{
			content[max_index] = element;

			return this;
		}
		else
		{
			_T* ptr = new _T[capacity <<= 1];
			memcpy(ptr, content, sizeof(_T) * (max_index + 1));
			delete[] content;
			content = ptr;
			content[max_index] = element;
			return this;
		}
	}

	inline void slice_r(size_t index) {
		for (size_t i = max_index - index; i <= max_index; ++i)
			content[i].~_T();
		max_index -= index;
	}


	inline _T get_s(size_t index) {	//Safe version
		if (index > max_index)
			throw std::out_of_range();
		return content[index];
	}

	inline _T get(size_t index) {
		return content[index];
	}

	inline _T get_r(size_t index) {
		return content[max_index - index];
	}

	inline _T& at(size_t index) {
		return content[index];
	}

	inline _T& at_r(size_t index) {
		return content[max_index - index];
	}

	inline void set(size_t index, _T element) {
		content[index] = element;
	}

	inline void set_r(size_t index, _T element) {
		content[max_index - index] = element;
	}


	inline _T& operator=(_T& type) {
		this->content = type.content;
		this->capacity = type.capacity;
		this->max_index = type.max_index;
		return this;
	}

	inline _T& operator[](size_t index) {
		return content[index];
	}
};

template <typename _T>
struct Queue : public Array<_T> {
	size_t min_index;

	Queue() : Array() {
		min_index = -1;
	}

	inline _T dequeue(){
		if (min_index == this->max_index)
		{
			_T result = this->content[min_index];
			min_index = -1;
			this->max_index = -1;
			return result;
		}
		return this->content[++min_index];
	}
};

struct Span : Array<uint8> {

	const static size_t INITIAL_CAPACITY = 1024;

	Span() {
		content = (uint8*)malloc(INITIAL_CAPACITY);
		max_index = 0;
		capacity = INITIAL_CAPACITY;
	}

	Span(size_t initial_capacity) {
		content = (uint8*)malloc(initial_capacity);
		max_index = 0;
		capacity = initial_capacity;
	}

	inline bool probe_amount(size_t size) {
		return capacity - max_index > size;
	}

	template<typename _V>
	void add(_V element) {
		if ((max_index + sizeof(_V)) < capacity)
		{
			*(_V*)(content+max_index) = element;
			max_index += sizeof(_V);
		}
		else
		{
			content = (uint8*)realloc(content, capacity <<= 1);
			/*uint8* ptr = new uint8[capacity <<= 1];
			memcpy(ptr, content, sizeof(uint8) * (max_index + 1));
			delete[] content;
			content = ptr;*/

			*(_V*)(content + max_index) = element;
			max_index += sizeof(_V);
		}
	}

	template<typename _V>
	inline _V& at(size_t index) {
		return *(_V*)(content + index);
	}

	template<typename _V>
	inline _V get(size_t index) {
		return *(_V*)(content + index);
	}

	//Insertion within occupied memory
	template<typename _V>
	void insert(_V element, size_t position) {
		if (max_index + sizeof(_V) < capacity) {
			memcpy(content + position + sizeof(_V), content + position, max_index - position);
			*(_V*)(content + position) = element;
			max_index += sizeof(_V);
		}
		else
		{
			uint8* ptr = new uint8[capacity <<= 1];
			memcpy(ptr, content, sizeof(uint8) * (position + 1));	//TODO: check why I put +1 here
			memcpy(ptr+position+sizeof(_V), content+position+sizeof(_V), sizeof(uint8) * (max_index - position + 1));
			delete[] content;
			content = ptr;

			*(_V*)(content + position) = element;
			max_index += sizeof(_V);
		}
	}

	inline void replace(void* source, size_t sourceSize, size_t destination, size_t destinationSize) {
		//TODO: make safe


		memcpy(content + destination, content + destination + (sourceSize - destinationSize), max_index - (destination + (sourceSize - destinationSize)));
		memcpy(content + destination, source, sourceSize);
		max_index += sourceSize - destinationSize;
	}

	inline void erase(size_t shift, size_t size) {
		memcpy(content + shift, content + shift + size, max_index - (shift + size));
		max_index -= shift;
	}

	inline void move_relative(size_t position, size_t size, int64 shift) {
		memcpy(content + position + shift, content + position, size);
	}

	inline void move_relative(size_t position, int64 shift) {
		memcpy(content + position + shift, content + position, max_index-position);
	}

	inline void move_absolute(size_t position, size_t size, size_t new_position) {
		memcpy(content + new_position, content + position, size);
	}

	inline void move_absolute(size_t position, size_t new_position) {
		memcpy(content + new_position, content + position, max_index-position);
	}
};

template<typename _Key, typename _Value>
using Table = std::unordered_map<_Key, _Value>;

/*
template<typename _Key, typename _Value>
struct Dictionary {

	Bucket<_Key, _Value>* buckets;
	uint32(*) (_Key) hash_function;

	template<typename _Key, typename _Value>
	struct Bucket {

	}
};*/