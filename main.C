#include <association.H>
#include <attributeenums.H>
#include <attributestructs.H>
#include <attributetraits.H>
#include <entitypath.H>
#include <predicateattr.H>
#include <predicateattrval.H>
#include <predicatepostfixexpr.H>
#include <target.H>
#include <target_service.H>

#include <iomanip>
#include <iostream>
extern "C"
{
#include <libfdt.h>
}

using namespace TARGETING;

int get_full_path(const void* fdt, int nodeoffset, char* out_path,
                  size_t out_size);
void printPropertiesOfNode(const void* fdt, const char* nodePath);

void print_all_node_paths(const void* fdt);

int main()
{
    try
    {
        auto& ts = TargetService::instance();
        ts.init("target/test/targeting_test.dtb");
        std::cout
            << "Test1: All targets with ATTR_PHYS_DEV_PATH childByPhysical all\n";
        {
            PredicateAttr<ATTR_PHYS_DEV_PATH> pred;
            for (auto&& tgt : ts.getAssociated(ts.getTopLevelTarget(),
                                               AssociationType::childByPhysical,
                                               RecursionLevel::all, &pred))
            {
                std::string path;
                if (tgt->tryGetAttr<ATTR_PHYS_DEV_PATH>(path))
                    std::cout << "  " << path << "\n";
                else
                    std::cout << "ATTR_PHYS_DEV_PATH not found " << std::endl;
            }
        }

        std::cout
            << "Test2: All targets with ATTR_PHYS_DEV_PATH childByAffinity all \n";
        {
            PredicateAttr<ATTR_PHYS_DEV_PATH> pred;
            for (auto&& tgt : ts.getAssociated(ts.getTopLevelTarget(),
                                               AssociationType::childByAffinity,
                                               RecursionLevel::all, &pred))
            {
                std::string path;
                if (tgt->tryGetAttr<ATTR_PHYS_DEV_PATH>(path))
                    std::cout << "  " << path << "\n";
                else
                    std::cout << "ATTR_PHYS_DEV_PATH not found " << std::endl;
            }
        }
        std::cout << "Test3: PredicatePostFoxExpr AttrVal and Attr \n";
        {
            PredicatePostfixExpr procAndDevPath;
            procAndDevPath
                .push(std::make_shared<PredicateAttrVal<ATTR_TYPE>>(TYPE_PROC))
                .push(std::make_shared<PredicateAttr<ATTR_PHYS_DEV_PATH>>())
                .And();
            for (auto&& parent : ts.getAssociated(
                     ts.getTopLevelTarget(), AssociationType::childByPhysical,
                     RecursionLevel::all, &procAndDevPath))
            {
                std::string parentPath;
                parent->tryGetAttr<ATTR_PHYS_DEV_PATH>(parentPath);
                std::cout << "PROC: " << parentPath << "\n";

                for (auto&& child :
                     ts.getAssociated(parent, AssociationType::childByPhysical,
                                      RecursionLevel::immediate))
                {
                    std::string childPath;
                    child->tryGetAttr<ATTR_PHYS_DEV_PATH>(childPath);
                    std::cout << "  └── " << childPath << "\n";
                }
            }
        }
        std::cout << "Convert ocmb1 binary data to EntityPath and to Target\n";
        {
            std::array<uint8_t, 21> bin = {
                0x23, 0x01, 0x00, 0x02, 0x00, 0x4B, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            EntityPath path =
                EntityPath::fromBinary(std::span<const uint8_t>{bin});
            auto ocmbTarget = ts.toTarget(path);
            std::string ocmb;
            ocmbTarget->tryGetAttr<ATTR_PHYS_DEV_PATH>(ocmb);
            std::cout << " entitypath to ocmb physical path " << ocmb << "\n";
            auto parentp =
                ts.getParentOf(ocmbTarget, AssociationType::parentByPhysical);
            parentp->tryGetAttr<ATTR_PHYS_DEV_PATH>(ocmb);
            std::cout << " parent of ocmb parent affinity path " << ocmb
                      << "\n";
            auto parenta =
                ts.getParentOf(ocmbTarget, AssociationType::parentByAffinity);
            parenta->tryGetAttr<ATTR_PHYS_DEV_PATH>(ocmb);
            std::cout << " parent of ocmb parent physical path " << ocmb
                      << "\n";
        }
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}

int get_full_path(const void* fdt, int nodeoffset, char* out_path,
                  size_t out_size)
{
    if (nodeoffset < 0 || !out_path || out_size == 0)
        return -1;

    char temp[1024] = {0};

    while (nodeoffset >= 0)
    {
        int len;
        const char* name = fdt_get_name(fdt, nodeoffset, &len);
        if (!name)
            return -1;

        // Prepend the current node name
        char new_path[1024];
        if (strcmp(temp, "") == 0)
            snprintf(new_path, sizeof(new_path), "/%s", name);
        else
            snprintf(new_path, sizeof(new_path), "/%s%s", name, temp);

        strncpy(temp, new_path, sizeof(temp) - 1);
        nodeoffset = fdt_parent_offset(fdt, nodeoffset);
    }

    strncpy(out_path, temp, out_size - 1);
    return 0;
}

void printPropertiesOfNode(const void* fdt, const char* nodePath)
{
    int offset = fdt_path_offset(fdt, nodePath);
    if (offset < 0)
    {
        std::cerr << "Node not found: " << nodePath << "\n";
        return;
    }

    std::cout << "Properties for node: " << nodePath << "\n";

    int prop_offset;
    int len = 0;
    const char* name = nullptr;

    fdt_for_each_property_offset(prop_offset, fdt, offset)
    {
        const void* data = fdt_getprop_by_offset(fdt, prop_offset, &name, &len);
        if (!data || !name || len <= 0)
            continue;

        std::cout << "  " << name << " = ";
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        for (int i = 0; i < len; ++i)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << (int)bytes[i] << " ";
        }
        std::cout << std::dec << "\n";
    }
}

void print_all_node_paths(const void* fdt)
{
    int offset = -1;
    char path[1024];

    while ((offset = fdt_next_node(fdt, offset, NULL)) >= 0)
    {
        if (get_full_path(fdt, offset, path, sizeof(path)) == 0)
        {
            printf("Node offset %d: %s\n", offset, path);
        }
        else
        {
            printf("Failed to get path for node %d\n", offset);
        }
    }
}
