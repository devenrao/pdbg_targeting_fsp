#pragma once

#include <targeting/common/attributes.H>
#include <targeting/common/predicates/predicatebase.H>
#include <targeting/common/target.H>

#include <string_view>

namespace targeting
{

template <const ATTRIBUTE_ID A> class PredicateStrFind : public PredicateBase
{
  public:
    PredicateStrFind(std::string_view i_desiredStr,
                     size_t i_desiredPos = std::string::npos) :
        iv_desiredStr(i_desiredStr), iv_desiredPos(i_desiredPos)
    {}

    PredicateStrFind& setDesiredString(std::string_view i_desiredStr)
    {
        iv_desiredStr = i_desiredStr;
        return *this;
    }

    PredicateStrFind&
        setDesiredPosition(size_t i_desiredPos = std::string::npos)
    {
        iv_desiredPos = i_desiredPos;
        return *this;
    }

    bool operator()(const Target* i_pTarget) const override
    {
        typename AttributeTraits<A>::Type attr{};
        if (!i_pTarget->tryGetAttr<A>(attr))
        {
            return false;
        }

        // Handle both char arrays and std::string
        std::string_view actualStr(reinterpret_cast<const char*>(attr),
                                   sizeof(attr));

        // Trim at null terminator if present
        auto nullPos = actualStr.find('\0');
        if (nullPos != std::string_view::npos)
        {
            actualStr = actualStr.substr(0, nullPos);
        }

        auto foundPos = actualStr.find(iv_desiredStr);
        return foundPos != std::string::npos &&
               (iv_desiredPos == std::string::npos ||
                foundPos == iv_desiredPos);
    }

  private:
    std::string_view iv_desiredStr;
    size_t iv_desiredPos;
};
} // namespace targeting
