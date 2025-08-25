#include "parameters/nearestparameter_nb.h"

#include "engine/api/nearest_parameters.hpp"
#include "utility/param_utility.h"
#include "parameters/parse_helpers.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <optional>

namespace nb = nanobind;
using namespace nb::literals;

void init_NearestParameters(nb::module_& m) {
    using osrm::engine::api::BaseParameters;
    using osrm::engine::api::NearestParameters;

    nb::class_<NearestParameters, BaseParameters>(m, "NearestParameters")
        .def(nb::init<>(), "Instantiates an instance of NearestParameters.\n\n"
            "Examples:\n\
                >>> nearest_params = osrm.NearestParameters(\n\
                        coordinates = [(7.41337, 43.72956)],\n\
                        exclude = ['motorway']\n\
                    )\n\
                >>> nearest_params.IsValid()\n\
                True\n\n"
            "Args:\n\
                BaseParameters (osrm.osrm_ext.BaseParameters): Keyword arguments from parent class.\n\n"
            "Returns:\n\
                __init__ (osrm.NearestParameters): A NearestParameters object, for usage in osrm.OSRM.Nearest.\n\
                IsValid (bool): A bool value denoting validity of parameter values.\n\n"
            "Attributes:\n\
                number_of_results (unsigned int): Number of nearest segments that should be returned.\n\
                BaseParameters (osrm.osrm_ext.BaseParameters): Attributes from parent class."
            )
    .def("__init__", [](NearestParameters* t, const nb::kwargs& kwargs){
        new (t) NearestParameters();
        std::vector<osrm::util::Coordinate> coordinates; std::vector<std::optional<osrm::engine::Hint>> hints; std::vector<std::optional<double>> radiuses; std::vector<std::optional<osrm::engine::Bearing>> bearings; std::vector<std::optional<osrm::engine::Approach>> approaches; bool generate_hints = true; std::vector<std::string> exclude; BaseParameters::SnappingType snapping = BaseParameters::SnappingType::Default; unsigned int number_of_results = t->number_of_results; // default 1 normally

    using namespace osrm_nb_parse;

        for(auto item: kwargs){
            std::string key = nb::cast<std::string>(item.first);
            if(key=="coordinates") { for(nb::handle h: nb::iter(item.second)) { if(nb::isinstance<nb::tuple>(h)) { auto tup=nb::tuple(h); if(tup.size()!=2) throw std::runtime_error("Coordinate tuple must have length 2"); double lon=nb::cast<double>(tup[0]); double lat=nb::cast<double>(tup[1]); coordinates.emplace_back(osrm::util::FloatLongitude{lon}, osrm::util::FloatLatitude{lat}); } else coordinates.push_back(nb::cast<osrm::util::Coordinate>(h)); }}
            else if(key=="hints") { for(nb::handle h: nb::iter(item.second)) hints.push_back(nb::cast<std::optional<osrm::engine::Hint>>(h)); }
            else if(key=="radiuses") { for(nb::handle h: nb::iter(item.second)) { if(h.is_none()) radiuses.push_back(std::optional<double>()); else radiuses.push_back(nb::cast<double>(h)); } }
            else if(key=="bearings") { for(nb::handle h: nb::iter(item.second)) bearings.push_back(nb::cast<std::optional<osrm::engine::Bearing>>(h)); }
            else if(key=="approaches") { for(nb::handle h: nb::iter(item.second)) approaches.push_back(nb::cast<std::optional<osrm::engine::Approach>>(h)); }
            else if(key=="generate_hints") generate_hints = nb::cast<bool>(item.second);
            else if(key=="exclude") { for(nb::handle h: nb::iter(item.second)) exclude.push_back(nb::cast<std::string>(h)); }
            else if(key=="snapping") { if(nb::isinstance<nb::str>(item.second)) snapping = parse_snapping(nb::cast<std::string>(item.second)); else snapping = nb::cast<BaseParameters::SnappingType>(item.second); }
            else if(key=="number_of_results") number_of_results = nb::cast<unsigned int>(item.second);
            else {
                throw std::invalid_argument("Unknown NearestParameters argument: "+key);
            }
        }

        t->number_of_results = number_of_results;
        osrm_nb_util::assign_baseparameters(t, std::move(coordinates), std::move(hints), std::move(radiuses), std::move(bearings), approaches, generate_hints, std::move(exclude), snapping);
    })
        .def_rw("number_of_results", &NearestParameters::number_of_results)
        .def("IsValid", &NearestParameters::IsValid);
}
