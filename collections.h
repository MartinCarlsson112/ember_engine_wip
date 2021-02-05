#pragma once
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cassert>



template<class T>
struct dynamic_array
{
	typedef uint32_t size_type;
	constexpr static double GROWTH_FACTOR = 1.5;

	void resize(const size_type new_size);
	void push_back(const T& value);

	void dispose()
	{
		if (data)
		{
			free(data);
			data = nullptr;
		}
	}

	T& front() const;
	T& back() const;
	void pop_back();
	T& operator[](size_type i) const {
		return data[i]; 
	}

	size_type size() const {
		return used;
	};

private:
	void resize_internal(const size_type new_size);

	void grow();
	size_type used = 0;
	size_type allocated = 0;
	T* data = nullptr;
};

template<class T>
void dynamic_array<T>::resize_internal(const size_type new_size)
{
	if (new_size > allocated)
	{
		auto temp = (T*)calloc(new_size, sizeof(T));
		if (temp)
		{
			if (data)
			{
				memcpy(temp, data, sizeof(T) * allocated);
				free(data);
			}
			data = temp;
			allocated = new_size;
		}
	}

}

template<class T>
void dynamic_array<T>::resize(const size_type new_size)
{
	resize_internal(new_size);
	used = new_size;
}

template<class T>
void dynamic_array<T>::push_back(const T& value)
{
	if (used + 1 >= allocated){
		grow();
	}
	data[used] = value;
	used++;
}


template<class T>
T& dynamic_array<T>::front() const
{
	return data[0];
}

template<class T>
T& dynamic_array<T>::back() const
{
	return data[used-1];
}

template<class T>
inline void dynamic_array<T>::pop_back()
{
	used--;
}


template<class T>
void dynamic_array<T>::grow()
{
	double val = 1 + static_cast<double>(allocated) * GROWTH_FACTOR;
	size_type new_size = static_cast<size_type>(val);
	resize_internal(new_size);
}

struct queue
{

};