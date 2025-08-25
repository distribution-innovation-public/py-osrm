# py-osrm
![PUSH_CI](https://github.com/gis-ops/py-osrm/actions/workflows/push_master.yml/badge.svg)

**py-osrm is a Python package that binds to [osrm-backend](https://github.com/Project-OSRM/osrm-backend) using [nanobind](https://github.com/wjakob/nanobind).**

---

## Supported Platforms
Platform | Arch
---|---
Linux | x86_64
MacOS | x86_64
Windows | x86_64
---

## Installation
py-osrm is supported on **CPython 3.8+**, and can be installed from source via running the following command in the source folder:
```
pip install .
```

## Example
The following example will showcase the process of calculating routes between two coordinates.

First, import the `osrm` library, and instantiate an instance of OSRM:
```python
import osrm

# Instantiate py_osrm instance
py_osrm = osrm.OSRM("./tests/test_data/ch/monaco.osrm")
```

Then, declare `RouteParameters` (all parameter classes now use a single flexible kwargs constructor that accepts strings for enums and coordinate tuples), and then pass it into the `py_osrm` instance:
```python
# Declare Route Parameters
route_params = osrm.RouteParameters(
    coordinates = [(7.41337, 43.72956), (7.41546, 43.73077)]
)

# Pass it into the py_osrm instance
res = py_osrm.Route(route_params)

# Print out result output
print(res["waypoints"])
print(res["routes"])
```
### Unified Parameter Constructors

All parameter types (`RouteParameters`, `MatchParameters`, `TripParameters`, `TableParameters`, `NearestParameters`, `TileParameters`) now share a unified, keyword-only style. Overloaded positional constructors were removed in favor of:

* Strings (case-insensitive) for enum fields, e.g. `geometries="geojson"`, `overview="false"`, `snapping="any"`, `gaps="ignore"`, `source="first"`, `destination="last"`, `fallback_coordinate_type="snapped"`.
* Coordinate tuples `(lon, lat)` in Python lists for all APIs taking coordinates.
* Lists of strings for annotations, e.g. `annotations=["duration","distance"]`.
* Optional values can be set to `None` (e.g. entries in `radiuses`, `bearings`, `approaches`).
* `TileParameters` accepts `TileParameters(x=..., y=..., z=...)`, `TileParameters(17059,11948,15)` or `TileParameters([17059,11948,15])`.

Example of different parameter classes:

```python
match_params = osrm.MatchParameters(
    coordinates=[(7.41,43.73),(7.42,43.74),(7.43,43.75)],
    timestamps=[1424684612,1424684616,1424684620],
    gaps="split",
    annotations=["speed"],
    geometries="polyline6",
)

trip_params = osrm.TripParameters(
    coordinates=[(7.41,43.73),(7.42,43.74)],
    source="first",
    destination="last",
    roundtrip=False,
    annotations=["duration"],
)

table_params = osrm.TableParameters(
    coordinates=[(7.41,43.73),(7.42,43.74),(7.43,43.75)],
    sources=[0],
    destinations=[1,2],
    annotations=["duration","distance"],
    fallback_speed=10.0,
    fallback_coordinate_type="snapped",
)

nearest_params = osrm.NearestParameters(
    coordinates=[(7.41,43.73)],
    number_of_results=3,
)

tile_params = osrm.TileParameters(x=17059,y=11948,z=15)
```

Unknown keyword arguments now raise an immediate `ValueError` / `invalid_argument` from the C++ layer for clarity.

---

## Documentation
[Documentation Page](https://gis-ops.github.io/py-osrm/)
