#pragma once

#include <dtree_loader.H>
#include <target.H>
#include <target_devtree_map.H>

#include <cstdint>
#include <generator>
#include <vector>
#include <predicatebase.H>
namespace TARGETING
{
using internal::TargetDevtreeMap;
class TargetService
{
  public:
    // Singleton accessor
    static TargetService& instance();

    // Public API
    void init(const std::string& dtbPath);

    void* getFDT() const noexcept
    {
        return _loader ? _loader->fdt() : nullptr;
    }

    std::generator<TargetPtr> iterateAllTargets() const;

    std::generator<TargetPtr> getAssociated(const PredicateBase& predicate) const;

    TargetPtr getParentOf(const TargetPtr child) const;
    
    std::generator<TargetPtr> getImmediateChildren(const TargetPtr& parent) const;
  private:
    TargetService() = default;
    ~TargetService() = default;

    TargetService(const TargetService&) = delete;
    TargetService& operator=(const TargetService&) = delete;

    [[nodiscard]] bool isInitialized() const noexcept
    {
        return _initialized;
    }
    std::unique_ptr<DeviceTreeLoader> _loader;
    std::unique_ptr<TargetDevtreeMap> _targetMap;

    bool _initialized{false};
};
} // namespace TARGETING
