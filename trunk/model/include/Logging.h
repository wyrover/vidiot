#ifndef MODEL_DEBUG_H
#define MODEL_DEBUG_H

namespace model {

template <class OBJ>
std::ostream& dump(std::ostream& os, std::list< boost::shared_ptr< OBJ > > list, int depth = 0 )
{
    typedef boost::shared_ptr< OBJ > OBJPtr;
    BOOST_FOREACH( OBJPtr obj, list )
    {
        dump(os,obj, depth);
    }
    return os;
}

std::ostream& dump(std::ostream& os, SequencePtr sequence, int depth = 0);
std::ostream& dump(std::ostream& os, TrackPtr track, int depth = 0);
std::ostream& dump(std::ostream& os, IClipPtr clip, int depth = 0);

template <class OBJ>
std::string dump(OBJ o, int depth = 0)
{
    std::ostringstream os;
    dump(os,o,depth);
    return os.str();
}

#define DUMP(o) std::endl << #o << model::dump(o,1)

} // namespace

#endif // MODEL_DEBUG_H