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
