#ifndef FIFO_WORK_H
#define FIFO_WORK_H

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <wx/string.h>
#include "UtilFifo.h"

typedef boost::function< void() > Callable;

class Work : boost::noncopyable
{
public:
    Work(Callable work, wxString description);
    virtual ~Work();

    void execute() const;
    wxString getDescription() const;

private:
    Callable mCallable;
    wxString mDescription;
};

typedef boost::shared_ptr<Work> WorkPtr;
typedef Fifo<WorkPtr> FifoWork;

std::ostream& operator<< (std::ostream& os, const WorkPtr& obj);
std::ostream& operator<< (std::ostream& os, const Work& obj);

#endif // FIFO_WORK_H