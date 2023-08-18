#include <array>
#include <chrono>

template<size_t _size>
class fps_counter
{
public:
    using clock = std::chrono::high_resolution_clock;

    void operator ++ (int)
    {
        clock::time_point tp = clock::now();

        if (_full)
        {
            ++_left %= _size;
        }

        while (_left != _right && _values[_left] < tp - period)
        {
            ++_left %= _size;
            _full = false;
        }

        _values[_right] = tp;

        ++_right %= _size;

        _full = _right == _left;
    }

    size_t count()
    {
        std::make_signed_t<size_t> fps = _right - _left + _full * _size;
        return (fps >= 0) ? fps : _size + fps;
    }

    void clear()
    {
        _left = _right = 0;
    }

private:
    static constexpr clock::duration period = std::chrono::seconds(1);

    std::array<clock::time_point, _size> _values;
    size_t _left{0};
    size_t _right{0};
    bool _full{false};
};