#include <target_devtree_map.H>

namespace TARGETING::internal
{

constexpr const char* ATTR_PHYS_PATH_PROP = "ATTR_PHYS_PATH";
constexpr const char* ATTR_AFFINITY_PATH_PROP = "ATTR_AFFINITY_PATH";

TargetDevtreeMap::TargetDevtreeMap(const void* fdt) : _fdt(fdt)
{
    indexAllNodes();
}

TargetPtr TargetDevtreeMap::getParentOf(const TargetPtr& child,
                                        const AssociationType type) const
{
    if (!child)
    {
        return nullptr;
    }

    // Validate association type
    if (type != parentByPhysical && type != parentByAffinity)
    {
        std::cerr << "Invalid association type for getParentOf(): " << type
                  << std::endl;
        return nullptr;
    }

    // Select appropriate path map
    const auto& pathMap =
        (type == parentByPhysical) ? _phyPathToNode : _affinityPathToNode;

    EntityPath childPath;
    bool gotPath = false;

    // Dispatch based on association type
    if (type == parentByPhysical)
    {
        gotPath = child->tryGetAttr<ATTR_PHYS_PATH>(childPath);
    }
    else if (type == parentByAffinity)
    {
        gotPath = child->tryGetAttr<ATTR_AFFINITY_PATH>(childPath);
    }

    if (!gotPath)
    {
        return nullptr;
    }

    EntityPath parentPath = childPath.copyRemoveLast();

    auto parentIt = pathMap.find(parentPath);
    if (parentIt == pathMap.end())
    {
        return nullptr;
    }

    return std::make_unique<Target>(_fdt, parentIt->second);
}

TargetPtr TargetDevtreeMap::toTarget(const EntityPath& i_entityPath) const
{
    auto it = _phyPathToNode.find(i_entityPath);
    if (it == _phyPathToNode.end())
    {
        return nullptr;
    }

    return std::make_unique<Target>(_fdt, it->second);
}

std::generator<TargetPtr> TargetDevtreeMap::getAssociated(
    const TargetPtr& source, AssociationType type,
    RecursionLevel recursionLevel, const PredicateBase* predicate) const
{
    using enum AssociationType;
    using enum RecursionLevel;

    if (!source)
    {
        co_return;
    }
    const std::map<EntityPath, int>* pathMap = nullptr;
    EntityPath sourcePath;
    // Select the right path and map
    switch (type)
    {
        case childByPhysical:
        case parentByPhysical:
            if (!source->tryGetAttr<ATTR_PHYS_PATH>(sourcePath))
                co_return;
            pathMap = &_phyPathToNode;
            break;

        case childByAffinity:
        case parentByAffinity:
            if (!source->tryGetAttr<ATTR_AFFINITY_PATH>(sourcePath))
                co_return;
            pathMap = &_affinityPathToNode;
            break;
    }

    if (sourcePath.getSize() == 0 || !pathMap)
        co_return;

    if (type == childByPhysical || type == childByAffinity)
    {
        for (const auto& [path, offset] : *pathMap)
        {
            bool match = false;
            if (recursionLevel == immediate)
            {
                match = path.getSize() == sourcePath.getSize() + 1 &&
                        path.equals(sourcePath, sourcePath.getSize());
            }
            else if (recursionLevel == all)
            {
                match = path.getSize() > sourcePath.getSize() &&
                        path.equals(sourcePath, sourcePath.getSize());
            }

            if (match)
            {
                auto tgt = std::make_unique<Target>(_fdt, offset);
                if (!predicate || (*predicate)(tgt))
                    co_yield std::move(tgt);
            }
        }
    }
    else // parentBy*
    {
        EntityPath current = sourcePath.copyRemoveLast();
        while (current.getSize() > 0)
        {
            if (const auto it = pathMap->find(current); it != pathMap->end())
            {
                auto tgt = std::make_unique<Target>(_fdt, it->second);
                if (!predicate || (*predicate)(tgt))
                    co_yield std::move(tgt);

                if (recursionLevel == immediate)
                    break;
            }
            current = current.copyRemoveLast();
        }
    }
}

void TargetDevtreeMap::indexAllNodes()
{
    int offset = -1;
    while ((offset = fdt_next_node(_fdt, offset, nullptr)) >= 0)
    {
        // Parse physical path
        EntityPath physPath =
            parseEntityPathProperty(offset, ATTR_PHYS_PATH_PROP);
        if (physPath.getSize() > 0 &&
            physPath.type() == EntityPath::PathType::Physical)
        {
            _phyPathToNode.emplace(physPath, offset);
        }

        // Parse affinity path
        EntityPath affinityPath =
            parseEntityPathProperty(offset, ATTR_AFFINITY_PATH_PROP);
        if (affinityPath.getSize() > 0 &&
            affinityPath.type() == EntityPath::PathType::Affinity)
        {
            _affinityPathToNode.emplace(affinityPath, offset);
        }
    }
}

EntityPath TargetDevtreeMap::parseEntityPathProperty(int offset,
                                                     const char* propName) const
{
    int len = 0;
    const uint8_t* prop = reinterpret_cast<const uint8_t*>(
        fdt_getprop(_fdt, offset, propName, &len));
    if (!prop || len <= 0)
    {
        throw std::runtime_error(
            std::string("Failed to find ") + propName + " for the target");
    }

    std::span<const uint8_t> data(prop, static_cast<size_t>(len));
    return EntityPath::fromBinary(data);
}
} // namespace TARGETING::internal
