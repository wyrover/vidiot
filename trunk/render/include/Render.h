#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "File.h" // TODO: Why does this prevent the UINT16_C macro error from occurring? Does this have to do with UtilFifo including #define BOOST_HAS_STDINT_H???

namespace model {

class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;

class Render
    : public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Render(SequencePtr sequence);

	virtual ~Render();

    void generate();

private:

    SequencePtr mSequence;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Render& obj );
};

} // namespace

#endif // MODEL_RENDER_H