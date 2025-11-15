#include "pyl_text.h"

namespace pyl {

void add_field(FieldMap& m, std::string name, std::any value) {
    m.emplace(std::move(name), std::move(value));
}

std::string any_to_string(const std::any& a) {
    if (!a.has_value()) return "<null>";

    const std::type_info& t = a.type();
    if (t == typeid(int))         return std::to_string(std::any_cast<int>(a));
    if (t == typeid(double))      return std::to_string(std::any_cast<double>(a));
    if (t == typeid(float))       return std::to_string(std::any_cast<float>(a));
    if (t == typeid(long))        return std::to_string(std::any_cast<long>(a));
    if (t == typeid(unsigned))    return std::to_string(std::any_cast<unsigned>(a));
    if (t == typeid(std::string)) return std::any_cast<std::string>(a);
    if (t == typeid(const char*)) return std::string(std::any_cast<const char*>(a));
    if (t == typeid(char*))       return std::string(std::any_cast<char*>(a));
    if (t == typeid(bool))        return std::any_cast<bool>(a) ? "true" : "false";

    return "<unknown>";
}

} // namespace pyl
