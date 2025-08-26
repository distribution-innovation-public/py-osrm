#ifndef OSRM_NB_JSONCONTAINER_H
#define OSRM_NB_JSONCONTAINER_H

#include "util/json_container.hpp"
// Rewritten to use std::variant directly (OSRM >=5.30 uses std::variant internally)
// Removed dependency on mapbox/variant and recursive_wrapper.

#include <nanobind/nanobind.h>
#include <variant>

#include <string>

void init_JSONContainer(nanobind::module_& m);

namespace json = osrm::util::json;
namespace nb = nanobind; // alias used in visitors below
using JSONValue = json::Value; // alias retained for backwards compatibility
// Rely on nanobind's native std::variant + basic types caster support.

struct ValueStringifyVisitor {
    std::string operator()(const json::String& str) { return "'" + str.value + "'"; }
    std::string operator()(const json::Number& num) { return std::to_string(num.value); }
    std::string operator()(const json::True&) { return "True"; }
    std::string operator()(const json::False&) { return "False"; }
    std::string operator()(const json::Null&) { return "None"; }
    std::string operator()(const json::Array& arr) { return visitarray(arr); }
    std::string operator()(const json::Object& obj) { return visitobject(obj); }

    std::string visitarray(const json::Array& arr) {
        std::string output = "[";
        for (std::size_t i = 0; i < arr.values.size(); ++i) {
            if (i != 0) output += ", ";
            output += std::visit(*this, arr.values[i]);
        }
        return output + "]";
    }
    std::string visitobject(const json::Object& obj) {
        std::string output = "{";
        bool first = true;
        for (auto&& kv : obj.values) {
            if (!first) output += ", ";
            output += "'" + std::string(kv.first) + "': ";
            output += std::visit(*this, kv.second);
            first = false;
        }
        return output + "}";
    }
};

// Visitor converting OSRM json::Value (std::variant) into native Python objects.
struct ToPythonVisitor {
    nb::object operator()(const json::String &s) const { return nb::str(s.value.c_str(), s.value.size()); }
    nb::object operator()(const json::Number &n) const { return nb::float_(n.value); }
    nb::object operator()(const json::True &) const { return nb::bool_(true); }
    nb::object operator()(const json::False &) const { return nb::bool_(false); }
    nb::object operator()(const json::Null &) const { return nb::none(); }
    nb::object operator()(const json::Array &arr) const {
        nb::list lst; for (auto const &v : arr.values) lst.append(std::visit(*this, v)); return lst; }
    nb::object operator()(const json::Object &obj) const {
        nb::dict d; for (auto const &kv : obj.values) d[ nb::str(kv.first.data(), kv.first.size()) ] = std::visit(*this, kv.second); return d; }
};

inline nb::object json_value_to_py(const json::Value &v) { return std::visit(ToPythonVisitor{}, v); }
inline nb::dict json_object_to_py(const json::Object &o) {
    ToPythonVisitor vis; return nb::cast<nb::dict>(vis(o)); }


#endif //OSRM_NB_JSONCONTAINER_H
