#include "parameters/routeparameter_nb.h"

#include "engine/api/route_parameters.hpp"
#include "utility/param_utility.h"
#include "parameters/parse_helpers.h"

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <algorithm>
#include <vector>


namespace nb = nanobind;
using namespace nb::literals;

void init_RouteParameters(nb::module_& m) {
    using osrm::engine::api::BaseParameters;
    using osrm::engine::api::RouteParameters;

    nb::class_<RouteParameters, BaseParameters>(m, "RouteParameters")
    .def(nb::init<>(), "Instantiates an instance of RouteParameters.\n\n"
            "Examples:\n\
                >>> route_params = osrm.RouteParameters(\n\
                        coordinates = [(7.41337, 43.72956), (7.41546, 43.73077)],\n\
                        steps = True,\n\
                        number_of_alternatives = 3,\n\
                        annotations = ['speed'],\n\
                        geometries = 'polyline',\n\
                        overview = 'simplified',\n\
                        continue_straight = False,\n\
                        waypoints = [0, 1],\n\
                        radiuses = [4.07, 4.07],\n\
                        bearings = [(200, 180), (250, 180)],\n\
                        # approaches = ['unrestricted', 'unrestricted'],\n\
                        generate_hints = False,\n\
                        exclude = ['motorway'],\n\
                        snapping = 'any'\n\
                    )\n\
                >>> route_params.IsValid()\n\
                True\n\n"
            "Args:\n\
                steps (bool): Return route steps for each route leg. (default False)\n\
                number_of_alternatives (int): Search for n alternative routes. (default 0)\n\
                annotations (list of 'none' | 'duration' |  'nodes' | 'distance' | 'weight' | 'datasources' \
                    | 'speed' | 'all'): Returns additional metadata for each coordinate along the route geometry. (default [])\n\
                geometries (string 'polyline' | 'polyline6' | 'geojson'): Returned route geometry format - influences overview and per step. (default "")\n\
                overview (string 'simplified' | 'full' | 'false'): Add overview geometry either full, simplified. (default '')\n\
                continue_straight (bool): Forces the route to keep going straight at waypoints, constraining u-turns. (default {})\n\
                waypoints (list of int): Treats input coordinates indicated by given indices as waypoints in returned Match object. (default [])\n\
                BaseParameters (osrm.osrm_ext.BaseParameters): Keyword arguments from parent class.\n\n"
            "Returns:\n\
                __init__ (osrm.RouteParameters): A RouteParameters object, for usage in Route.\n\
                IsValid (bool): A bool value denoting validity of parameter values.\n\n"
            "Attributes:\n\
                steps (bool): Return route steps for each route leg.\n\
                alternatives (bool): Search for alternative routes.\n\
                number_of_alternatives (int): Search for n alternative routes.\n\
                annotations_type (string): Returns additional metadata for each coordinate along the route geometry.\n\
                geometries (string): Returned route geometry format - influences overview and per step.\n\
                overview (string): Add overview geometry either full, simplified.\n\
                continue_straight (bool): Forces the route to keep going straight at waypoints, constraining u-turns.\n\
                BaseParameters (osrm.osrm_ext.BaseParameters): Attributes from parent class."
            )
    .def("__init__", [](RouteParameters* t, const nb::kwargs& kwargs) {
        new (t) RouteParameters();

        // Defaults
        bool steps = false;
        int number_of_alternatives = 0;
        std::vector<RouteParameters::AnnotationsType> ann_enums;
        RouteParameters::GeometriesType geometries = RouteParameters::GeometriesType::Polyline; // default empty previously
        RouteParameters::OverviewType overview = RouteParameters::OverviewType::Simplified; // mimic previous default ""
        std::optional<bool> continue_straight;
        std::vector<std::size_t> waypoints;
        std::vector<osrm::util::Coordinate> coordinates;
        std::vector<std::optional<osrm::engine::Hint>> hints;
        std::vector<std::optional<double>> radiuses;
        std::vector<std::optional<osrm::engine::Bearing>> bearings;
        std::vector<std::optional<osrm::engine::Approach>> approaches;
        bool generate_hints = true;
        std::vector<std::string> exclude;
        BaseParameters::SnappingType snapping = BaseParameters::SnappingType::Default;

    using namespace osrm_nb_parse;

        for (auto item : kwargs) {
            std::string key = nb::cast<std::string>(item.first);
            if(key=="steps") steps = nb::cast<bool>(item.second);
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
            // Other lists (hints, radiuses, bearings, approaches) kept simple: only accept already-typed values for now
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
                throw std::invalid_argument("Unknown RouteParameters argument: " + key);
            }
        }

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
        .def_rw("steps", &RouteParameters::steps)
        .def_rw("alternatives", &RouteParameters::alternatives)
        .def_rw("number_of_alternatives", &RouteParameters::number_of_alternatives)
        .def_rw("annotations_type", &RouteParameters::annotations_type)
        .def_rw("geometries", &RouteParameters::geometries)
        .def_rw("overview", &RouteParameters::overview)
        .def_rw("continue_straight", &RouteParameters::continue_straight)
        .def("IsValid", &RouteParameters::IsValid);

    // Enums: expose directly; Python can still use strings via helper constructors if needed later.
    nb::enum_<RouteParameters::GeometriesType>(m, "RouteGeometriesType")
        .value("Polyline", RouteParameters::GeometriesType::Polyline)
        .value("Polyline6", RouteParameters::GeometriesType::Polyline6)
        .value("GeoJSON", RouteParameters::GeometriesType::GeoJSON);

    nb::enum_<RouteParameters::OverviewType>(m, "RouteOverviewType")
        .value("Simplified", RouteParameters::OverviewType::Simplified)
        .value("Full", RouteParameters::OverviewType::Full)
        .value("False", RouteParameters::OverviewType::False);

    nb::enum_<RouteParameters::AnnotationsType>(m, "RouteAnnotationsType")
        .value("None", RouteParameters::AnnotationsType::None)
        .value("Duration", RouteParameters::AnnotationsType::Duration)
        .value("Nodes", RouteParameters::AnnotationsType::Nodes)
        .value("Distance", RouteParameters::AnnotationsType::Distance)
        .value("Weight", RouteParameters::AnnotationsType::Weight)
        .value("Datasources", RouteParameters::AnnotationsType::Datasources)
        .value("Speed", RouteParameters::AnnotationsType::Speed)
        .value("All", RouteParameters::AnnotationsType::All);
}
