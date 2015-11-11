
#ifndef PRECISION_INTEGER_HPP
#define PRECISION_INTEGER_HPP

#include <cstdint>

#include <boost/operators.hpp>

template <int P=10>
class precision_integer :
    boost::operators<precision_integer<P>,
    boost::addable2<precision_integer<P>, int,
    boost::subtractable2<precision_integer<P>, int,
    boost::multipliable2<precision_integer<P>, int,
    boost::dividable2<precision_integer<P>, int,
    boost::less_than_comparable<precision_integer<P>, int>>>>>> {
public:
	precision_integer(int i=0, bool precision=false) :
		val{i * (precision ? 1 : P)} { }
	explicit precision_integer(double d, bool precision=false) :
		val{static_cast<int>(d * (precision ? 1 : P))} { }
	
	precision_integer operator -() {
		return precision_integer{-val, true};
	}
    bool operator <(const precision_integer &pi) const {
        return val < pi.val;
    }
    bool operator <(int i) const {
        return val < i * P;
    }
    bool operator ==(const precision_integer &pi) const {
        return val == pi.val;
    }
    bool operator ==(int i) const {
        return val == i * P;
    }
	precision_integer &operator +=(const precision_integer &other) {
		val += other.val;
		return *this;
	}
	precision_integer &operator +=(int i) {
		val += (i * P);
		return *this;
	}
	precision_integer &operator -=(const precision_integer &other) {
		val -= other.val;
		return *this;
	}
	precision_integer &operator -=(int i) {
		val -= (i * P);
		return *this;
	}
	precision_integer &operator *=(const precision_integer &other) {
		val = (val * other.val) / (P * P);
		return *this;
	}
	precision_integer &operator *=(int i) {
		val *= i;
		return *this;
	}
	precision_integer &operator /=(const precision_integer &other) {
		val = (val * P) / other.val;
		return *this;
	}
	precision_integer &operator /=(int i) {
		val /= i;
		return *this;
	}

	explicit operator uint8_t() const { return static_cast<int32_t>(*this); }
	explicit operator int8_t() const { return static_cast<int32_t>(*this); }
	explicit operator uint16_t() const { return static_cast<int32_t>(*this); }
	explicit operator int16_t() const { return static_cast<int32_t>(*this); }
	explicit operator uint32_t() const { return static_cast<int32_t>(*this); }
	operator int32_t() const { return val / P; }
	
	explicit operator double() const {
		return val / static_cast<double>(P);
	}
private:
	int val;
};

#endif
