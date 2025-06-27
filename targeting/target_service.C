#include <target_service.H>
extern "C"
{
#include <libfdt.h>
}
#include <log.hpp>

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

    int rootOffset = fdt_path_offset(_loader->fdt(), "/");
    if (rootOffset < 0)
    {
        throw std::runtime_error("Failed to find root node");
    }

    _targetMap = std::unique_ptr<TargetDevtreeMap>(new TargetDevtreeMap(_loader->fdt()));
    _initialized = true;
}

std::generator<TargetPtr> TargetService::iterateAllTargets() const
{
    if (!_targetMap)
    {
        co_return;
    }

    for (auto&& target : _targetMap->iterateAllTargets())
    {
        co_yield std::move(target);
    }
}

std::generator<TargetPtr> TargetService::getAssociated(const PredicateBase& predicate) const
{
    for (auto&& target : iterateAllTargets())  // target is TargetPtr
    {
        if (predicate(target))
        {
            co_yield std::move(target); // return ownership
        }
    }
}

std::generator<TargetPtr>
TargetService::getImmediateChildren(const TargetPtr& parent) const
{
    if (!_targetMap)
    {
        co_return;
    }

    for (auto&& child : _targetMap->getImmediateChildren(parent))
    {
        co_yield std::move(child);
    }
}
} // namespace TARGETING
