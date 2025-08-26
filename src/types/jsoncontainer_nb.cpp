#include "types/jsoncontainer_nb.h"

#include "util/json_container.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/make_iterator.h>
#include <nanobind/stl/string.h>
#include <variant>

namespace nb = nanobind;
namespace json = osrm::util::json;

void init_JSONContainer(nb::module_& m) {
    nb::class_<json::Object>(m, "Object")
        .def(nb::init<>())
        .def("__len__", [](const json::Object& obj) {
            return obj.values.size();
        })
        .def("__bool__", [](const json::Object& obj) {
            return !obj.values.empty();
        })
        .def("__repr__", [](const json::Object& obj) {
            ValueStringifyVisitor visitor;
            return visitor.visitobject(obj);
        })
        .def("__getitem__", [](const json::Object& obj, const std::string& key) -> nb::object {
            auto it = obj.values.find(key);
            if (it == obj.values.end()) throw nb::key_error((std::string("Key not found: ") + key).c_str());
            return std::visit(ToPythonVisitor{}, it->second);
        })
        .def("get", [](const json::Object& obj, const std::string& key, nb::object default_) -> nb::object {
            auto it = obj.values.find(key);
            if (it == obj.values.end()) return default_;
            return std::visit(ToPythonVisitor{}, it->second);
        }, nb::arg("key"), nb::arg("default") = nb::none())
        .def("keys", [](const json::Object& obj) {
            nb::list keys; for (auto const &kv : obj.values) keys.append(nb::str(kv.first.data(), kv.first.size())); return keys; })
        .def("values", [](const json::Object& obj) {
            nb::list vals; for (auto const &kv : obj.values) vals.append(json_value_to_py(kv.second)); return vals; })
        .def("items", [](const json::Object& obj) {
            nb::list items; for (auto const &kv : obj.values) { nb::tuple t = nb::make_tuple(nb::str(kv.first.data(), kv.first.size()), json_value_to_py(kv.second)); items.append(t);} return items; })
        .def("__iter__", [](const json::Object& obj) {
            nb::list keys;
            for (auto const &kv : obj.values) keys.append(nb::str(kv.first.data(), kv.first.size()));
            return keys; // list is iterable
        });

    nb::class_<json::Array>(m, "Array")
        .def(nb::init<>())
        .def("__len__", [](const json::Array& arr) {
            return arr.values.size();
        })
        .def("__bool__", [](const json::Array& arr) {
            return !arr.values.empty();
        })
        .def("__repr__", [](const json::Array& arr) {
            ValueStringifyVisitor visitor;
            return visitor.visitarray(arr);
        })
        .def("__getitem__", [](const json::Array& arr, int i) -> nb::object {
            if (i < 0 || static_cast<size_t>(i) >= arr.values.size()) throw nb::index_error();
            return json_value_to_py(arr.values[i]);
        })
        .def("__iter__", [](const json::Array& arr) {
            nb::list out; for (auto const &v : arr.values) out.append(json_value_to_py(v)); return out; })
        .def("to_list", [](const json::Array& arr) {
            nb::list out; for (auto const &v : arr.values) out.append(json_value_to_py(v)); return out; })
        .def("append", [](json::Array& arr, nb::object) {
            throw nb::type_error("Mutation not supported");
        })
        ;

    nb::class_<json::String>(m, "String")
        .def(nb::init<std::string>());
    nb::class_<json::Number>(m, "Number")
        .def(nb::init<double>());

    nb::class_<json::True>(m, "True")
        .def(nb::init<>());
    nb::class_<json::False>(m, "False")
        .def(nb::init<>());
    nb::class_<json::Null>(m, "Null")
        .def(nb::init<>());
}
