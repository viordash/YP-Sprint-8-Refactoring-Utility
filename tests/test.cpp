#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

class RefactorToolTest : public ::testing::Test {
protected:
    const std::string temp_file = "build/refactor_test_XXXXXX.cpp";
    const std::string refactor_tool = "build/refactor_tool";

    void TearDown() override { std::remove(temp_file.c_str()); }

    std::string readTempFile() {
        std::ifstream file(temp_file);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    void writeTempFile(const std::string &content) {
        std::ofstream input_file(temp_file);
        input_file << content;
        input_file.close();
    }

    void runRefactorTool() {
        std::string command = refactor_tool + " " + temp_file + " --";
        int result = std::system(command.c_str());
        ASSERT_EQ(result, 0);
    }

    std::string remove_whitespaces(const std::string &str) {
        std::string result = str;
        result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(std::remove(result.begin(), result.end(), '\t'), result.end());
        return result;
    }
};

// ============================================================================
// Tests for 'virtual dtor'
// ============================================================================

TEST_F(RefactorToolTest, AddsVirtualToNonVirtualDestructor) {
    const std::string original = R"(
class Base {
public:
    ~Base() {}
};

class Derived : public Base {};
)";

    const std::string expected = R"(
class Base {
public:
    virtual ~Base() {}
};

class Derived : public Base {};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, SkipVirtualDestructorForHasNoDescedants) {
    const std::string original = R"(
class Base {
public:
    ~Base() {}
};
)";

    const std::string expected = R"(
class Base {
public:
    ~Base() {}
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, SkipAlreadyVirtualDestructor) {
    const std::string original = R"(
class Base {
public:
   virtual ~Base() {}
};

class Derived : public Base {};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, MultipleInheritance) {
    const std::string original = R"(
class Base1 {
public:
    ~Base1() {}
};

class Base2 {
public:
    ~Base2() {}
};

class Derived : public Base1, public Base2 {};
)";

    const std::string expected = R"(
class Base1 {
public:
    virtual ~Base1() {}
};

class Base2 {
public:
    virtual ~Base2() {}
};

class Derived : public Base1, public Base2 {};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, InheritanceChain) {
    const std::string original = R"(
class GrandParent {
public:
    ~GrandParent() {}
};

class Parent : public GrandParent {
public:
    ~Parent() {}
};

class Child : public Parent {};
)";

    const std::string expected = R"(
class GrandParent {
public:
    virtual ~GrandParent() {}
};

class Parent : public GrandParent {
public:
    virtual ~Parent() {}
};

class Child : public Parent {};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

// ============================================================================
// Tests for 'override'
// ============================================================================

TEST_F(RefactorToolTest, AddsOverrideToSimpleMethod) {
    const std::string original = R"(
class Base {
public:
    virtual void func() {}
};

class Derived : public Base {
public:
    void func() {}
};
)";

    const std::string expected = R"(
class Base {
public:
    virtual void func() {}
};

class Derived : public Base {
public:
    void func() override {}
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, AddsOverrideToPureVirtualRedeclaration) {
    const std::string original = R"(
class Base {
public:
    virtual void func() = 0;
};

class Derived : public Base {
public:
    void func() = 0;
};
)";

    const std::string expected = R"(
class Base {
public:
    virtual void func() = 0;
};

class Derived : public Base {
public:
    void func() override = 0;
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, SkipMethodWithExistingOverride) {
    const std::string original = R"(
class Base {
public:
    virtual void func() {}
};

class Derived : public Base {
public:
    void func() override {}
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, IgnoresNonVirtualMethod) {
    const std::string original = R"(
class Base {
public:
    void func() {}
};

class Derived : public Base {
public:
    void func() {}
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, IgnoresVirtualDestructors) {
    const std::string original = R"(
class Base {
public:
    virtual ~Base() {}
};

class Derived : public Base {
public:
    ~Derived() {}
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, HandlesInheritanceChainForOverride) {
    const std::string original = R"(
class GrandParent {
public:
    virtual void func() {}
};

class Parent : public GrandParent {
public:
    void func() {}
};

class Child : public Parent {
public:
    void func() {}
};
)";

    const std::string expected = R"(
class GrandParent {
public:
    virtual void func() {}
};

class Parent : public GrandParent {
public:
    void func() override {}
};

class Child : public Parent {
public:
    void func() override {}
};
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}