#include <vector>
#include "catch2/catch.hpp"
#include "ini.hpp"

TEST_CASE("MainSection", "[main_section]")
{
    ini::Data d = ini::parse("a=b");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection(""));
    REQUIRE(d[""].getSize() == 1);
    REQUIRE(d[""].hasValue("a"));
    REQUIRE(d[""]["a"] == "b");
}

TEST_CASE("Section", "[section]")
{
    ini::Data d = ini::parse("[s]\na=b");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection("s"));
    REQUIRE(d["s"].getSize() == 1);
    REQUIRE(d["s"].hasValue("a"));
    REQUIRE(d["s"]["a"] == "b");
}

TEST_CASE("Unicode", "[unicode]")
{
    ini::Data d = ini::parse("[š]\nā=ē");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection("š"));
    REQUIRE(d["š"].getSize() == 1);
    REQUIRE(d["š"].hasValue("ā"));
    REQUIRE(d["š"]["ā"] == "ē");
}

TEST_CASE("Encoding", "[encoding]")
{
    ini::Data d;
    d[""]["a"] = "a";
    d["foo"]["bar"] = "b";
    d["foo"]["baz"] = "ā";

    REQUIRE(ini::encode(d) == "a=a\n[foo]\nbar=b\nbaz=ā\n");
}

TEST_CASE("Byte", "[byte]")
{
    std::vector<std::byte> data = {
        static_cast<std::byte>('a'),
        static_cast<std::byte>('='),
        static_cast<std::byte>('b')
    };

    ini::Data d = ini::parse(data);
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection(""));
    REQUIRE(d[""].getSize() == 1);
    REQUIRE(d[""].hasValue("a"));
    REQUIRE(d[""]["a"] == "b");
}
