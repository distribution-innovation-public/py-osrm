cmake_minimum_required(VERSION 3.18)

set(PROJECT_NAME "osrm")
set(EXT_NAME "osrm_ext")
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT SKBUILD)
  message(WARNING
  "This CMake file is meant to be executed using 'scikit-build-core'.
  Running it directly will almost certainly not produce the desired
  result. If you are a user trying to install this package, use the
  command below, which will install all necessary build dependencies,
  compile the package in an isolated environment, and then install it.
  =====================================================================
   $ pip install .
  ====================================================================="
  )
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Prefer lld when using Clang on Linux for faster and more reliable linking (can override with OSRM_DISABLE_LLD=1)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT DEFINED ENV{OSRM_DISABLE_LLD})
  # -fuse-ld=lld lets the driver pick up ld.lld; no need for path. We append to all linker flag sets.
  set(_LLD_FLAG "-fuse-ld=lld")
  add_link_options(${_LLD_FLAG})
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${_LLD_FLAG}" CACHE STRING "" FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${_LLD_FLAG}" CACHE STRING "" FORCE)
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${_LLD_FLAG}" CACHE STRING "" FORCE)
  message(STATUS "Using lld linker (set OSRM_DISABLE_LLD=1 to disable).")
endif()

find_package(Python 3.13
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule
)
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR
)
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
find_package(nanobind CONFIG REQUIRED)

set(SRCS
  src/osrm_nb.cpp
  src/engineconfig_nb.cpp
  src/utility/osrm_utility.cpp
  src/utility/param_utility.cpp

  src/parameters/baseparameter_nb.cpp
  src/parameters/routeparameter_nb.cpp
  src/parameters/matchparameter_nb.cpp
  src/parameters/nearestparameter_nb.cpp
  src/parameters/tableparameter_nb.cpp
  src/parameters/tileparameter_nb.cpp
  src/parameters/tripparameter_nb.cpp
  
  src/types/optional_nb.cpp
  src/types/coordinate_nb.cpp
  src/types/jsoncontainer_nb.cpp
  src/types/approach_nb.cpp
  src/types/bearing_nb.cpp
)
nanobind_add_module(
  ${EXT_NAME}
  STABLE_ABI
  NB_STATIC
  ${SRCS}
  ${UTIL}
  ${PARAMS}
)

target_include_directories(${EXT_NAME} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utility
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parameters
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(LibOSRM)

if(LibOSRM_FOUND)
  # Snapshot original libs so we can safely mutate list variables
  set(_ORIG_LIBOSRM_LIBRARIES "${LibOSRM_LIBRARIES}")
  set(_ORIG_LIBOSRM_DEP_LIBS "${LibOSRM_DEPENDENT_LIBRARIES}")
  # Resolve any Boost::component entries to actual libraries when imported targets are unavailable
  # (CMake 4 removed FindBoost module unless CMP0167 policy handled; Homebrew Boost may lack config files).
  foreach(_comp date_time iostreams random regex system thread)
    if(NOT TARGET Boost::${_comp})
      # Check if the token appears in LibOSRM_LIBRARIES list
      list(FIND LibOSRM_LIBRARIES "Boost::${_comp}" _idx)
      if(_idx GREATER -1)
        # Try to locate the real library
        find_library(BOOST_${_comp}_LIB NAMES boost_${_comp}
          HINTS /opt/homebrew/opt/boost@1.89 /opt/homebrew /usr/local /usr
          PATH_SUFFIXES lib)
        if(BOOST_${_comp}_LIB)
          list(REMOVE_AT LibOSRM_LIBRARIES ${_idx})
          list(APPEND LibOSRM_LIBRARIES ${BOOST_${_comp}_LIB})
          message(STATUS "Resolved Boost::${_comp} -> ${BOOST_${_comp}_LIB}")
        else()
          message(WARNING "Could not resolve Boost::${_comp}; attempting fallback -lboost_${_comp}")
          list(REMOVE_AT LibOSRM_LIBRARIES ${_idx})
          list(APPEND LibOSRM_LIBRARIES -lboost_${_comp})
        endif()
      endif()
    endif()
  endforeach()

  # Augment with pkg-config libs directly (more reliable than parsed variables that may contain Boost:: tokens)
  find_program(PKG_CONFIG_EXECUTABLE pkg-config)
  if(PKG_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --libs libosrm OUTPUT_VARIABLE _PKG_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(_PKG_LIBS)
      separate_arguments(_PKG_LIBS_LIST NATIVE_COMMAND "${_PKG_LIBS}")
      list(APPEND LibOSRM_LIBRARIES ${_PKG_LIBS_LIST})
    endif()
  endif()

  # Deduplicate list
  list(REMOVE_DUPLICATES LibOSRM_LIBRARIES)

  # Ensure critical Boost libs are present; add them if missing.
  # Treat boost_system as optional (header-only in some builds); include others as required
  set(_OPTIONAL_BOOST_LIBS system)
  set(_REQUIRED_BOOST_LIBS thread iostreams date_time regex random filesystem chrono)
  foreach(_rb ${_REQUIRED_BOOST_LIBS} ${_OPTIONAL_BOOST_LIBS})
    set(_found FALSE)
    foreach(_entry ${LibOSRM_LIBRARIES})
      if(_entry MATCHES "boost_${_rb}(\\.|$)" OR _entry STREQUAL "-lboost_${_rb}")
        set(_found TRUE)
      endif()
    endforeach()
    if(NOT _found)
      # Try locate full path
      find_library(BOOST_${_rb}_LIB NAMES boost_${_rb}-mt boost_${_rb} HINTS /opt/homebrew/opt/boost@1.89 /opt/homebrew/opt/boost /opt/homebrew /usr/local /usr PATH_SUFFIXES lib lib64)
      if(BOOST_${_rb}_LIB)
        list(APPEND LibOSRM_LIBRARIES ${BOOST_${_rb}_LIB})
        message(STATUS "Added missing Boost lib: ${BOOST_${_rb}_LIB}")
      else()
        if(NOT _rb IN_LIST _OPTIONAL_BOOST_LIBS)
          list(APPEND LibOSRM_LIBRARIES -lboost_${_rb})
          message(WARNING "Falling back to -lboost_${_rb} (full path not found)")
        else()
          message(STATUS "Skipping boost_${_rb} (header-only / not built)")
        endif()
      endif()
    endif()
  endforeach()

  # Replace any lingering -lboost_* flags with absolute paths if we can now find them (especially boost_system)
  foreach(_rb ${_REQUIRED_BOOST_LIBS} ${_OPTIONAL_BOOST_LIBS})
    foreach(_entry ${LibOSRM_LIBRARIES})
      if(_entry STREQUAL "-lboost_${_rb}")
        find_library(BOOST_${_rb}_LIB_RE NAMES boost_${_rb}-mt boost_${_rb} HINTS /opt/homebrew/opt/boost@1.89 /opt/homebrew/opt/boost /opt/homebrew /usr/local /usr PATH_SUFFIXES lib lib64)
        if(BOOST_${_rb}_LIB_RE)
          list(REMOVE_ITEM LibOSRM_LIBRARIES "-lboost_${_rb}")
          list(APPEND LibOSRM_LIBRARIES ${BOOST_${_rb}_LIB_RE})
          message(STATUS "Resolved fallback -lboost_${_rb} -> ${BOOST_${_rb}_LIB_RE}")
        elseif(_rb IN_LIST _OPTIONAL_BOOST_LIBS)
          # Remove placeholder entirely for optional libs if still unresolved
          list(REMOVE_ITEM LibOSRM_LIBRARIES "-lboost_${_rb}")
          message(STATUS "Removed unresolved optional -lboost_${_rb}")
        endif()
      endif()
    endforeach()
  endforeach()

  # Final de-duplication
  list(REMOVE_DUPLICATES LibOSRM_LIBRARIES)

  # Hardcode Homebrew boost@1.89 absolute library paths if brew prefix is available
  if(APPLE AND NOT DEFINED ENV{OSRM_SKIP_HARDCODE_BOOST})
    find_program(BREW_EXECUTABLE brew)
    if(BREW_EXECUTABLE)
      execute_process(COMMAND ${BREW_EXECUTABLE} --prefix boost@1.89 OUTPUT_VARIABLE _BREW_BOOST_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
      if(_BREW_BOOST_PREFIX AND EXISTS "${_BREW_BOOST_PREFIX}/lib")
        set(_BOOST_LIB_DIR "${_BREW_BOOST_PREFIX}/lib")
        set(_BOOST_LIBS_ABS system thread iostreams date_time regex random filesystem chrono)
        foreach(_bl ${_BOOST_LIBS_ABS})
          set(_cand "${_BOOST_LIB_DIR}/libboost_${_bl}.dylib")
          if(EXISTS "${_cand}")
            # Remove any -lboost_<lib> placeholders
            list(FILTER LibOSRM_LIBRARIES EXCLUDE REGEX "^-lboost_${_bl}$")
            # Append if not already present
            list(FIND LibOSRM_LIBRARIES "${_cand}" _already)
            if(_already EQUAL -1)
              list(APPEND LibOSRM_LIBRARIES "${_cand}")
              message(STATUS "Hardcoded Boost library: ${_cand}")
            endif()
          endif()
        endforeach()
      endif()
    endif()
  endif()
  # Remove any residual Boost:: tokens (unresolved) to prevent CMake error; they will have been replaced or dropped
  list(FILTER LibOSRM_LIBRARIES EXCLUDE REGEX "^Boost::")
  list(FILTER LibOSRM_DEPENDENT_LIBRARIES EXCLUDE REGEX "^Boost::")

  message(STATUS "Using LibOSRM libraries: ${LibOSRM_LIBRARIES}")
  message(STATUS "Using LibOSRM dependent libraries: ${LibOSRM_DEPENDENT_LIBRARIES}")

  target_link_libraries(${EXT_NAME} PRIVATE ${LibOSRM_LIBRARIES} ${LibOSRM_DEPENDENT_LIBRARIES})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LibOSRM_CXXFLAGS}")

  file(GLOB OSRM_EXECS ${LibOSRM_EXEC_DIR}/osrm-*)
  install(FILES ${OSRM_EXECS} TYPE BIN 
          PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ 
          OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)
else()
  include(FetchContent)
  FetchContent_Declare(
    libosrm
    GIT_REPOSITORY https://github.com/Project-OSRM/osrm-backend
    GIT_TAG v6.0.0 #d4dc297f75b574f0236eb8ea08670082d2493719 # 6905 May 27
    GIT_SUBMODULES ""
  )
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-array-bounds -Wno-uninitialized -Wno-free-nonheap-object")

  FetchContent_GetProperties(libosrm)
  if(NOT libosrm_POPULATED)
    FetchContent_Populate(libosrm)

    # Optional: force a clean rebuild of OSRM backend (set OSRM_FORCE_REBUILD=1)
    if(DEFINED ENV{OSRM_FORCE_REBUILD} AND EXISTS ${libosrm_BINARY_DIR})
      message(WARNING "OSRM_FORCE_REBUILD set - removing existing libosrm binary directory: ${libosrm_BINARY_DIR}")
      file(REMOVE_RECURSE ${libosrm_BINARY_DIR})
      file(MAKE_DIRECTORY ${libosrm_BINARY_DIR})
    endif()

    # If a stale libosrm.a from another architecture was cached (e.g. x86_64 reused on aarch64),
    # detect mismatch via 'file' output and remove it so the sub-build will regenerate it.
    function(osrm_check_static_lib_arch LIB EXPECTED_TAG)
      if(EXISTS "${LIB}")
        find_program(OSRM_FILE_CMD file)
        if(OSRM_FILE_CMD)
          execute_process(COMMAND ${OSRM_FILE_CMD} -b ${LIB} OUTPUT_VARIABLE _FILE_OUT OUTPUT_STRIP_TRAILING_WHITESPACE)
          string(TOLOWER "${_FILE_OUT}" _FILE_OUT_L)
          string(TOLOWER "${EXPECTED_TAG}" _EXP_L)
          if(NOT _FILE_OUT_L MATCHES "${_EXP_L}")
            message(WARNING "Architecture mismatch for ${LIB}: expected contains '${EXPECTED_TAG}', file says '${_FILE_OUT}'. Removing to force rebuild.")
            file(REMOVE "${LIB}")
          endif()
        endif()
      endif()
    endfunction()
    # Map common processor names
    set(_OSRM_EXPECTED_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    if(_OSRM_EXPECTED_ARCH MATCHES "arm64")
      set(_OSRM_EXPECTED_ARCH "aarch64")
    endif()
    # Pre-existing lib path (before add_subdirectory it generally won't exist yet, but in incremental builds it may)
    osrm_check_static_lib_arch("${libosrm_BINARY_DIR}/libosrm.a" "${_OSRM_EXPECTED_ARCH}")

    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
    add_subdirectory(
      ${libosrm_SOURCE_DIR}
      ${libosrm_BINARY_DIR}
    )
  endif()

  target_include_directories(${EXT_NAME} 
    PUBLIC
      ${libosrm_SOURCE_DIR}/include
      ${libosrm_SOURCE_DIR}/third_party/variant/include
      ${libosrm_SOURCE_DIR}/third_party/flatbuffers/include
  )
  target_link_libraries(${EXT_NAME}
    PRIVATE
      osrm
  )
endif()

if (MSVC)
  add_definitions(-DBOOST_ALL_NO_LIB)
endif()

install(TARGETS ${EXT_NAME} LIBRARY DESTINATION ${PROJECT_NAME})
