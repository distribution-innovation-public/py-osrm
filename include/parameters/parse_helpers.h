#pragma once

#include <string>
#include <algorithm>
#include "engine/api/route_parameters.hpp"
#include "engine/api/base_parameters.hpp"
#include "engine/api/match_parameters.hpp"
#include "engine/api/trip_parameters.hpp"
#include "engine/api/table_parameters.hpp"

namespace osrm_nb_parse {

inline std::string to_lower(std::string s){ std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }

// Route / common enums
inline osrm::engine::api::RouteParameters::GeometriesType parse_geometries(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="polyline") return osrm::engine::api::RouteParameters::GeometriesType::Polyline;
    if(ls=="polyline6") return osrm::engine::api::RouteParameters::GeometriesType::Polyline6;
    if(ls=="geojson") return osrm::engine::api::RouteParameters::GeometriesType::GeoJSON;
    throw std::runtime_error("Invalid geometries string: "+s);
}
inline osrm::engine::api::RouteParameters::OverviewType parse_overview(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="simplified"||ls.empty()) return osrm::engine::api::RouteParameters::OverviewType::Simplified;
    if(ls=="full") return osrm::engine::api::RouteParameters::OverviewType::Full;
    if(ls=="false") return osrm::engine::api::RouteParameters::OverviewType::False;
    throw std::runtime_error("Invalid overview string: "+s);
}
inline osrm::engine::api::RouteParameters::AnnotationsType parse_route_annotation(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="none") return osrm::engine::api::RouteParameters::AnnotationsType::None;
    if(ls=="duration") return osrm::engine::api::RouteParameters::AnnotationsType::Duration;
    if(ls=="nodes") return osrm::engine::api::RouteParameters::AnnotationsType::Nodes;
    if(ls=="distance") return osrm::engine::api::RouteParameters::AnnotationsType::Distance;
    if(ls=="weight") return osrm::engine::api::RouteParameters::AnnotationsType::Weight;
    if(ls=="datasources") return osrm::engine::api::RouteParameters::AnnotationsType::Datasources;
    if(ls=="speed") return osrm::engine::api::RouteParameters::AnnotationsType::Speed;
    if(ls=="all") return osrm::engine::api::RouteParameters::AnnotationsType::All;
    throw std::runtime_error("Invalid annotation string: "+s);
}
inline osrm::engine::api::BaseParameters::SnappingType parse_snapping(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="default"||ls.empty()) return osrm::engine::api::BaseParameters::SnappingType::Default;
    if(ls=="any") return osrm::engine::api::BaseParameters::SnappingType::Any;
    throw std::runtime_error("Invalid snapping string: "+s);
}
// Match
inline osrm::engine::api::MatchParameters::GapsType parse_gaps(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="split"||ls.empty()) return osrm::engine::api::MatchParameters::GapsType::Split;
    if(ls=="ignore") return osrm::engine::api::MatchParameters::GapsType::Ignore;
    throw std::runtime_error("Invalid gaps string: "+s);
}
// Trip
inline osrm::engine::api::TripParameters::SourceType parse_trip_source(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="any"||ls.empty()) return osrm::engine::api::TripParameters::SourceType::Any;
    if(ls=="first") return osrm::engine::api::TripParameters::SourceType::First;
    throw std::runtime_error("Invalid source string: "+s);
}
inline osrm::engine::api::TripParameters::DestinationType parse_trip_destination(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="any"||ls.empty()) return osrm::engine::api::TripParameters::DestinationType::Any;
    if(ls=="last") return osrm::engine::api::TripParameters::DestinationType::Last;
    throw std::runtime_error("Invalid destination string: "+s);
}
// Table
inline osrm::engine::api::TableParameters::AnnotationsType parse_table_annotation(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="none") return osrm::engine::api::TableParameters::AnnotationsType::None;
    if(ls=="duration") return osrm::engine::api::TableParameters::AnnotationsType::Duration;
    if(ls=="distance") return osrm::engine::api::TableParameters::AnnotationsType::Distance;
    if(ls=="all") return osrm::engine::api::TableParameters::AnnotationsType::All;
    throw std::runtime_error("Invalid table annotation string: "+s);
}
inline osrm::engine::api::TableParameters::FallbackCoordinateType parse_fallback_coordinate_type(const std::string& s){
    auto ls = to_lower(s);
    if(ls=="input"||ls.empty()) return osrm::engine::api::TableParameters::FallbackCoordinateType::Input;
    if(ls=="snapped") return osrm::engine::api::TableParameters::FallbackCoordinateType::Snapped;
    throw std::runtime_error("Invalid fallback_coordinate_type string: "+s);
}

}
