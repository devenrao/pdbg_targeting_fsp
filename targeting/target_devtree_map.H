#pragma once

#include "attributeenums.H"
#include "entitypath.H"
#include "target.H"
extern "C"
{
#include <libfdt.h>
}
#include <algorithm>
#include <generator>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace TARGETING::internal
{
class TargetDevtreeMap
{
  public:
    explicit TargetDevtreeMap(const void* fdt);
    ~TargetDevtreeMap() = default;

    TargetPtr getParentOf(const TargetPtr child) const;

    std::generator<TargetPtr> iterateAllTargets() const;

    TargetPtr getNextTarget(const TargetPtr& target) const noexcept;

    TargetPtr getTopLevelTarget() const noexcept;

    TargetPtr toTarget(const EntityPath& i_entityPath) const;

    std::generator<TargetPtr>
        getImmediateChildren(const TargetPtr& parent) const;

  private:
    void indexAllNodes();

    EntityPath parseEntityPath(int offset) const;

    std::generator<TargetPtr> preOrderTraversal(const EntityPath& root) const;

    std::generator<TargetPtr> iterateTargetsByType(TYPE type) const;

    std::generator<TargetPtr> iterateDescendantsOfType(TYPE type) const;

    std::generator<TargetPtr> iterateImmediateChildrenOfType(TYPE type) const;

  private:
    const void* _fdt;

    struct NodeInfo
    {
        int offset;
        EntityPath path;
    };

    std::map<EntityPath, NodeInfo> _pathToNode;
    std::unordered_map<TYPE, std::vector<NodeInfo>> _typeIndex;
};
} // namespace TARGETING
