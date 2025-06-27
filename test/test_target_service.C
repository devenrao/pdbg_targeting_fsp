#include <attributeenums.H>
#include <entitypath.H>
#include <target.H>
#include <target_devtree_map.H>
#include <target_service.H>

#include <fstream>
#include <vector>

#include <gtest/gtest.h>
#include <array>

using namespace targeting;

TEST(TargetDevtreeMapTest, LoadTopLevelTargetAndCheckAttributes)
{
    // Load DTB into memory
    std::ifstream file("test/targeting_test.dtb", std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "Failed to open targeting_test.dtb";

    std::vector<char> dtbData((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();

    ASSERT_GT(dtbData.size(), 0) << "DTB is empty";

    const void* fdt = dtbData.data();
    ASSERT_EQ(fdt_check_header(fdt), 0) << "Invalid FDT header";

    // Construct devtree map
    TargetDevtreeMap map(fdt);

    // Build expected top-level EntityPath
    EntityPath path;
    path.setType(EntityPath::PathType::Physical);
    path.addLast(TYPE_SYS, 0);

    auto target = map.toTarget(path);
    ASSERT_NE(target, nullptr) << "Top-level target not found";

    // Check ATTR_TYPE
    uint8_t typeVal = 0;
    bool foundType = target->tryGetAttr<ATTR_TYPE>(typeVal);
    ASSERT_TRUE(foundType) << "ATTR_TYPE not found";
    EXPECT_EQ(typeVal, TYPE_SYS) << "ATTR_TYPE value mismatch";

    // Check ATTR_CLASS
    uint8_t classVal = 0;
    bool foundClass = target->tryGetAttr<ATTR_CLASS>(classVal);
    ASSERT_TRUE(foundClass) << "ATTR_CLASS not found";
    EXPECT_EQ(classVal, 0x01) << "ATTR_CLASS value mismatch";
}

using namespace TARGETING;

TEST(EntityPathTest, FromBinary_SingleElement)
{
    std::array<uint8_t, 3> data = { 0x21, 0x01, 0x00 }; // Physical, 1 element: SYS0

    EntityPath path = EntityPath::fromBinary(data);
    EXPECT_EQ(path.type(), EntityPath::PathType::Physical);
    EXPECT_EQ(path.getSize(), 1);
    EXPECT_EQ(path[0].type(), TYPE_SYS);
    EXPECT_EQ(path[0].instance, 0);
    EXPECT_EQ(path.toString(), "Physical/Sys0");
}

TEST(EntityPathTest, FromBinary_TwoElements)
{
    std::array<uint8_t, 5> data = { 0x22, 0x01, 0x00, 0x02, 0x00 }; // SYS0, NODE0

    EntityPath path = EntityPath::fromBinary(data);
    EXPECT_EQ(path.getSize(), 2);
    EXPECT_EQ(path[0].type(), TYPE_SYS);
    EXPECT_EQ(path[1].type(), TYPE_NODE);
    EXPECT_EQ(path.toString(), "Physical/Sys0/Node0");
}

TEST(EntityPathTest, FromBinary_ThreeElements)
{
    std::array<uint8_t, 7> data = { 0x23, 0x01, 0x00, 0x02, 0x00, 0x05, 0x00 }; // SYS0, NODE0, PROC0

    EntityPath path = EntityPath::fromBinary(data);
    EXPECT_EQ(path.getSize(), 3);
    EXPECT_EQ(path[2].type(), TYPE_PROC);
    EXPECT_EQ(path.toString(), "Physical/Sys0/Node0/Proc0");
}

TEST(EntityPathTest, FromBinary_TooShortData)
{
    std::array<uint8_t, 1> data = { 0x21 }; // Missing two bytes for one element

    EntityPath path = EntityPath::fromBinary(data);
    EXPECT_EQ(path.getSize(), 0);
}

