
#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <memory>

namespace m {

class serializer final {
    class impl;
public:
    serializer();
    serializer(const uint8_t *data, size_t len, bool is_compressed);
    ~serializer();

    bool is_serializing() const;
    bool is_deserializing() const;

    const uint8_t *data() const;
    size_t size() const;

    void compress();
    void decompress();
    bool is_compressed() const;
    
    serializer &operator &(bool &b);
    serializer &operator &(int8_t &i);
    serializer &operator &(int16_t &i);
    serializer &operator &(int32_t &i);
    serializer &operator &(uint8_t &i);
    serializer &operator &(uint16_t &i);
    serializer &operator &(uint32_t &i);
    serializer &operator &(float &f);
private:
    bool writer_;
    bool needs_decompress_;
    std::unique_ptr<impl> impl_;
private:
    template <typename T> serializer &serialize(T &t);
};

class serializable {
public:
    virtual ~serializable() {}
    virtual void serialize(serializer &s) = 0;
};

}

#endif

