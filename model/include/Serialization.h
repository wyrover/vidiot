// Copyright 2013 Eric Raijmakers.
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

#ifndef MODEL_SERIALIZATION_H
#define MODEL_SERIALIZATION_H

namespace model {

/// Register all objects that need to be serialized. To avoid difficult to resolve
/// 'unregistered_class' exceptions. Typically, when serializing a - ex. File -
/// class through a parent - Node - pointer, it may occur that the 
/// derived class is not automatically registered in the archive. Since that occurs
/// rather unexpected (when making changes unrelated to serialization), all types
/// persisted are registered via this method, both for loading and saving.
/// 
/// All concrete objects that are serialized should be included here.
/// Interfaces/Abstract do not have to be included (won't compile...)
template < class Archive >
void registerClasses(Archive& ar);

} // namespace

#endif // MODEL_SERIALIZATION_H
