#pragma once

#include "attributeenums.H"
#include "entitypath.H"
#include "target.H"
extern "C"
{
#include <libfdt.h>
}
#include <association.H>
#include <predicatebase.H>
#include <generator>
#include <map>
namespace TARGETING::internal
{
class TargetDevtreeMap
{
  public:
    explicit TargetDevtreeMap(const void* fdt);
    ~TargetDevtreeMap() = default;

    TargetPtr getParentOf(const TargetPtr& child,
                          const AssociationType type) const;

    TargetPtr toTarget(const EntityPath& i_entityPath) const;

    std::generator<TargetPtr> getAssociated(
        const TargetPtr& source, const AssociationType type = childByAffinity,
        const RecursionLevel recursionLevel = immediate,
        const PredicateBase* predicate = nullptr) const;

  private:
    void indexAllNodes();

    EntityPath parseEntityPathProperty(int offset, const char* propName) const;

  private:
    const void* _fdt;
    std::map<EntityPath, int> _phyPathToNode;
    std::map<EntityPath, int> _affinityPathToNode;
};
} // namespace TARGETING::internal
