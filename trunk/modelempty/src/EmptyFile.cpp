#include "EmptyFile.h"

#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyFile::EmptyFile()
{
}

EmptyFile::EmptyFile(const EmptyFile& other)
{
}

EmptyFile* EmptyFile::clone() const
{
    return new EmptyFile(static_cast<const EmptyFile&>(*this));
}

EmptyFile::~EmptyFile()
{
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts EmptyFile::getLength() const
{
    // todo remove class altogether
    return 0;
}

void EmptyFile::moveTo(pts position)
{
    FATAL;
}

wxString EmptyFile::getDescription() const
{
    return wxString();
}

void EmptyFile::clean()
{
}

} //namespace