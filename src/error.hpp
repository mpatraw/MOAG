
#ifndef ERROR_HPP
#define ERROR_HPP

namespace m {

class initialization_error {
public:
    initialization_error(const char *msg) : whatstr{msg} { }

    virtual const char *what() const { return whatstr.c_str(); }
protected:
    std::string whatstr;
};

}

#endif
