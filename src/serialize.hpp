
#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <memory>

namespace m {

class serializer final {
    class impl;
public:
    serializer();
    serializer(const uint8_t *data, size_t len);

    bool is_serializing() const;
    bool is_deserializing() const;

    const uint8_t *data() const;
    size_t length() const;
    
    template <typename T>
    serializer &operator &(T &t);
private:
    bool writer_;
    std::unique_ptr<impl> impl_;
};

}

#endif

