#include <cassert>
#include <algorithm>
#include <vector>

#include <snappy.h>
#include <enet/enet.h>

#include "serialize.hpp"

static inline uint32_t host_to_network_float(float f) {
    return htonl(*reinterpret_cast<uint32_t *>(&f));
}

static inline float network_to_host_float(uint32_t i) {
    i = ntohl(i);
    return *reinterpret_cast<float *>(&i);
}

namespace m {

class serializer::impl final {
public:
    impl() : pos{ 0 } {}

    bool empty() const { return chunk.empty(); }
    size_t size() const { return chunk.size(); }
    const uint8_t *data() const { return chunk.data(); }
    const uint8_t *remaining_data() const { return data() + pos; }
    size_t remaining_size() const { return size() - pos; }
    bool can_read(size_t amount = 1) const { return remaining_size() >= amount; }

    void reread() { pos = 0; }
    void rewrite() { chunk.clear(); }

    void load(const uint8_t *data, size_t len) {
        std::copy_n(data, len, std::back_inserter(chunk));
    }

    impl &operator <<(float f) {
        *this << host_to_network_float(f);
        return *this;
    }
    impl &operator <<(bool b) {
        chunk.push_back(static_cast<uint8_t>(b));
        return *this;
    }
    impl &operator <<(uint8_t i) {
        chunk.push_back(i);
        return *this;
    }
    impl &operator <<(int8_t i) {
        *this << static_cast<uint8_t>(i);
        return *this;
    }
    impl &operator <<(uint16_t i) {
        auto v = htons(i);
        chunk.push_back((v >> 0) & 0xff);
        chunk.push_back((v >> 8) & 0xff);
        return *this;
    }
    impl &operator <<(int16_t i) {
        *this << static_cast<uint16_t>(i);
        return *this;
    }
    impl &operator <<(uint32_t i) {
        auto v = htonl(i);
        chunk.push_back((v >> 0) & 0xff);
        chunk.push_back((v >> 8) & 0xff);
        chunk.push_back((v >> 16) & 0xff);
        chunk.push_back((v >> 24) & 0xff);
        return *this;
    }
    impl &operator <<(int32_t i) {
        *this << static_cast<uint32_t>(i);
        return *this;
    }

    impl &operator >>(float &f) {
        assert(can_read(sizeof(uint32_t)));
        uint32_t i;
        *this >> i;
        f = network_to_host_float(i);
        return *this;
    }
    impl &operator >>(bool &b) {
        assert(can_read(1));
        b = chunk[pos++];
        return *this;
    }
    impl &operator >>(uint8_t &i) {
        assert(can_read(1));
        i = chunk[pos++];
        return *this;
    }
    impl &operator >>(int8_t &i) {
        *this >> reinterpret_cast<uint8_t &>(i);
        return *this;
    }
    impl &operator >>(uint16_t &i) {
        assert(can_read(2));
        uint16_t v{ 0 };
        v |= chunk[pos++] << 0;
        v |= chunk[pos++] << 8;
        i = ntohs(v);
        return *this;
    }
    impl &operator >>(int16_t &i) {
        *this >> reinterpret_cast<uint16_t &>(i);
        return *this;
    }
    impl &operator >>(uint32_t &i) {
        assert(can_read(4));
        uint32_t v{ 0 };
        v |= chunk[pos++] << 0;
        v |= chunk[pos++] << 8;
        v |= chunk[pos++] << 16;
        v |= chunk[pos++] << 24;
        i = ntohl(v);
        return *this;
    }
    impl &operator >>(int32_t &i) {
        *this >> reinterpret_cast<uint32_t &>(i);
        return *this;
    }

private:
    std::vector<uint8_t> chunk;
    size_t pos;
};

serializer::~serializer() {}

serializer::serializer() :
    writer_(true),
    needs_decompress_(false),
    impl_{std::make_unique<serializer::impl>()} {
}

serializer::serializer(const uint8_t *data, size_t len, bool is_compressed) :
    writer_(false),
    needs_decompress_(is_compressed),
    impl_{std::make_unique<impl>()} {
    impl_->load(data, len);
}

bool serializer::is_serializing() const {
    return writer_;
}

bool serializer::is_deserializing() const {
    return !writer_;
}

const uint8_t *serializer::data() const {
    return impl_->data();
}

size_t serializer::size() const {
    return impl_->size();
}

void serializer::compress() {
    if (!writer_) {
        return;
    }
    std::string dest;
    snappy::Compress(reinterpret_cast<const char *>(impl_->data()), impl_->size(), &dest);
    impl_->rewrite();
    impl_->load(reinterpret_cast<const uint8_t *>(dest.data()), dest.size());
    needs_decompress_ = true;
}

void serializer::decompress() {
    if (writer_) {
        return;
    }
    std::string dest;
    snappy::Uncompress(reinterpret_cast<const char *>(impl_->data()), impl_->size(), &dest);
    impl_->rewrite();
    impl_->reread();
    impl_->load(reinterpret_cast<const uint8_t *>(dest.data()), dest.size());
    needs_decompress_ = false;
}

bool serializer::is_compressed() const {
    return needs_decompress_;
}

template <typename T>
serializer &serializer::serialize(T &t) {
    assert(!needs_decompress_ && "can't write/read");
    if (writer_) {
        (*impl_) << t;
    } else {
        (*impl_) >> t;
    }
    return *this;
}

serializer &serializer::operator &(bool &v) {
    return serialize(v);
}

serializer &serializer::operator &(int8_t &v) {
    return serialize(v);
}

serializer &serializer::operator &(int16_t &v) {
    return serialize(v);
}

serializer &serializer::operator &(int32_t &v) {
    return serialize(v);
}

serializer &serializer::operator &(uint8_t &v) {
    return serialize(v);
}

serializer &serializer::operator &(uint16_t &v) {
    return serialize(v);
}

serializer &serializer::operator &(uint32_t &v) {
    return serialize(v);
}

serializer &serializer::operator &(float &v) {
    return serialize(v);
}

}
