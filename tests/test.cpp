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

TEST_F(RefactorToolTest, Dtor_AddsVirtualToNonVirtual) {
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

TEST_F(RefactorToolTest, Dtor_SkipForHasNoDescedants) {
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

TEST_F(RefactorToolTest, Dtor_SkipAlreadyVirtual) {
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

TEST_F(RefactorToolTest, Dtor_MultipleInheritance) {
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

TEST_F(RefactorToolTest, Dtor_InheritanceChain) {
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

TEST_F(RefactorToolTest, Override_AddsToSimpleMethod) {
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

TEST_F(RefactorToolTest, Override_AddsToPureVirtualRedeclaration) {
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

TEST_F(RefactorToolTest, Override_SkipMethodWithExistingOverride) {
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

TEST_F(RefactorToolTest, Override_IgnoresNonVirtualMethod) {
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

TEST_F(RefactorToolTest, Override_IgnoresVirtualDestructors) {
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

TEST_F(RefactorToolTest, Override_HandlesInheritanceChain) {
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

// ============================================================================
// Tests for 'const T&' in range-for
// ============================================================================

TEST_F(RefactorToolTest, InRangeLoop_AddsReferenceToConstAutoLoopVar) {
    const std::string original = R"(
struct MyType { char ch; };
void func() {
    MyType arr[100];
    for (const auto x : arr) {}
}
)";

    const std::string expected = R"(
struct MyType { char ch; };
void func() {
    MyType arr[100];
    for (const auto& x : arr) {}
}
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, InRangeLoop_AddsReferenceToConstExplicitTypeLoopVar) {
    const std::string original = R"(
struct MyType { char ch; };
void func() {
    MyType arr[100];
    for (const MyType x : arr) {}
}
)";

    const std::string expected = R"(
struct MyType { char ch; };
void func() {
    MyType arr[100];
    for (const MyType& x : arr) {}
}
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(expected));
}

TEST_F(RefactorToolTest, InRangeLoop_SkipsAlreadyExistingReference) {
    const std::string original = R"(
struct MyType { char ch; };
void func() {
    MyType arr[100];
    for (const auto& x : arr) {}
}
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, InRangeLoop_SkipsFundamentalTypes) {
    const std::string original = R"(
void func() {
    int arr[100];
    for (const int x : arr) {}
    double arr2[100];
    for (const double d : arr2) {}
    char arr3[100];
    for (const char ch : arr3) {}
}
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, InRangeLoop_SkipsPointerTypes) {
    const std::string original = R"(
void func() {
    const char* arr[100];
    for (const char* s : arr) {}    
    int* arr2[100];
    for (const int* p : arr2) {}
}
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}

TEST_F(RefactorToolTest, InRangeLoop_SkipsNonConstVariables) {
    const std::string original = R"(
struct MyType { char ch; };
void func() {
    MyType arr[100];
    for (auto x : arr) {}
    for (MyType x : arr) {}
}
)";

    writeTempFile(original);
    runRefactorTool();
    auto refactored = readTempFile();

    ASSERT_EQ(remove_whitespaces(refactored), remove_whitespaces(original));
}