#include "CppUnitTest.h"
#include "../Core/IniParser.h"
#include <fstream>
#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IniParserTests
{
    // Helper to create temporary INI files for testing
    class TempIniFile {
    public:
        TempIniFile(const std::string& content) 
            : m_path("test_temp.ini") 
        {
            std::ofstream file(m_path);
            file << content;
        }
        ~TempIniFile() {
            std::filesystem::remove(m_path);
        }
        const std::string& GetPath() const { return m_path; }
    private:
        std::string m_path;
    };

    TEST_CLASS(IniParserLoadTests)
    {
    public:
        TEST_METHOD(LoadNonexistentFileReturnsFalse)
        {
            IniParser parser;
            Assert::IsFalse(parser.Load("nonexistent_file_12345.ini"));
        }

        TEST_METHOD(LoadEmptyFileReturnsTrue)
        {
            TempIniFile file("");
            IniParser parser;
            Assert::IsTrue(parser.Load(file.GetPath()));
        }

        TEST_METHOD(LoadValidFileReturnsTrue)
        {
            TempIniFile file("[Section]\nKey=Value");
            IniParser parser;
            Assert::IsTrue(parser.Load(file.GetPath()));
        }
    };

    TEST_CLASS(IniParserStringTests)
    {
    public:
        TEST_METHOD(GetStringExistingKey)
        {
            TempIniFile file("[Section]\nName=TestValue");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Section", "Name");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(std::string("TestValue"), result.value());
        }

        TEST_METHOD(GetStringNonexistentKey)
        {
            TempIniFile file("[Section]\nName=TestValue");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Section", "Missing");
            Assert::IsFalse(result.has_value());
        }

        TEST_METHOD(GetStringNonexistentSection)
        {
            TempIniFile file("[Section]\nName=TestValue");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Missing", "Name");
            Assert::IsFalse(result.has_value());
        }

        TEST_METHOD(GetStringWithDefault)
        {
            TempIniFile file("[Section]\nName=TestValue");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::AreEqual(std::string("TestValue"), 
                parser.GetString("Section", "Name", "Default"));
            Assert::AreEqual(std::string("Default"), 
                parser.GetString("Section", "Missing", "Default"));
        }

        TEST_METHOD(GetStringWithSpaces)
        {
            TempIniFile file("[Section]\n  Key  =  Value With Spaces  ");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Section", "Key");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(std::string("Value With Spaces"), result.value());
        }
    };

    TEST_CLASS(IniParserIntTests)
    {
    public:
        TEST_METHOD(GetIntValidValue)
        {
            TempIniFile file("[Section]\nNumber=42");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetInt("Section", "Number");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(42, result.value());
        }

        TEST_METHOD(GetIntNegativeValue)
        {
            TempIniFile file("[Section]\nNumber=-100");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetInt("Section", "Number");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(-100, result.value());
        }

        TEST_METHOD(GetIntInvalidValue)
        {
            TempIniFile file("[Section]\nNumber=NotANumber");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetInt("Section", "Number");
            Assert::IsFalse(result.has_value());
        }

        TEST_METHOD(GetIntWithDefault)
        {
            TempIniFile file("[Section]\nNumber=42");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::AreEqual(42, parser.GetInt("Section", "Number", 0));
            Assert::AreEqual(99, parser.GetInt("Section", "Missing", 99));
        }
    };

    TEST_CLASS(IniParserFloatTests)
    {
    public:
        TEST_METHOD(GetFloatValidValue)
        {
            TempIniFile file("[Section]\nValue=3.14");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetFloat("Section", "Value");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(3.14f, result.value(), 0.001f);
        }

        TEST_METHOD(GetFloatNegativeValue)
        {
            TempIniFile file("[Section]\nValue=-2.5");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetFloat("Section", "Value");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(-2.5f, result.value(), 0.001f);
        }

        TEST_METHOD(GetFloatIntegerValue)
        {
            TempIniFile file("[Section]\nValue=42");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetFloat("Section", "Value");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(42.0f, result.value(), 0.001f);
        }

        TEST_METHOD(GetFloatWithDefault)
        {
            TempIniFile file("[Section]\nValue=1.5");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::AreEqual(1.5f, parser.GetFloat("Section", "Value", 0.0f), 0.001f);
            Assert::AreEqual(9.9f, parser.GetFloat("Section", "Missing", 9.9f), 0.001f);
        }
    };

    TEST_CLASS(IniParserBoolTests)
    {
    public:
        TEST_METHOD(GetBoolTrueValues)
        {
            TempIniFile file("[Section]\nA=true\nB=True\nC=TRUE\nD=1\nE=yes\nF=Yes");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::IsTrue(parser.GetBool("Section", "A").value());
            Assert::IsTrue(parser.GetBool("Section", "B").value());
            Assert::IsTrue(parser.GetBool("Section", "C").value());
            Assert::IsTrue(parser.GetBool("Section", "D").value());
            Assert::IsTrue(parser.GetBool("Section", "E").value());
            Assert::IsTrue(parser.GetBool("Section", "F").value());
        }

        TEST_METHOD(GetBoolFalseValues)
        {
            TempIniFile file("[Section]\nA=false\nB=False\nC=FALSE\nD=0\nE=no\nF=No");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::IsFalse(parser.GetBool("Section", "A").value());
            Assert::IsFalse(parser.GetBool("Section", "B").value());
            Assert::IsFalse(parser.GetBool("Section", "C").value());
            Assert::IsFalse(parser.GetBool("Section", "D").value());
            Assert::IsFalse(parser.GetBool("Section", "E").value());
            Assert::IsFalse(parser.GetBool("Section", "F").value());
        }

        TEST_METHOD(GetBoolInvalidValue)
        {
            TempIniFile file("[Section]\nValue=maybe");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetBool("Section", "Value");
            Assert::IsFalse(result.has_value());
        }

        TEST_METHOD(GetBoolWithDefault)
        {
            TempIniFile file("[Section]\nEnabled=true");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::IsTrue(parser.GetBool("Section", "Enabled", false));
            Assert::IsTrue(parser.GetBool("Section", "Missing", true));
        }
    };

    TEST_CLASS(IniParserColorTests)
    {
    public:
        TEST_METHOD(GetColorRGB)
        {
            TempIniFile file("[Section]\nColor=255,128,64");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetColor("Section", "Color");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(static_cast<unsigned char>(255), result.value().r);
            Assert::AreEqual(static_cast<unsigned char>(128), result.value().g);
            Assert::AreEqual(static_cast<unsigned char>(64), result.value().b);
            Assert::AreEqual(static_cast<unsigned char>(255), result.value().a);
        }

        TEST_METHOD(GetColorRGBA)
        {
            TempIniFile file("[Section]\nColor=100,150,200,128");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetColor("Section", "Color");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(static_cast<unsigned char>(100), result.value().r);
            Assert::AreEqual(static_cast<unsigned char>(150), result.value().g);
            Assert::AreEqual(static_cast<unsigned char>(200), result.value().b);
            Assert::AreEqual(static_cast<unsigned char>(128), result.value().a);
        }

        TEST_METHOD(GetColorWithDefault)
        {
            TempIniFile file("[Section]\nColor=10,20,30");
            IniParser parser;
            parser.Load(file.GetPath());
            Color defaultColor = {0, 0, 0, 255};
            Color result = parser.GetColor("Section", "Missing", defaultColor);
            Assert::AreEqual(static_cast<unsigned char>(0), result.r);
        }
    };

    TEST_CLASS(IniParserCommentTests)
    {
    public:
        TEST_METHOD(IgnoresSemicolonComments)
        {
            TempIniFile file("; This is a comment\n[Section]\nKey=Value");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Section", "Key");
            Assert::IsTrue(result.has_value());
            Assert::AreEqual(std::string("Value"), result.value());
        }

        TEST_METHOD(IgnoresHashComments)
        {
            TempIniFile file("# This is a comment\n[Section]\nKey=Value");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Section", "Key");
            Assert::IsTrue(result.has_value());
        }

        TEST_METHOD(IgnoresEmptyLines)
        {
            TempIniFile file("\n\n[Section]\n\nKey=Value\n\n");
            IniParser parser;
            parser.Load(file.GetPath());
            auto result = parser.GetString("Section", "Key");
            Assert::IsTrue(result.has_value());
        }
    };

    TEST_CLASS(IniParserMultipleSectionsTests)
    {
    public:
        TEST_METHOD(MultipleSections)
        {
            TempIniFile file("[Section1]\nA=1\n[Section2]\nB=2\n[Section3]\nC=3");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::AreEqual(1, parser.GetInt("Section1", "A", 0));
            Assert::AreEqual(2, parser.GetInt("Section2", "B", 0));
            Assert::AreEqual(3, parser.GetInt("Section3", "C", 0));
        }

        TEST_METHOD(SameKeyDifferentSections)
        {
            TempIniFile file("[Section1]\nKey=Value1\n[Section2]\nKey=Value2");
            IniParser parser;
            parser.Load(file.GetPath());
            Assert::AreEqual(std::string("Value1"), 
                parser.GetString("Section1", "Key", ""));
            Assert::AreEqual(std::string("Value2"), 
                parser.GetString("Section2", "Key", ""));
        }
    };
}
