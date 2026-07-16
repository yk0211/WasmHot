#include "core/object.h"
#include "core/component_storage.h"

namespace wasmh {

ComponentData* GameObject::GetComponent(ComponentType type)
{
    return storage ? storage->Get(header.object_id, type) : nullptr;
}

const ComponentData* GameObject::GetComponent(ComponentType type) const
{
    return storage ? storage->Get(header.object_id, type) : nullptr;
}

void GameObject::SetComponent(ComponentType type, ComponentData data)
{
    if (storage)
    {
        storage->Set(header.object_id, type, std::move(data));
    }
}

bool GameObject::RemoveComponent(ComponentType type)
{
    return storage ? storage->Remove(header.object_id, type) : false;
}

} // namespace wasmh
