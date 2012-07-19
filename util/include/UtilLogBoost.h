#ifndef UTIL_LOG_BOOST_H
#define UTIL_LOG_BOOST_H

#include <boost/filesystem/path.hpp> // todo remove

std::ostream& operator<< (std::ostream& os, const boost::filesystem::path& obj);

#endif //UTIL_LOG_BOOST_H