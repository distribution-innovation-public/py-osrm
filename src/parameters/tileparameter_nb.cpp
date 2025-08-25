#include "parameters/tileparameter_nb.h"

#include "engine/api/tile_parameters.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <stdexcept>

namespace nb = nanobind;
using namespace nb::literals;

void init_TileParameters(nb::module_& m) {
    using osrm::engine::api::TileParameters;

    nb::class_<TileParameters>(m, "TileParameters", nb::is_final())
    .def(nb::init<>(), "Instantiates an instance of TileParameters.\n\n"
                "Examples:\n\
                >>> tile_params = osrm.TileParameters([17059, 11948, 15])\n\
                >>> tile_params = osrm.TileParameters(\n\
                        x = 17059,\n\
                        y = 11948,\n\
                        z = 15\n\
                    )\n\
                >>> tile_params.IsValid()\n\
                True\n\n"
            "Args:\n\
                list (list of int): Instantiates an instance of TileParameters using an array [x, y, z].\n\
                x (int): x value.\n\
                y (int): y value.\n\
                z (int): z value.\n\n"
            "Returns:\n\
                __init__ (osrm.TileParameters): A TileParameters object, for usage in Tile.\n\
                IsValid (bool): A bool value denoting validity of parameter values.\n\n"
            "Attributes:\n\
                x (int): x value.\n\
                y (int): y value.\n\
                z (int): z value."
            )
        .def("__init__", [](TileParameters* t, nb::args args, const nb::kwargs& kwargs){
            unsigned int x = 0, y = 0, z = 0; bool set_any=false;
            // Positional compatibility:
            if(args.size()==1) {
                nb::handle seq = args[0];
                if(nb::isinstance<nb::sequence>(seq)) {
                    std::vector<unsigned int> vals; for(nb::handle h: nb::iter(seq)) vals.push_back(nb::cast<unsigned int>(h));
                    if(vals.size()!=3) throw std::runtime_error("Tile list/tuple must have length 3");
                    x=vals[0]; y=vals[1]; z=vals[2]; set_any=true;
                }
            } else if(args.size()==3) {
                x = nb::cast<unsigned int>(args[0]); y = nb::cast<unsigned int>(args[1]); z = nb::cast<unsigned int>(args[2]); set_any=true;
            } else if(args.size()!=0) {
                throw std::runtime_error("TileParameters expects either (x,y,z) or ([x,y,z]) or keyword args");
            }
            if(kwargs){
                for(auto item: kwargs){
                    std::string key = nb::cast<std::string>(item.first);
                    if(key=="x") { x = nb::cast<unsigned int>(item.second); set_any=true; }
                    else if(key=="y") { y = nb::cast<unsigned int>(item.second); set_any=true; }
                    else if(key=="z") { z = nb::cast<unsigned int>(item.second); set_any=true; }
                    else if(key=="tile" || key=="xyz") {
                        std::vector<unsigned int> vals; for(nb::handle h: nb::iter(item.second)) vals.push_back(nb::cast<unsigned int>(h));
                        if(vals.size()!=3) throw std::runtime_error("Tile list/tuple must have length 3");
                        x=vals[0]; y=vals[1]; z=vals[2]; set_any=true;
                    } else {
                        throw std::invalid_argument("Unknown TileParameters argument: "+key);
                    }
                }
            }
            if(!set_any) throw std::runtime_error("TileParameters requires x,y,z or tile=[x,y,z]");
            new (t) TileParameters{x,y,z};
        })
        .def_rw("x", &TileParameters::x)
        .def_rw("y", &TileParameters::y)
        .def_rw("z", &TileParameters::z)
        .def("IsValid", &TileParameters::IsValid);
    // Removed implicit conversion; kwargs/tile list used instead.
}
