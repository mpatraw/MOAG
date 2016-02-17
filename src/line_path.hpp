
#ifndef LINE_PATH_HPP
#define LINE_PATH_HPP

#include <vector>
#include <tuple>

template <typename I=int>
class line_path final {
public:
    typedef std::tuple<I, I> point;

public:
    line_path(I x0, I y0, I x1, I y1) {
        reset(x0, y0, x1, y1);
    }

    void reset(I x0, I y0, I x1, I y1) {
        points.clear();
        I dx = std::abs(x1 - x0);
        int sx = x0 < x1 ? 1 : -1;
        I dy = std::abs(y1 - y0);
        int sy = y0 < y1 ? 1 : -1;
        I err = (dx > dy ? dx : -dy) / 2;
        I e2;

        while (true) {
            points.push_back(point{x0, y0});
            if (x0 == x1 && y0 == y1) {
                break;
            }
            e2 = err;
            if (e2 > -dx) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dy) {
                err += dx;
                y0 += sy;
            }
        }
    }

    const point &first() const { return points[0]; }
    const point &last() const { return points[points.size() - 1]; }

    size_t length() const { return points.size(); }
    typename std::vector<point>::iterator begin() { return points.begin(); }
    typename std::vector<point>::iterator end() { return points.end(); }
    typename std::vector<point>::const_iterator begin() const { return points.begin(); }
    typename std::vector<point>::const_iterator end() const { return points.end(); }
    typename std::vector<point>::const_iterator cbegin() const { return points.begin(); }
    typename std::vector<point>::const_iterator cend() const { return points.end(); }
private:
    std::vector<point> points;
};

#endif
