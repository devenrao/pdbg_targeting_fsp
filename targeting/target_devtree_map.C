#include <target_devtree_map.H>

#include <iostream>

namespace TARGETING::internal
{

TargetDevtreeMap::TargetDevtreeMap(const void* fdt) : _fdt(fdt)
{
    indexAllNodes();
}

std::generator<TargetPtr> TargetDevtreeMap::iterateTargetsByType(TYPE type) const
{
    auto it = _typeIndex.find(type);
    if (it == _typeIndex.end())
        co_return;

    for (const auto& info : it->second)
    {
        co_yield std::make_unique<Target>(_fdt, info.offset);
    }
}

std::generator<TargetPtr> TargetDevtreeMap::iterateDescendantsOfType(TYPE type) const
{
    auto it = _typeIndex.find(type);
    if (it == _typeIndex.end())
        co_return;

    for (const NodeInfo& parent : it->second)
    {
        for (const auto& [path, info] : _pathToNode)
        {
            if (path.getSize() > parent.path.getSize() &&
                path.equals(parent.path, parent.path.getSize()))
            {
                co_yield std::make_unique<Target>(_fdt, info.offset);
            }
        }
    }
}

std::generator<TargetPtr>
TargetDevtreeMap::iterateImmediateChildrenOfType(TYPE type) const
{
    auto it = _typeIndex.find(type);
    if (it == _typeIndex.end())
        co_return;

    for (const NodeInfo& parent : it->second)
    {
        for (const auto& [path, info] : _pathToNode)
        {
            if (path.getSize() == parent.path.getSize() + 1 &&
                path.equals(parent.path, parent.path.getSize()))
            {
                co_yield std::make_unique<Target>(_fdt, info.offset);
            }
        }
    }
}

void TargetDevtreeMap::indexAllNodes()
{
    int offset = -1;
    while ((offset = fdt_next_node(_fdt, offset, nullptr)) >= 0)
    {
        EntityPath path = parseEntityPath(offset);
        if (path.getSize() == 0 || path.type() != EntityPath::PathType::Physical)
        {
            continue;
        }
        NodeInfo info{offset, path};

        _pathToNode.emplace(path, info);
        const auto& lastElem = path[path.getSize() - 1];
        _typeIndex[lastElem.type()].push_back(info);
    }
}

EntityPath TargetDevtreeMap::parseEntityPath(int offset) const
{
    int len = 0;
    const uint8_t* prop = reinterpret_cast<const uint8_t*>(
        fdt_getprop(_fdt, offset, "ATTR_PHYS_BIN_PATH", &len));

    std::span<const uint8_t> data(
        prop, (prop && len > 0) ? static_cast<size_t>(len) : 0);

    return EntityPath::fromBinary(data);;
}

TargetPtr TargetDevtreeMap::getParentOf(const TargetPtr child) const
{
    auto it = std::find_if(_pathToNode.begin(), _pathToNode.end(),
                           [&child](const auto& pair) {
                               return pair.second.offset == child->_offset;
                           });

    if (it == _pathToNode.end())
    {
        return nullptr;
    }

    EntityPath parentPath = it->first.copyRemoveLast();

    auto parentIt = _pathToNode.find(parentPath);
    if (parentIt == _pathToNode.end())
    {
        return nullptr;
    }

    return std::make_unique<Target>(_fdt, parentIt->second.offset);
}

std::generator<TargetPtr>
    TargetDevtreeMap::preOrderTraversal(const EntityPath& root) const
{
    auto it = _pathToNode.find(root);
    if (it == _pathToNode.end())
    {
        co_return;
    }
    co_yield std::make_unique<Target>(_fdt, it->second.offset);

    for (const auto& [path, info] : _pathToNode)
    {
        if (path.getSize() == root.getSize() + 1 && path.equals(root, root.getSize()))
        {
            for (auto&& child : preOrderTraversal(path))
            {
                co_yield std::move(child);
            }
        }
    }
}

std::generator<TargetPtr> TargetDevtreeMap::iterateAllTargets() const
{
    for (const auto& [path, info] : _pathToNode)
    {
        if (path.getSize() == 1)
        {
            for (auto&& node : preOrderTraversal(path))
            {
                co_yield std::move(node);
            }
        }
    }
}

TargetPtr
    TargetDevtreeMap::getNextTarget(const TargetPtr& target) const noexcept
{
    EntityPath path;
    if (!target->tryGetAttr<ATTR_PHYS_BIN_PATH>(path))
    {
        return nullptr;
    }

    bool found = false;

    for (auto&& t : preOrderTraversal(path))
    {
        if (found)
        {
            return std::move(t);
        }

        EntityPath otherPath;
        if (t->tryGetAttr<ATTR_PHYS_PATH>(otherPath) && otherPath == path)
        {
            found = true;
        }
    }

    return nullptr;
}

TargetPtr TargetDevtreeMap::getTopLevelTarget() const noexcept
{
    for (const auto& [path, info] : _pathToNode)
    {
        if (path.getSize() == 1)
        {
            return std::make_unique<Target>(_fdt, info.offset);
        }
    }
    return nullptr;
}

TargetPtr TargetDevtreeMap::toTarget(const EntityPath& i_entityPath) const
{
    auto it = _pathToNode.find(i_entityPath);
    if (it == _pathToNode.end())
    {
        return nullptr;
    }

    return std::make_unique<Target>(_fdt, it->second.offset);
}

std::generator<TargetPtr>
TargetDevtreeMap::getImmediateChildren(const TargetPtr& parent) const
{
    // Step 1: Look up the path of the parent target
    auto it = std::find_if(_pathToNode.begin(), _pathToNode.end(),
                           [&parent](const auto& pair) {
                               return pair.second.offset == parent->_offset;
                           });

    if (it == _pathToNode.end())
    {
        co_return;
    }

    const EntityPath& parentPath = it->first;

    // Step 2: Look for paths that are exactly one level deeper and start with parentPath
    for (const auto& [path, info] : _pathToNode)
    {
        if (path.getSize() == parentPath.getSize() + 1 &&
            path.equals(parentPath, parentPath.getSize()))
        {
            co_yield std::make_unique<Target>(_fdt, info.offset);
        }
    }
}
} // namespace TARGETING::internal
