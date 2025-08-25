#include "parameters/tableparameter_nb.h"

#include "engine/api/table_parameters.hpp"
#include "utility/param_utility.h"
#include "parameters/parse_helpers.h"

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <unordered_map>

namespace nb = nanobind;
using namespace nb::literals;

void init_TableParameters(nb::module_& m) {
    using osrm::engine::api::BaseParameters;
    using osrm::engine::api::TableParameters;
static const std::unordered_map<std::string, TableParameters::AnnotationsType> table_annotations_map {
    { "none", TableParameters::AnnotationsType::None },
    { std::string(), TableParameters::AnnotationsType::None },
    { "duration", TableParameters::AnnotationsType::Duration },
    { "distance", TableParameters::AnnotationsType::Distance },
    { "all", TableParameters::AnnotationsType::All }
};

    nb::class_<TableParameters, BaseParameters>(m, "TableParameters")
        .def(nb::init<>(), "Instantiates an instance of TableParameters.\n\n"
            "Examples:\n\
                >>> table_params = osrm.TableParameters(\n\
                        coordinates = [(7.41337, 43.72956), (7.41546, 43.73077)],\n\
                        sources = [0],\n\
                        destinations = [1],\n\
                        annotations = ['duration'],\n\
                        fallback_speed = 1,\n\
                        fallback_coordinate_type = 'input',\n\
                        scale_factor = 0.9\n\
                    )\n\
                >>> table_params.IsValid()\n\
                True\n\n"
            "Args:\n\
                sources (list of int): Use location with given index as source. (default [])\n\
                destinations (list of int): Use location with given index as destination. (default [])\n\
                annotations (list of 'none' | 'duration' | 'distance' | 'all'): \
                    Returns additional metadata for each coordinate along the route geometry. (default [])\n\
                fallback_speed (float): If no route found between a source/destination pair, calculate the as-the-crow-flies distance, \
                    then use this speed to estimate duration. (default INVALID_FALLBACK_SPEED)\n\
                fallback_coordinate_type (string 'input' | 'snapped'): When using a fallback_speed, use the user-supplied coordinate (input), \
                    or the snapped location (snapped) for calculating distances. (default '')\n\
                scale_factor: Scales the table duration values by this number (use in conjunction with annotations=durations). (default 1.0)\n\
                BaseParameters (osrm.osrm_ext.BaseParameters): Keyword arguments from parent class.\n\n"
            "Returns:\n\
                __init__ (osrm.TableParameters): A TableParameters object, for usage in Table.\n\
                IsValid (bool): A bool value denoting validity of parameter values.\n\n"
            "Attributes:\n\
                sources (list of int): Use location with given index as source.\n\
                destinations (list of int): Use location with given index as destination.\n\
                annotations (string): Returns additional metadata for each coordinate along the route geometry.\n\
                fallback_speed (float): If no route found between a source/destination pair, calculate the as-the-crow-flies distance, \
                    then use this speed to estimate duration.\n\
                fallback_coordinate_type (string): When using a fallback_speed, use the user-supplied coordinate (input), \
                    or the snapped location (snapped) for calculating distances.\n\
                scale_factor: Scales the table duration values by this number (use in conjunction with annotations=durations).\n\
                BaseParameters (osrm.osrm_ext.BaseParameters): Attributes from parent class."
            )
    .def("__init__", [](TableParameters* t, const nb::kwargs& kwargs){
        new (t) TableParameters();

        std::vector<std::size_t> sources;
        std::vector<std::size_t> destinations;
        std::vector<TableParameters::AnnotationsType> annotations_vec; // collected then bit-or
        double fallback_speed = osrm::from_alias<double>(INVALID_FALLBACK_SPEED);
        TableParameters::FallbackCoordinateType fallback_coordinate_type = TableParameters::FallbackCoordinateType::Input;
        double scale_factor = 1.0;
        std::vector<osrm::util::Coordinate> coordinates;
        std::vector<std::optional<osrm::engine::Hint>> hints;
        std::vector<std::optional<double>> radiuses;
        std::vector<std::optional<osrm::engine::Bearing>> bearings;
        std::vector<std::optional<osrm::engine::Approach>> approaches;
        bool generate_hints = true;
        std::vector<std::string> exclude;
        BaseParameters::SnappingType snapping = BaseParameters::SnappingType::Default;

    using namespace osrm_nb_parse;

        for(auto item: kwargs){
            std::string key = nb::cast<std::string>(item.first);
            if(key=="sources") { for(nb::handle h: nb::iter(item.second)) sources.push_back(nb::cast<std::size_t>(h)); }
            else if(key=="destinations") { for(nb::handle h: nb::iter(item.second)) destinations.push_back(nb::cast<std::size_t>(h)); }
        else if(key=="annotations") {
                annotations_vec.clear();
                for(nb::handle h: nb::iter(item.second)) {
            if(nb::isinstance<nb::str>(h)) annotations_vec.push_back(parse_table_annotation(nb::cast<std::string>(h)));
                    else annotations_vec.push_back(nb::cast<TableParameters::AnnotationsType>(h));
                }
            }
            else if(key=="fallback_speed") fallback_speed = nb::cast<double>(item.second);
            else if(key=="fallback_coordinate_type") {
                if(nb::isinstance<nb::str>(item.second)) fallback_coordinate_type = parse_fallback_coordinate_type(nb::cast<std::string>(item.second));
                else fallback_coordinate_type = nb::cast<TableParameters::FallbackCoordinateType>(item.second);
            }
            else if(key=="scale_factor") scale_factor = nb::cast<double>(item.second);
            else if(key=="coordinates") {
                for(nb::handle h: nb::iter(item.second)) {
                    if(nb::isinstance<nb::tuple>(h)) {
                        auto tup = nb::tuple(h); if(tup.size()!=2) throw std::runtime_error("Coordinate tuple must have length 2");
                        double lon = nb::cast<double>(tup[0]); double lat = nb::cast<double>(tup[1]);
                        coordinates.emplace_back(osrm::util::FloatLongitude{lon}, osrm::util::FloatLatitude{lat});
                    } else coordinates.push_back(nb::cast<osrm::util::Coordinate>(h));
                }
            }
            else if(key=="exclude") { for(nb::handle h: nb::iter(item.second)) exclude.push_back(nb::cast<std::string>(h)); }
            else if(key=="generate_hints") generate_hints = nb::cast<bool>(item.second);
            else if(key=="snapping") {
                if(nb::isinstance<nb::str>(item.second)) snapping = parse_snapping(nb::cast<std::string>(item.second));
                else snapping = nb::cast<BaseParameters::SnappingType>(item.second);
            }
            else if(key=="hints") { for(nb::handle h: nb::iter(item.second)) hints.push_back(nb::cast<std::optional<osrm::engine::Hint>>(h)); }
            else if(key=="radiuses") { for(nb::handle h: nb::iter(item.second)) { if(h.is_none()) radiuses.push_back(std::optional<double>()); else radiuses.push_back(nb::cast<double>(h)); } }
            else if(key=="bearings") { for(nb::handle h: nb::iter(item.second)) bearings.push_back(nb::cast<std::optional<osrm::engine::Bearing>>(h)); }
            else if(key=="approaches") { for(nb::handle h: nb::iter(item.second)) approaches.push_back(nb::cast<std::optional<osrm::engine::Approach>>(h)); }
            else {
                throw std::invalid_argument("Unknown TableParameters argument: "+key);
            }
        }

        t->sources = std::move(sources);
        t->destinations = std::move(destinations);
        t->annotations = osrm_nb_util::calculate_tableannotations_type(annotations_vec);
        t->fallback_speed = fallback_speed;
        t->fallback_coordinate_type = fallback_coordinate_type;
        t->scale_factor = scale_factor;

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
        .def_rw("sources", &TableParameters::sources)
        .def_rw("destinations", &TableParameters::destinations)
        .def_rw("fallback_speed", &TableParameters::fallback_speed)
        .def_rw("fallback_coordinate_type", &TableParameters::fallback_coordinate_type)
        .def_rw("annotations", &TableParameters::annotations)
        .def_rw("scale_factor", &TableParameters::scale_factor)
        .def("IsValid", &TableParameters::IsValid);

    nb::enum_<TableParameters::FallbackCoordinateType>(m, "TableFallbackCoordinateType")
        .value("Input", TableParameters::FallbackCoordinateType::Input)
        .value("Snapped", TableParameters::FallbackCoordinateType::Snapped);

    nb::enum_<TableParameters::AnnotationsType>(m, "TableAnnotationsType")
        .value("None", TableParameters::AnnotationsType::None)
        .value("Duration", TableParameters::AnnotationsType::Duration)
        .value("Distance", TableParameters::AnnotationsType::Distance)
        .value("All", TableParameters::AnnotationsType::All);
}
