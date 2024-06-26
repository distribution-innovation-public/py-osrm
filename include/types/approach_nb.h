#ifndef OSRM_NB_APPROACH_H
#define OSRM_NB_APPROACH_H

#include "engine/approach.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <nanobind/stl/unordered_map.h>

using osrm::engine::Approach;

void init_Approach(nanobind::module_& m);

static const std::unordered_map<std::string, Approach> approach_map {
    { "curb", Approach::CURB },
    { "opposite", Approach::OPPOSITE },
    { "unrestricted", Approach::UNRESTRICTED }
};

#endif //OSRM_NB_APPROACH_H
