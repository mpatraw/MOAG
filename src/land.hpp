
#ifndef LAND_HPP
#define LAND_HPP

#include <array>

namespace m {

const int land_width = 600;
const int land_height = 800;

// This is used so another class can be informed when dirt is set or unset,
// this is primarily usefull for a sprite class or something.
//
// `land` takes ownership of the delegate.
class land_delegate {
public:
    virtual ~land_delegate() {}
    virtual void set_dirt(int x, int y) = 0;
    virtual void set_air(int x, int y) = 0;
};

class land {
public:
    land() {
        dirt.fill(0);
    }

    // Takes ownership.
    void set_delegate(land_delegate *d) {
        delegate.reset(d);
    }

    void set_dirt(int x, int y) {
        if (x < 0 || y < 0 || x >= land_width || y >= land_height) {
            return;
        }
        dirt[y * land_width + x] = 1;
        if (delegate) {
            delegate->set_dirt(x, y);
        }
    }

    void set_air(int x, int y) {
        if (x < 0 || y < 0 || x >= land_width || y >= land_height) {
            return;
        }
        dirt[y * land_width + x] = 0;
        if (delegate) {
            delegate->set_air(x, y);
        }
    }

    land_delegate &get_delegate() { return *delegate.get(); }

    bool is_dirt(int x, int y) const {
        if (x < 0 || y < 0 || x >= land_width || y >= land_height) {
            return true;
        }
        return dirt[y * land_width + x] != 0;
    }

    bool is_air(int x, int y) const {
        return !is_dirt(x, y);
    }

    const uint8_t *data() const { return dirt.data(); }

private:
    std::unique_ptr<land_delegate> delegate;
    std::array<uint8_t, land_width * land_height> dirt;
};

}

#endif
