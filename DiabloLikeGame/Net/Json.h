#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <stdexcept>

// Simple JSON value representation
// For production, consider using nlohmann/json, but this keeps dependencies minimal
namespace Json {

class Value;

using Null = std::monostate;
using Bool = bool;
using Int = int64_t;
using Float = double;
using String = std::string;
using Array = std::vector<Value>;
using Object = std::unordered_map<std::string, Value>;

class Value {
public:
    using Variant = std::variant<Null, Bool, Int, Float, String, Array, Object>;
    
    Value() : m_data(Null{}) {}
    Value(std::nullptr_t) : m_data(Null{}) {}
    Value(bool v) : m_data(v) {}
    Value(int v) : m_data(static_cast<Int>(v)) {}
    Value(int64_t v) : m_data(v) {}
    Value(double v) : m_data(v) {}
    Value(float v) : m_data(static_cast<Float>(v)) {}
    Value(const char* v) : m_data(String(v)) {}
    Value(const String& v) : m_data(v) {}
    Value(String&& v) : m_data(std::move(v)) {}
    Value(const Array& v) : m_data(v) {}
    Value(Array&& v) : m_data(std::move(v)) {}
    Value(const Object& v) : m_data(v) {}
    Value(Object&& v) : m_data(std::move(v)) {}
    
    // Type checks
    [[nodiscard]] bool IsNull() const { return std::holds_alternative<Null>(m_data); }
    [[nodiscard]] bool IsBool() const { return std::holds_alternative<Bool>(m_data); }
    [[nodiscard]] bool IsInt() const { return std::holds_alternative<Int>(m_data); }
    [[nodiscard]] bool IsFloat() const { return std::holds_alternative<Float>(m_data); }
    [[nodiscard]] bool IsString() const { return std::holds_alternative<String>(m_data); }
    [[nodiscard]] bool IsArray() const { return std::holds_alternative<Array>(m_data); }
    [[nodiscard]] bool IsObject() const { return std::holds_alternative<Object>(m_data); }
    [[nodiscard]] bool IsNumber() const { return IsInt() || IsFloat(); }
    
    // Getters with type checking
    [[nodiscard]] bool AsBool() const { return std::get<Bool>(m_data); }
    [[nodiscard]] Int AsInt() const { return std::get<Int>(m_data); }
    [[nodiscard]] Float AsFloat() const { 
        if (IsInt()) return static_cast<Float>(std::get<Int>(m_data));
        return std::get<Float>(m_data); 
    }
    [[nodiscard]] const String& AsString() const { return std::get<String>(m_data); }
    [[nodiscard]] const Array& AsArray() const { return std::get<Array>(m_data); }
    [[nodiscard]] const Object& AsObject() const { return std::get<Object>(m_data); }
    [[nodiscard]] Array& AsArray() { return std::get<Array>(m_data); }
    [[nodiscard]] Object& AsObject() { return std::get<Object>(m_data); }
    
    // Optional getters (safe)
    [[nodiscard]] std::optional<bool> GetBool() const {
        if (IsBool()) return AsBool();
        return std::nullopt;
    }
    [[nodiscard]] std::optional<Int> GetInt() const {
        if (IsInt()) return AsInt();
        return std::nullopt;
    }
    [[nodiscard]] std::optional<Float> GetFloat() const {
        if (IsNumber()) return AsFloat();
        return std::nullopt;
    }
    [[nodiscard]] std::optional<String> GetString() const {
        if (IsString()) return AsString();
        return std::nullopt;
    }
    
    // Object access
    [[nodiscard]] bool Has(const std::string& key) const {
        if (!IsObject()) return false;
        return AsObject().count(key) > 0;
    }
    
    [[nodiscard]] const Value& operator[](const std::string& key) const {
        static Value null;
        if (!IsObject()) return null;
        auto it = AsObject().find(key);
        return it != AsObject().end() ? it->second : null;
    }
    
    Value& operator[](const std::string& key) {
        if (!IsObject()) m_data = Object{};
        return AsObject()[key];
    }
    
    // Array access
    [[nodiscard]] const Value& operator[](size_t index) const {
        return AsArray().at(index);
    }
    
    Value& operator[](size_t index) {
        return AsArray().at(index);
    }
    
    [[nodiscard]] size_t Size() const {
        if (IsArray()) return AsArray().size();
        if (IsObject()) return AsObject().size();
        return 0;
    }
    
    // Serialization
    [[nodiscard]] std::string Stringify(bool pretty = false, int indent = 0) const;
    
    // Parsing
    static Value Parse(const std::string& json);

private:
    Variant m_data;
};

// Helper to build objects fluently
class ObjectBuilder {
public:
    ObjectBuilder& Add(const std::string& key, Value value) {
        m_obj[key] = std::move(value);
        return *this;
    }
    
    Value Build() { return Value(std::move(m_obj)); }
    operator Value() { return Build(); }
    
private:
    Object m_obj;
};

// Helper to build arrays fluently
class ArrayBuilder {
public:
    ArrayBuilder& Add(Value value) {
        m_arr.push_back(std::move(value));
        return *this;
    }
    
    Value Build() { return Value(std::move(m_arr)); }
    operator Value() { return Build(); }
    
private:
    Array m_arr;
};

inline ObjectBuilder MakeObject() { return ObjectBuilder{}; }
inline ArrayBuilder MakeArray() { return ArrayBuilder{}; }

} // namespace Json
