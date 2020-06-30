#include <iostream>
#include <vector>
#include "ini.hpp"

namespace
{
    class TestError final: public std::logic_error
    {
    public:
        explicit TestError(const std::string& str): std::logic_error(str) {}
        explicit TestError(const char* str): std::logic_error(str) {}
    };

    class TestRunner final
    {
    public:
        template <class T, class ...Args>
        void run(const std::string& name, T test, Args ...args) noexcept
        {
            try
            {
                test(args...);
                std::cerr << name << " succeeded\n";
            }
            catch (const TestError& e)
            {
                std::cerr << name << " failed: " << e.what() << '\n';
                result = false;
            }
        }

        bool getResult() const noexcept { return result; }

    private:
        bool result = true;
    };

    void testMainSection()
    {
        ini::Data d = ini::parse("a=b");
        if (d.getSize() != 1)
            throw TestError("Expected one section");

        if (!d.hasSection(""))
            throw TestError("Expected the main section");

        if (d[""].getSize() != 1)
            throw TestError("Expected one value");

        if (!d[""].hasValue("a"))
            throw TestError("Wrong key");

        if (d[""]["a"] != "b")
            throw TestError("Wrong value");
    }

    void testSection()
    {
        ini::Data d = ini::parse("[s]\na=b");
        if (d.getSize() != 1)
            throw TestError("Expected one section");

        if (!d.hasSection("s"))
            throw TestError("Wrong section");

        if (d["s"].getSize() != 1)
            throw TestError("Expected one value");

        if (!d["s"].hasValue("a"))
            throw TestError("Wrong key");

        if (d["s"]["a"] != "b")
            throw TestError("Wrong value");
    }

    void testUnicode()
    {
        ini::Data d = ini::parse("[š]\nā=ē");
        if (d.getSize() != 1)
            throw TestError("Expected one section");

        if (!d.hasSection("š"))
            throw TestError("Wrong section");

        if (d["š"].getSize() != 1)
            throw TestError("Expected one value");

        if (!d["š"].hasValue("ā"))
            throw TestError("Wrong key");

        if (d["š"]["ā"] != "ē")
            throw TestError("Wrong value");
    }

    void testEncoding()
    {
        ini::Data d;
        d[""]["a"] = "a";
        d["foo"]["bar"] = "b";
        d["foo"]["baz"] = "ā";

        if (ini::encode(d) != "a=a\n[foo]\nbar=b\nbaz=ā\n")
            throw TestError("Wrong encoded result");
    }

    enum class byte: unsigned char {};

    void testByte()
    {
        std::vector<byte> data = {
            static_cast<byte>('a'),
            static_cast<byte>('='),
            static_cast<byte>('b')
        };

        ini::Data d = ini::parse(data);
        if (d.getSize() != 1)
            throw TestError("Expected one section");

        if (!d.hasSection(""))
            throw TestError("Expected the main section");

        if (d[""].getSize() != 1)
            throw TestError("Expected one value");

        if (!d[""].hasValue("a"))
            throw TestError("Wrong key");

        if (d[""]["a"] != "b")
            throw TestError("Wrong value");
    }
}

int main()
{
    TestRunner testRunner;
    testRunner.run("testMainSection", testMainSection);
    testRunner.run("testSection", testSection);
    testRunner.run("testUnicode", testUnicode);
    testRunner.run("testEncoding", testEncoding);
    testRunner.run("testByte", testByte);

    if (testRunner.getResult())
        std::cout << "Success\n";

    return testRunner.getResult() ? EXIT_SUCCESS : EXIT_FAILURE;
}
