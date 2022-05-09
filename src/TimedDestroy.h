#ifndef CGT_TIMEDDESTROY_H
#define CGT_TIMEDDESTROY_H

#include "Component.h"


class TimedDestroy : public Component
{
public:
    using Component::Component;

    void update(Seconds delta);

    Seconds timer() const;
    void timer(Seconds time);
private:
    Seconds _timer;
};


#endif
