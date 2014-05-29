// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef PRECOMPILED_TEST_H
#define PRECOMPILED_TEST_H

#include "Precompiled.h"

#include <cxxtest/TestSuite.h>
#include <cxxtest/TestTracker.h>
//#include <list> // todo why not in precompiled?
#include <typeinfo>
#include <wx/uiaction.h>

#include "FixtureProject.h" // todo these in precompiled testauto only?
#include "HelperTestSuite.h" // todo cleanup includes in cpp files in testaudo and test as a result of including Precompiled.h
#include "SuiteCreator.h"
#include "UtilLog.h"

#endif
