#include <attributeenums.H>
#include <attributestructs.H>
#include <attributetraits.H>
#include <target.H>
#include <target_service.H>
#include <predicateattr.H>
#include <predicateattrval.H>
#include <iomanip>
#include <iostream>

extern "C"
{
#include <libfdt.h>
}

int main()
{
    try
    {
        auto& ts = TARGETING::TargetService::instance();
        ts.init("test/targeting_test.dtb");

        std::cout << "****Listing all targets ********" << std::endl;
        for (auto&& target : ts.iterateAllTargets())  // generator-based iteration
        {
            std::string physDevPath;
            if (target->tryGetAttr<TARGETING::ATTR_PHYS_DEV_PATH>(physDevPath))
            {
                std::cout << "ATTR: " << physDevPath << std::endl;
            }
            else
            {
                std::cout << "ATTR_PHYS_DEV_PATH not found for target " << target->getName() << std::endl;
            }
        }

        std::cout << "****Listing all targets which have ATTR_TYPE attribute ********" << std::endl;
        {
            TARGETING::PredicateAttr<TARGETING::ATTR_TYPE> pred;
            for (TARGETING::TargetPtr target : ts.getAssociated(pred))
            {
                std::string physDevPath;
                if (target->tryGetAttr<TARGETING::ATTR_PHYS_DEV_PATH>(physDevPath))
                {
                    std::cout << "ATTR: " << physDevPath << std::endl;
                }
                else
                {
                    std::cout << "ATTR: not found for" << target->getName() << std::endl;
                }
            }
        }

        std::cout << "****Listing all targets which have ATTR_TYPE=PROC ********" << std::endl;
        {
            TARGETING::PredicateAttrVal<TARGETING::ATTR_TYPE> pred(TARGETING::TYPE_PROC);
            for (TARGETING::TargetPtr target : ts.getAssociated(pred))
            {
                std::string physDevPath;
                if (target->tryGetAttr<TARGETING::ATTR_PHYS_DEV_PATH>(physDevPath))
                {
                    std::cout << "ATTR: " << physDevPath << std::endl;
                }
                else
                {
                    std::cout << "ATTR: not found for" << target->getName() << std::endl;
                }
            }
        }

        std::cout << "****Listing immediate children of a proc target ********" << std::endl;
        {
            TARGETING::PredicateAttrVal<TARGETING::ATTR_TYPE> pred(TARGETING::TYPE_PROC);
            for (TARGETING::TargetPtr target : ts.getAssociated(pred))
            {
                std::string physDevPath;
                if (target->tryGetAttr<TARGETING::ATTR_PHYS_DEV_PATH>(physDevPath))
                {
                    std::cout << "##Parent ATTR: " << physDevPath << std::endl;
                }
                for (auto&& child : ts.getImmediateChildren(target))
                {
                    std::string physDevPath;
                    if (child->tryGetAttr<TARGETING::ATTR_PHYS_DEV_PATH>(physDevPath))
                    {
                        std::cout << "CHILD ATTR: " << physDevPath << std::endl;
                    }
                    else
                    {
                        std::cout << "ATTR: not found for" << child->getName() << std::endl;
                    }
                }
                break;
            }
        }
    }
    catch (std::exception& ex)
    {
        std::cout << "exception raised " << ex.what() << std::endl;
    }
}

