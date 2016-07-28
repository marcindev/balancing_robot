#ifndef BASE_MOCK_H
#define BASE_MOCK_H

class BaseMock
{
public:
    BaseMock() : _enabled(true)
    {
    }

    virtual ~BaseMock() { }

    bool isEnabled() const { return _enabled; }
    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
private:
    bool _enabled;
};

#endif // BASE_MOCK_H

