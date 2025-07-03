#pragma once

#include <attributeenums.H>
#include <attributetraits.H>

#include <cstring>
#include <iostream>
#include <memory>
extern "C"
{
#include <libfdt.h>
}

namespace TARGETING
{
class Target;
using TargetPtr = std::unique_ptr<Target>;
class Target
{
  public:
    Target(const void* fdt, int offset) : _fdt(fdt), _offset(offset) {}
    ~Target() = default;
    template <const TARGETING::ATTRIBUTE_ID A>
    bool tryGetAttr(
        typename TARGETING::AttributeTraits<A>::Type& o_attrValue) const;

    std::string getName() const
    {
        const char* name = fdt_get_name(_fdt, _offset, nullptr);
        return name ? std::string(name) : "";
    }
    const void* _fdt = nullptr;
    int _offset = -1;
};

namespace // local use only
{
template <typename T>
bool tryGetAttrHelper(const void* fdt, int offset, std::string propName,
                      T& outVal)
{
    int len = 0;

    const void* prop = fdt_getprop(fdt, offset, propName.c_str(), &len);
    if (!prop || len < static_cast<int>(sizeof(T)))
    {
        return false;
    }

    if (static_cast<size_t>(len) < sizeof(T))
    {
        std::cout << "  Property found but too small: " << propName
                  << ", len=" << len << " expected=" << sizeof(T) << std::endl;
        return false;
    }

    std::memcpy(&outVal, prop, sizeof(T));
    return true;
}

template <>
inline bool tryGetAttrHelper<EntityPath>(
    const void* fdt, int offset, std::string propName, EntityPath& outVal)
{
    int len = 0;
    const uint8_t* prop = reinterpret_cast<const uint8_t*>(
        fdt_getprop(fdt, offset, propName.c_str(), &len));

    if (!prop || len < static_cast<int>(sizeof(EntityPath)))
        return false;

    std::span<const uint8_t> data(prop, static_cast<size_t>(len));
    outVal = EntityPath::fromBinary(data);

    // If it has zero size after decoding, consider it a failure
    return outVal.getSize() > 0;
}

template <>
inline bool tryGetAttrHelper<std::string>(
    const void* fdt, int offset, std::string propName, std::string& outVal)
{
    int len = 0;
    const char* prop = static_cast<const char*>(
        fdt_getprop(fdt, offset, propName.c_str(), &len));

    if (!prop || len <= 0)
        return false;

    outVal.assign(prop, len);
    return true;
}
} // namespace

template <const ATTRIBUTE_ID A>
bool Target::tryGetAttr(typename AttributeTraits<A>::Type& o_attrValue) const
{
    auto nameOpt = tryGetAttrName<A>();
    if (!nameOpt)
    {
        std::cout << "Attribute name not found, missing in attributenums.H"
                  << std::endl;
        return false;
    }
    return tryGetAttrHelper(_fdt, _offset, *nameOpt, o_attrValue);
}
} // namespace TARGETING
