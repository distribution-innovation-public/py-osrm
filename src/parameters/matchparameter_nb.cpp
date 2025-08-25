#include "parameters/matchparameter_nb.h"

#include "engine/api/match_parameters.hpp"
#include "utility/param_utility.h"
#include "parameters/parse_helpers.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <optional>

namespace nb = nanobind;
using namespace nb::literals;

void init_MatchParameters(nb::module_& m) {
    using osrm::engine::api::RouteParameters;
    using osrm::engine::api::MatchParameters;

    nb::class_<MatchParameters, RouteParameters> match_cls(m, "MatchParameters");
    match_cls.def(nb::init<>(), "Instantiates an instance of MatchParameters.\n\n"
            "Examples:\n\
                >>> match_params = osrm.MatchParameters(\n\
                        coordinates = [(7.41337, 43.72956), (7.41546, 43.73077), (7.41862, 43.73216)],\n\
                        timestamps = [1424684612, 1424684616, 1424684620],\n\
                        gaps = 'split',\n\
                        tidy = True\n\
                    )\n\
                >>> match_params.IsValid()\n\
                True\n\n"
            "Args:\n\
                timestamps (list of unsigned int): Timestamps for the input locations in seconds since UNIX epoch. (default [])\n\
                gaps ('split' | 'ignore'): Allows the input track splitting based on huge timestamp gaps between points. (default '')\n\
                tidy (bool): Allows input track modification for noisy tracks. (default False)\n\
                RouteParameters (osrm.RouteParameters): Keyword arguments from parent class.\n\n"
            "Returns:\n\
                __init__ (osrm.MatchParameters): A MatchParameters object, for usage in Match.\n\
                IsValid (bool): A bool value denoting validity of parameter values.\n\n"
            "Attributes:\n\
                timestamps (list of unsigned int)\n\
                gaps (string)\n\
                tidy (bool)\n\
                RouteParameters (osrm.RouteParameters): Attributes from parent class."
            )
        .def("__init__", [](MatchParameters* t, const nb::kwargs& kwargs){
            new (t) MatchParameters();

            // Inherited RouteParameters defaults (keep in sync with RouteParameters binding)
            bool steps = false;
            int number_of_alternatives = 0;
            std::vector<RouteParameters::AnnotationsType> ann_enums;
            RouteParameters::GeometriesType geometries = RouteParameters::GeometriesType::Polyline;
            RouteParameters::OverviewType overview = RouteParameters::OverviewType::Simplified;
            std::optional<bool> continue_straight;
            std::vector<std::size_t> waypoints;
            // BaseParameters defaults
            std::vector<osrm::util::Coordinate> coordinates;
            std::vector<std::optional<osrm::engine::Hint>> hints;
            std::vector<std::optional<double>> radiuses;
            std::vector<std::optional<osrm::engine::Bearing>> bearings;
            std::vector<std::optional<osrm::engine::Approach>> approaches;
            bool generate_hints = true;
            std::vector<std::string> exclude;
            BaseParameters::SnappingType snapping = BaseParameters::SnappingType::Default;
            // Match specific
            std::vector<unsigned> timestamps;
            MatchParameters::GapsType gaps_type = MatchParameters::GapsType::Split; // default if not provided? Use Split as neutral
            bool tidy = false;

            using namespace osrm_nb_parse;

            for(auto item : kwargs){
                std::string key = nb::cast<std::string>(item.first);
                if(key=="timestamps") {
                    for(nb::handle h : nb::iter(item.second)) timestamps.push_back(nb::cast<unsigned>(h));
                } else if(key=="gaps") {
                    if(nb::isinstance<nb::str>(item.second)) gaps_type = parse_gaps(nb::cast<std::string>(item.second));
                    else gaps_type = nb::cast<MatchParameters::GapsType>(item.second);
                } else if(key=="tidy") tidy = nb::cast<bool>(item.second);
                else if(key=="steps") steps = nb::cast<bool>(item.second);
                else if(key=="number_of_alternatives") number_of_alternatives = nb::cast<int>(item.second);
                else if(key=="annotations") {
                    ann_enums.clear();
                    for(nb::handle h : nb::iter(item.second)) {
                        if(nb::isinstance<nb::str>(h)) ann_enums.push_back(parse_route_annotation(nb::cast<std::string>(h)));
                        else ann_enums.push_back(nb::cast<RouteParameters::AnnotationsType>(h));
                    }
                }
                else if(key=="geometries") {
                    if(nb::isinstance<nb::str>(item.second)) geometries = parse_geometries(nb::cast<std::string>(item.second));
                    else geometries = nb::cast<RouteParameters::GeometriesType>(item.second);
                }
                else if(key=="overview") {
                    if(nb::isinstance<nb::str>(item.second)) overview = parse_overview(nb::cast<std::string>(item.second));
                    else overview = nb::cast<RouteParameters::OverviewType>(item.second);
                }
                else if(key=="continue_straight") {
                    if(item.second.is_none()) continue_straight.reset(); else continue_straight = nb::cast<bool>(item.second);
                }
                else if(key=="waypoints") {
                    for(nb::handle h : nb::iter(item.second)) waypoints.push_back(nb::cast<std::size_t>(h));
                }
                else if(key=="coordinates") {
                    for(nb::handle h : nb::iter(item.second)) {
                        if(nb::isinstance<nb::tuple>(h)) {
                            auto tup = nb::tuple(h);
                            if(tup.size()!=2) throw std::runtime_error("Coordinate tuple must have length 2");
                            double lon = nb::cast<double>(tup[0]);
                            double lat = nb::cast<double>(tup[1]);
                            coordinates.emplace_back(osrm::util::FloatLongitude{lon}, osrm::util::FloatLatitude{lat});
                        } else {
                            coordinates.push_back(nb::cast<osrm::util::Coordinate>(h));
                        }
                    }
                }
                else if(key=="exclude") {
                    for(nb::handle h : nb::iter(item.second)) exclude.push_back(nb::cast<std::string>(h));
                }
                else if(key=="generate_hints") generate_hints = nb::cast<bool>(item.second);
                else if(key=="snapping") {
                    if(nb::isinstance<nb::str>(item.second)) snapping = parse_snapping(nb::cast<std::string>(item.second));
                    else snapping = nb::cast<BaseParameters::SnappingType>(item.second);
                }
                else if(key=="hints") {
                    for(nb::handle h : nb::iter(item.second)) hints.push_back(nb::cast<std::optional<osrm::engine::Hint>>(h));
                }
                else if(key=="radiuses") {
                    for(nb::handle h : nb::iter(item.second)) {
                        if(h.is_none()) radiuses.push_back(std::optional<double>()); else radiuses.push_back(nb::cast<double>(h));
                    }
                }
                else if(key=="bearings") {
                    for(nb::handle h : nb::iter(item.second)) bearings.push_back(nb::cast<std::optional<osrm::engine::Bearing>>(h));
                }
                else if(key=="approaches") {
                    for(nb::handle h : nb::iter(item.second)) approaches.push_back(nb::cast<std::optional<osrm::engine::Approach>>(h));
                }
                else {
                    throw std::invalid_argument("Unknown MatchParameters argument: " + key);
                }
            }

            t->timestamps = std::move(timestamps);
            t->gaps = gaps_type;
            t->tidy = tidy;

            osrm_nb_util::assign_routeparameters(t,
                                                 steps,
                                                 number_of_alternatives,
                                                 ann_enums,
                                                 geometries,
                                                 overview,
                                                 continue_straight,
                                                 waypoints);

            osrm_nb_util::assign_baseparameters(t,
                                                std::move(coordinates),
                                                std::move(hints),
                                                std::move(radiuses),
                                                std::move(bearings),
                                                approaches,
                                                generate_hints,
                                                std::move(exclude),
                                                snapping);
        })
        .def_rw("timestamps", &MatchParameters::timestamps)
        .def_rw("gaps", &MatchParameters::gaps)
        .def_rw("tidy", &MatchParameters::tidy)
        .def("IsValid", &MatchParameters::IsValid);
    // Expose enum directly (strings can be mapped manually in Python if needed)
    nb::enum_<MatchParameters::GapsType>(m, "MatchGapsType")
        .value("Split", MatchParameters::GapsType::Split)
        .value("Ignore", MatchParameters::GapsType::Ignore);
}
