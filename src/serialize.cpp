#include <cassert>
#include <algorithm>
#include <vector>

#include <raknet/BitStream.h>
#include <snappy.h>

#include "serialize.hpp"

// RakNet uses native endianness by default. Turn that shit off, yo.
#undef __BITSTREAM_NATIVE_END

namespace m {

class serializer::impl final {
public:
    RakNet::BitStream bs;
};

serializer::serializer() :
    writer_(true),
    impl_{std::make_unique<serializer::impl>()} {
}

serializer::serializer(const uint8_t *data, size_t len) :
    writer_(false),
    impl_{std::make_unique<impl>()} {
    impl_->bs.Write(reinterpret_cast<const char *>(data), len);
}

bool serializer::is_serializing() const {
    return writer_;
}

bool serializer::is_deserializing() const {
    return !writer_;
}

const uint8_t *serializer::data() const {
    return impl_->bs.GetData();
}

size_t serializer::length() const {
    return impl_->bs.GetNumberOfBytesUsed();
}

void serializer::compress() {
    if (!writer_) {
        return;
    }
    std::string dest;
    snappy::Uncompress(reinterpret_cast<const char *>(data()), length(), &dest);
    impl_->bs.Reset();
    impl_->bs.Write(dest.data(), dest.size());
}

void serializer::decompress() {
    if (writer_) {
        return;
    }
    std::string dest;
    snappy::Compress(reinterpret_cast<const char *>(data()), length(), &dest);
    impl_->bs.Reset();
    impl_->bs.Write(dest.data(), dest.size());
}

template <typename T>
serializer &serializer::operator &(T &t) {
    if (writer_) {
        (*impl_) << t;
    } else {
        (*impl_) >> t;
    }
}

}

