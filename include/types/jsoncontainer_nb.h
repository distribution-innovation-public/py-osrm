#ifndef OSRM_NB_JSONCONTAINER_H
#define OSRM_NB_JSONCONTAINER_H

#include "util/json_container.hpp"
// Rewritten to use std::variant directly (OSRM >=5.30 uses std::variant internally)
// Removed dependency on mapbox/variant and recursive_wrapper.

#include <nanobind/nanobind.h>

#include <string>

void init_JSONContainer(nanobind::module_& m);

namespace json = osrm::util::json;
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

#endif //OSRM_NB_JSONCONTAINER_H
