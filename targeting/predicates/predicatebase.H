#pragma once
#include <target.H>
namespace TARGETING
{

class Target;

// Interface class for predicates
class PredicateBase
{
  public:
    virtual ~PredicateBase() = default;

    virtual bool operator()(const TargetPtr& i_pTarget) const = 0;

  protected:
    PredicateBase() = default;
    PredicateBase(const PredicateBase&) = default;
    PredicateBase& operator=(const PredicateBase&) = default;
};

} // namespace TARGETING
