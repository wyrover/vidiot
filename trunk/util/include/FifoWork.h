#ifndef FIFO_WORK_H
#define FIFO_WORK_H

#include "UtilFifo.h"

typedef boost::function< void() > Callable;

class Work : boost::noncopyable
{
public:
    Work(Callable work);
    virtual ~Work();

    void execute() const;

private:
    Callable mCallable;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Work& obj);

};

typedef Fifo<WorkPtr> FifoWork;
std::ostream& operator<< (std::ostream& os, const WorkPtr& obj);

#endif // FIFO_WORK_H