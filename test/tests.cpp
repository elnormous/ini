#include <cstddef>
#include <vector>
#include "catch2/catch.hpp"
#include "ini.hpp"

TEST_CASE("Empty", "[decoding]")
{
    const ini::Data d = ini::parse("");
    REQUIRE(d.getSize() == 0);
}

TEST_CASE("MainSection", "[decoding]")
{
    const ini::Data d = ini::parse("a=b");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection(""));
    REQUIRE(d[""].getSize() == 1);
    REQUIRE(d[""].hasValue("a"));
    REQUIRE(d[""]["a"] == "b");
}

TEST_CASE("Section", "[decoding]")
{
    const ini::Data d = ini::parse("[s]\na=b");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection("s"));
    REQUIRE(d["s"].getSize() == 1);
    REQUIRE(d["s"].hasValue("a"));
    REQUIRE(d["s"]["a"] == "b");
}

TEST_CASE("Unicode", "[decoding]")
{
    const ini::Data d = ini::parse("[š]\nā=ē");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection("š"));
    REQUIRE(d["š"].getSize() == 1);
    REQUIRE(d["š"].hasValue("ā"));
    REQUIRE(d["š"]["ā"] == "ē");
}

TEST_CASE("Comments", "[decoding]")
{
    const ini::Data d = ini::parse("[s];aa\na=b; bb");
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection("s"));
    REQUIRE(d["s"].getSize() == 1);
    REQUIRE(d["s"].hasValue("a"));
    REQUIRE(d["s"]["a"] == "b");
}

TEST_CASE("Value encoding", "[encoding]")
{
    ini::Data d;
    d[""]["a"] = "a";

    REQUIRE(ini::encode(d) == "a=a\n");
}

TEST_CASE("UTF-8 encoding", "[encoding]")
{
    ini::Data d;
    d[""]["a"] = "ā";

    REQUIRE(ini::encode(d) == "a=ā\n");
}

TEST_CASE("Section encoding", "[encoding]")
{
    ini::Data d;
    d["foo"]["a"] = "a";

    REQUIRE(ini::encode(d) == "[foo]\na=a\n");
}

TEST_CASE("Byte", "[byte]")
{
    const std::vector<std::byte> data = {
        static_cast<std::byte>('a'),
        static_cast<std::byte>('='),
        static_cast<std::byte>('b')
    };

    const ini::Data d = ini::parse(data);
    REQUIRE(d.getSize() == 1);
    REQUIRE(d.hasSection(""));
    REQUIRE(d[""].getSize() == 1);
    REQUIRE(d[""].hasValue("a"));
    REQUIRE(d[""]["a"] == "b");
}

TEST_CASE("Range-based for loop of sections")
{
    SECTION("Mutable")
    {
        ini::Data data;
        data["0"] = ini::Section{};
        data["1"] = ini::Section{};

        int counter = 0;

        for (auto& i : data)
            REQUIRE(i.first == std::to_string(counter++));

        REQUIRE(counter == 2);
    }

    SECTION("Const")
    {
        ini::Data data;
        data["0"] = ini::Section{};
        data["1"] = ini::Section{};

        const auto& constData = data;

        int counter = 0;

        for (auto& i : constData)
            REQUIRE(i.first == std::to_string(counter++));

        REQUIRE(counter == 2);
    }
}

TEST_CASE("Range-based for loop of values")
{
    SECTION("Mutable")
    {
        ini::Section section;
        section["0"] = std::string{};
        section["1"] = std::string{};

        int counter = 0;

        for (auto& i : section)
            REQUIRE(i.first == std::to_string(counter++));

        REQUIRE(counter == 2);
    }

    SECTION("Const")
    {
        ini::Section section;
        section["0"] = std::string{};
        section["1"] = std::string{};

        const auto& constSection = section;

        int counter = 0;

        for (auto& i : constSection)
            REQUIRE(i.first == std::to_string(counter++));

        REQUIRE(counter == 2);
    }
}

TEST_CASE("Invalid section")
{
    ini::Data data;
    const auto& constData = data;

    REQUIRE_THROWS_AS(constData["a"], ini::RangeError);
}

TEST_CASE("Invalid value")
{
    ini::Section section;
    const auto& constSection = section;

    REQUIRE_THROWS_AS(constSection["a"], ini::RangeError);
}
