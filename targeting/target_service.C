#include <target_service.H>
extern "C"
{
#include <libfdt.h>
}

#include <fstream>
#include <vector>

namespace TARGETING
{
// Recursive generator for pre-order traversal
TargetService& TargetService::instance()
{
    static TargetService service;
    return service;
}

void TargetService::init(const std::string& dtbPath)
{
    if (_initialized)
    {
        return;
    }
    _loader = std::make_unique<DeviceTreeLoader>(dtbPath);

    _rootOffset = fdt_path_offset(_loader->fdt(), "/");
    if (_rootOffset < 0)
    {
        throw std::runtime_error("Failed to find root node");
    }

    _targetMap =
        std::unique_ptr<TargetDevtreeMap>(new TargetDevtreeMap(_loader->fdt()));
    _initialized = true;
}

std::generator<TargetPtr> TargetService::getAssociated(
    const TargetPtr& source, AssociationType type,
    RecursionLevel recursionLevel, const PredicateBase* predicate) const
{
    if (!_targetMap)
        co_return;

    for (auto&& target :
         _targetMap->getAssociated(source, type, recursionLevel, predicate))
    {
        co_yield std::move(target);
    }
}

TargetPtr TargetService::getParentOf(const TargetPtr& child,
                                     AssociationType type) const
{
    return _targetMap->getParentOf(child, type);
}

TargetPtr TargetService::toTarget(const EntityPath& entityPath) const
{
    if (!_targetMap)
    {
        return nullptr;
    }
    return _targetMap->toTarget(entityPath);
}

TargetPtr TargetService::getTopLevelTarget() const noexcept
{
    if (_rootOffset < 0)
    {
        return nullptr;
    }
    return std::make_unique<Target>(_loader->fdt(), _rootOffset);
}
} // namespace TARGETING
