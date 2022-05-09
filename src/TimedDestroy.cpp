#include "TimedDestroy.h"

void TimedDestroy::update(Seconds delta)
{
    _timer -= delta;
    if (_timer.count() <= 0)
    {
        owner().destroy();
    }
}

Seconds TimedDestroy::timer() const
{
    return _timer;
}

void TimedDestroy::timer(Seconds time)
{
    _timer = time;
}
