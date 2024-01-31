#pragma once

#include <algorithm>


/*
// A heightmap is simply a 2D array that holds float.
*/
#define ASSERT_IS_IN_MAP(width, height, x, y) { if (y*width + x >= width*height || x < 0 || y < 0) return 0.f;}

class Heightmap {

public:

	Heightmap() : m_width(0), m_height(0)
	{
		m_values = nullptr;
	}

	Heightmap(int width, int height) : m_width(width), m_height(height)
	{
		m_values = new float[m_width * m_height];
	}

	Heightmap(const Heightmap& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		m_values = new float[m_width * m_height];
		std::copy(other.begin(), other.end(), m_values);


	}

	Heightmap& operator=(const Heightmap& other) {
		if (m_values) delete[] m_values;
		m_width = other.m_width;
		m_height = other.m_height;
		m_values = new float[m_width * m_height];
		std::copy(other.begin(), other.end(), m_values);
		return *this;

	}

	~Heightmap() { delete[] m_values; }

public:

	float getAt(int u, int v) const {
		ASSERT_IS_IN_MAP(m_width, m_height, u, v);
		return m_values[v * m_width + u];
	}
	float setAt(int u, int v, float value) {
		ASSERT_IS_IN_MAP(m_width, m_height, u, v);
		m_values[v * m_width + u] = value;
	}



public:
	int getWidth() const { return m_width; }
	int getHeigth() const { return m_height; }
	float getMaxHeight() const { return *std::max_element(m_values, m_values + (m_width * m_height)); }

public:
	struct Iterator {

		using value_type = float;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = float*;  // or also value_type*
		using reference = float&;  // or also value_type&
		

		Iterator(pointer ptr) : m_ptr(ptr) {}
		reference operator*() const { return *m_ptr; }
		pointer operator->() { return m_ptr; }

		// Prefix increment
		Iterator& operator++() { m_ptr++; return *this; }

		// Postfix increment
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

	private:

		pointer m_ptr;
	};

	Iterator begin() const { return Iterator(&m_values[0]); }
	Iterator end() const { return Iterator(&m_values[m_width * m_height]); }


private:

	int m_width;
	int m_height;

	float* m_values;


};



