#ifndef FIFO_WORK_H
#define FIFO_WORK_H

#include <wx/string.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
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

typedef boost::shared_ptr<Work> WorkPtr;
typedef Fifo<WorkPtr> FifoWork;
std::ostream& operator<< (std::ostream& os, const WorkPtr& obj);

#endif // FIFO_WORK_H