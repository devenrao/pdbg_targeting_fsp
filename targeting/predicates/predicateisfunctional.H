#pragma once

#include <predicatebase.H>
#include <target.H>
#include <attributeenums.H>
#include <attributetraits.H>

namespace targeting
{

class PredicateIsFunctional : public PredicateBase
{
  public:
    PredicateIsFunctional() = default;
    ~PredicateIsFunctional() override = default;

    bool operator()(const TargetPtr& target) const override
    {
        return target && target->getAttr<ATTR_HWAS_STATE>().functional;
    }
};

} // namespace targeting
