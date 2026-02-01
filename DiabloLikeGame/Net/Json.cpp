#include "Json.h"
#include <cctype>
#include <cstdlib>

namespace Json {

// ============== Stringify Implementation ==============

std::string Value::Stringify(bool pretty, int indent) const {
    std::ostringstream ss;
    const std::string indentStr = pretty ? std::string(indent * 2, ' ') : "";
    const std::string nextIndent = pretty ? std::string((indent + 1) * 2, ' ') : "";
    const std::string newline = pretty ? "\n" : "";
    const std::string space = pretty ? " " : "";
    
    if (IsNull()) {
        ss << "null";
    } else if (IsBool()) {
        ss << (AsBool() ? "true" : "false");
    } else if (IsInt()) {
        ss << AsInt();
    } else if (IsFloat()) {
        ss << AsFloat();
    } else if (IsString()) {
        ss << '"';
        for (char c : AsString()) {
            switch (c) {
                case '"': ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default: ss << c; break;
            }
        }
        ss << '"';
    } else if (IsArray()) {
        const auto& arr = AsArray();
        if (arr.empty()) {
            ss << "[]";
        } else {
            ss << "[" << newline;
            for (size_t i = 0; i < arr.size(); ++i) {
                ss << nextIndent << arr[i].Stringify(pretty, indent + 1);
                if (i + 1 < arr.size()) ss << ",";
                ss << newline;
            }
            ss << indentStr << "]";
        }
    } else if (IsObject()) {
        const auto& obj = AsObject();
        if (obj.empty()) {
            ss << "{}";
        } else {
            ss << "{" << newline;
            size_t i = 0;
            for (const auto& [key, val] : obj) {
                ss << nextIndent << '"' << key << '"' << ":" << space;
                ss << val.Stringify(pretty, indent + 1);
                if (++i < obj.size()) ss << ",";
                ss << newline;
            }
            ss << indentStr << "}";
        }
    }
    
    return ss.str();
}

// ============== Parser Implementation ==============

class Parser {
public:
    explicit Parser(const std::string& json) : m_json(json), m_pos(0) {}
    
    Value Parse() {
        SkipWhitespace();
        auto result = ParseValue();
        SkipWhitespace();
        if (m_pos != m_json.size()) {
            throw std::runtime_error("Unexpected characters after JSON");
        }
        return result;
    }

private:
    const std::string& m_json;
    size_t m_pos;
    
    char Peek() const {
        return m_pos < m_json.size() ? m_json[m_pos] : '\0';
    }
    
    char Get() {
        return m_pos < m_json.size() ? m_json[m_pos++] : '\0';
    }
    
    void SkipWhitespace() {
        while (m_pos < m_json.size() && std::isspace(static_cast<unsigned char>(m_json[m_pos]))) {
            ++m_pos;
        }
    }
    
    bool Match(const char* str) {
        size_t len = std::strlen(str);
        if (m_pos + len > m_json.size()) return false;
        if (m_json.compare(m_pos, len, str) == 0) {
            m_pos += len;
            return true;
        }
        return false;
    }
    
    Value ParseValue() {
        SkipWhitespace();
        char c = Peek();
        
        if (c == 'n') return ParseNull();
        if (c == 't' || c == 'f') return ParseBool();
        if (c == '"') return ParseString();
        if (c == '[') return ParseArray();
        if (c == '{') return ParseObject();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return ParseNumber();
        
        throw std::runtime_error("Unexpected character in JSON");
    }
    
    Value ParseNull() {
        if (Match("null")) return Value(nullptr);
        throw std::runtime_error("Expected 'null'");
    }
    
    Value ParseBool() {
        if (Match("true")) return Value(true);
        if (Match("false")) return Value(false);
        throw std::runtime_error("Expected 'true' or 'false'");
    }
    
    Value ParseString() {
        if (Get() != '"') throw std::runtime_error("Expected '\"'");
        
        std::string result;
        while (true) {
            char c = Get();
            if (c == '\0') throw std::runtime_error("Unterminated string");
            if (c == '"') break;
            if (c == '\\') {
                c = Get();
                switch (c) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += c; break;
                }
            } else {
                result += c;
            }
        }
        return Value(std::move(result));
    }
    
    Value ParseNumber() {
        size_t start = m_pos;
        bool isFloat = false;
        
        if (Peek() == '-') ++m_pos;
        while (std::isdigit(static_cast<unsigned char>(Peek()))) ++m_pos;
        
        if (Peek() == '.') {
            isFloat = true;
            ++m_pos;
            while (std::isdigit(static_cast<unsigned char>(Peek()))) ++m_pos;
        }
        
        if (Peek() == 'e' || Peek() == 'E') {
            isFloat = true;
            ++m_pos;
            if (Peek() == '+' || Peek() == '-') ++m_pos;
            while (std::isdigit(static_cast<unsigned char>(Peek()))) ++m_pos;
        }
        
        std::string numStr = m_json.substr(start, m_pos - start);
        if (isFloat) {
            return Value(std::stod(numStr));
        } else {
            return Value(static_cast<Int>(std::stoll(numStr)));
        }
    }
    
    Value ParseArray() {
        if (Get() != '[') throw std::runtime_error("Expected '['");
        
        Array arr;
        SkipWhitespace();
        
        if (Peek() == ']') {
            ++m_pos;
            return Value(std::move(arr));
        }
        
        while (true) {
            arr.push_back(ParseValue());
            SkipWhitespace();
            
            if (Peek() == ']') {
                ++m_pos;
                break;
            }
            if (Get() != ',') throw std::runtime_error("Expected ',' or ']'");
        }
        
        return Value(std::move(arr));
    }
    
    Value ParseObject() {
        if (Get() != '{') throw std::runtime_error("Expected '{'");
        
        Object obj;
        SkipWhitespace();
        
        if (Peek() == '}') {
            ++m_pos;
            return Value(std::move(obj));
        }
        
        while (true) {
            SkipWhitespace();
            auto keyVal = ParseString();
            std::string key = keyVal.AsString();
            
            SkipWhitespace();
            if (Get() != ':') throw std::runtime_error("Expected ':'");
            
            obj[key] = ParseValue();
            SkipWhitespace();
            
            if (Peek() == '}') {
                ++m_pos;
                break;
            }
            if (Get() != ',') throw std::runtime_error("Expected ',' or '}'");
        }
        
        return Value(std::move(obj));
    }
};

Value Value::Parse(const std::string& json) {
    Parser parser(json);
    return parser.Parse();
}

} // namespace Json
