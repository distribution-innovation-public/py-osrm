#include "parameters/tripparameter_nb.h"

#include "engine/api/trip_parameters.hpp"
#include "utility/param_utility.h"
#include "parameters/parse_helpers.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <optional>

namespace nb = nanobind;
using namespace nb::literals;

void init_TripParameters(nb::module_& m) {
    using osrm::engine::api::RouteParameters;
    using osrm::engine::api::TripParameters;

    nb::class_<TripParameters, RouteParameters>(m, "TripParameters")
        .def(nb::init<>(), "Instantiates an instance of TripParameters.\n\n"
            "Examples:\n\
                >>> trip_params = osrm.TripParameters(\n\
                        coordinates = [(7.41337, 43.72956), (7.41546, 43.73077)],\n\
                        source = 'any',\n\
                        destination = 'last',\n\
                        roundtrip = False\n\
                    )\n\
                >>> trip_params.IsValid()\n\
                True\n\n"
            "Args:\n\
                source (string 'any' | 'first'): Returned route starts at 'any' or 'first' coordinate. (default '')\n\
                destination (string 'any' | 'last'): Returned route ends at 'any' or 'last' coordinate. (default '')\n\
                roundtrip (bool): Returned route is a roundtrip (route returns to first location). (default True)\n\
                RouteParameters (osrm.RouteParameters): Keyword arguments from parent class.\n\n"
            "Returns:\n\
                __init__ (osrm.TripParameters): A TripParameters object, for usage in Trip.\n\
                IsValid (bool): A bool value denoting validity of parameter values.\n\n"
            "Attributes:\n\
                source (string): Returned route starts at 'any' or 'first' coordinate.\n\
                destination (string): Returned route ends at 'any' or 'last' coordinate.\n\
                roundtrip (bool): Returned route is a roundtrip (route returns to first location).\n\
                RouteParameters (osrm.RouteParameters): Attributes from parent class."
            )
    .def("__init__", [](TripParameters* t, const nb::kwargs& kwargs){
        new (t) TripParameters();

        // RouteParameters defaults
        bool steps = false; int number_of_alternatives = 0; std::vector<RouteParameters::AnnotationsType> ann_enums;
        RouteParameters::GeometriesType geometries = RouteParameters::GeometriesType::Polyline;
        RouteParameters::OverviewType overview = RouteParameters::OverviewType::Simplified;
        std::optional<bool> continue_straight; std::vector<std::size_t> waypoints;
        // BaseParameters defaults
        std::vector<osrm::util::Coordinate> coordinates; std::vector<std::optional<osrm::engine::Hint>> hints;
        std::vector<std::optional<double>> radiuses; std::vector<std::optional<osrm::engine::Bearing>> bearings;
        std::vector<std::optional<osrm::engine::Approach>> approaches; bool generate_hints = true; std::vector<std::string> exclude;
        BaseParameters::SnappingType snapping = BaseParameters::SnappingType::Default;
        // Trip specific
        TripParameters::SourceType source = TripParameters::SourceType::Any;
        TripParameters::DestinationType destination = TripParameters::DestinationType::Any;
    bool roundtrip = true; // Align with documented default

    using namespace osrm_nb_parse;

        for(auto item: kwargs){
            std::string key = nb::cast<std::string>(item.first);
            if(key=="source") { if(nb::isinstance<nb::str>(item.second)) source = parse_trip_source(nb::cast<std::string>(item.second)); else source = nb::cast<TripParameters::SourceType>(item.second); }
            else if(key=="destination") { if(nb::isinstance<nb::str>(item.second)) destination = parse_trip_destination(nb::cast<std::string>(item.second)); else destination = nb::cast<TripParameters::DestinationType>(item.second); }
            else if(key=="roundtrip") roundtrip = nb::cast<bool>(item.second);
            else if(key=="steps") steps = nb::cast<bool>(item.second);
            else if(key=="number_of_alternatives" || key=="alternatives") number_of_alternatives = nb::cast<int>(item.second);
            else if(key=="annotations") { ann_enums.clear(); for(nb::handle h: nb::iter(item.second)) { if(nb::isinstance<nb::str>(h)) ann_enums.push_back(parse_route_annotation(nb::cast<std::string>(h))); else ann_enums.push_back(nb::cast<RouteParameters::AnnotationsType>(h)); }}
            else if(key=="geometries") { if(nb::isinstance<nb::str>(item.second)) geometries = parse_geometries(nb::cast<std::string>(item.second)); else geometries = nb::cast<RouteParameters::GeometriesType>(item.second); }
            else if(key=="overview") { if(nb::isinstance<nb::str>(item.second)) overview = parse_overview(nb::cast<std::string>(item.second)); else overview = nb::cast<RouteParameters::OverviewType>(item.second); }
            else if(key=="continue_straight") { if(item.second.is_none()) continue_straight.reset(); else continue_straight = nb::cast<bool>(item.second); }
            else if(key=="waypoints") { for(nb::handle h: nb::iter(item.second)) waypoints.push_back(nb::cast<std::size_t>(h)); }
            else if(key=="coordinates") { for(nb::handle h: nb::iter(item.second)) { if(nb::isinstance<nb::tuple>(h)) { auto tup=nb::tuple(h); if(tup.size()!=2) throw std::runtime_error("Coordinate tuple must have length 2"); double lon=nb::cast<double>(tup[0]); double lat=nb::cast<double>(tup[1]); coordinates.emplace_back(osrm::util::FloatLongitude{lon}, osrm::util::FloatLatitude{lat}); } else coordinates.push_back(nb::cast<osrm::util::Coordinate>(h)); }}
            else if(key=="exclude") { for(nb::handle h: nb::iter(item.second)) exclude.push_back(nb::cast<std::string>(h)); }
            else if(key=="generate_hints") generate_hints = nb::cast<bool>(item.second);
            else if(key=="snapping") { if(nb::isinstance<nb::str>(item.second)) snapping = parse_snapping(nb::cast<std::string>(item.second)); else snapping = nb::cast<BaseParameters::SnappingType>(item.second); }
            else if(key=="hints") { for(nb::handle h: nb::iter(item.second)) hints.push_back(nb::cast<std::optional<osrm::engine::Hint>>(h)); }
            else if(key=="radiuses") { for(nb::handle h: nb::iter(item.second)) { if(h.is_none()) radiuses.push_back(std::optional<double>()); else radiuses.push_back(nb::cast<double>(h)); }}
            else if(key=="bearings") { for(nb::handle h: nb::iter(item.second)) bearings.push_back(nb::cast<std::optional<osrm::engine::Bearing>>(h)); }
            else if(key=="approaches") { for(nb::handle h: nb::iter(item.second)) approaches.push_back(nb::cast<std::optional<osrm::engine::Approach>>(h)); }
            else if(key=="alternatives") { number_of_alternatives = nb::cast<int>(item.second); }
            else {
                throw std::invalid_argument("Unknown TripParameters argument: "+key);
            }
        }

        t->source = source; t->destination = destination; t->roundtrip = roundtrip;
        osrm_nb_util::assign_routeparameters(t, steps, number_of_alternatives, ann_enums, geometries, overview, continue_straight, waypoints);
        osrm_nb_util::assign_baseparameters(t, std::move(coordinates), std::move(hints), std::move(radiuses), std::move(bearings), approaches, generate_hints, std::move(exclude), snapping);
    })
        .def_rw("source", &TripParameters::source)
        .def_rw("destination", &TripParameters::destination)
        .def_rw("roundtrip", &TripParameters::roundtrip)
        .def("IsValid", &TripParameters::IsValid);

    nb::enum_<TripParameters::SourceType>(m, "TripSourceType")
        .value("Any", TripParameters::SourceType::Any)
        .value("First", TripParameters::SourceType::First);

    nb::enum_<TripParameters::DestinationType>(m, "TripDestinationType")
        .value("Any", TripParameters::DestinationType::Any)
        .value("Last", TripParameters::DestinationType::Last);
}
