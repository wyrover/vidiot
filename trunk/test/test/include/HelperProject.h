#ifndef HELPER_PROJECT_H
#define HELPER_PROJECT_H

#include "HelperFileSystem.h"

namespace test {

std::pair<RandomTempDirPtr, wxFileName> SaveProjectAndClose();

} // namespace

#endif // HELPER_PROJECT_H