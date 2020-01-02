#pragma once
#include <string.h>

class dynamic_bitset {
public:
	dynamic_bitset(int bitCount) { resize(bitCount); }
	~dynamic_bitset() { delete[] m_bits; }

public:
	void reset() {
		memset(m_bits, 0, sizeof(unsigned) * m_nIntegers);
	}

	void resize(int bitCount) {
		int nIntegers = div32(bitCount) + 1;
		if (nIntegers > m_nIntegers) {
			delete[] m_bits;
			m_bits = new unsigned[m_nIntegers = nIntegers];
		}

		memset(m_bits, 0, sizeof(unsigned) * m_nIntegers);
	}

	void set(int index, bool on) {
		if (on) {
			m_bits[div32(index)] |= (1 << mod32(index));
		}
		else {
			m_bits[div32(index)] &= ~(1 << mod32(index));
		}
	}

	bool get(int index) const {
		return !!(m_bits[div32(index)] & (1 << mod32(index)));
	}

private:
	dynamic_bitset(const dynamic_bitset&);
	dynamic_bitset& operator= (const dynamic_bitset&);

private:
	int div32(int x) const { return x >> 5; }
	int mod32(int x) const { return x & (32 - 1); }

private:
	int m_nIntegers = 0;
	unsigned* m_bits = nullptr;
};
