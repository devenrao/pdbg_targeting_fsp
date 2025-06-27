#pragma once

#include <predicatebase.H>
#include <target.H>
#include <attributeenums.H>
#include <attributetraits.H>

namespace TARGETING
{

class Target;

template <const ATTRIBUTE_ID A> class PredicateAttr : public PredicateBase
{
  public:
    explicit PredicateAttr()
    {
    }

    ~PredicateAttr() override = default;

    bool operator()(const TargetPtr& target) const override
    {
        typename AttributeTraits<A>::Type tmp; // intentionally uninitialized
        bool match = target->tryGetAttr<A>(tmp);
        return match;
    }

    // Optional: prevent copy/move if not needed
    PredicateAttr(const PredicateAttr&) = delete;
    PredicateAttr& operator=(const PredicateAttr&) = delete;
};
} // namespace TARGETING
