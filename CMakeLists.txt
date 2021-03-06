# @@@LICENSE
#
#      Copyright (c) 2008-2013 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

cmake_minimum_required(VERSION 2.8.7)
project(luna-service2 C)

include(webOS/webOS)
include(CTest)

webos_modules_init(1 0 0 QUALIFIER RC4)
webos_component(3 5 1)

include(FindPkgConfig)

include_directories(src/libluna-service2)
include_directories(include/public)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(GTHREAD2 REQUIRED gthread-2.0)
include_directories(${GTHREAD2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GTHREAD2_CFLAGS_OTHER})

# check if we have cjson
pkg_check_modules(CJSON REQUIRED cjson)
include_directories(${CJSON_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${CJSON_CFLAGS_OTHER})

# check if we have PmLogLib
pkg_check_modules(PMLOGLIB REQUIRED PmLogLib)
include_directories(${PMLOGLIB_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOGLIB_CFLAGS_OTHER})

webos_machine_impl_dep()
include(webOS/LegacyDefines)

set(LS2_PERMS "")
set(LS2_RESTRICTED "")

if(WEBOS_TARGET_MACHINE_IMPL STREQUAL hardware)
	# Only root (owner) will be permitted access to certain binaries on device
	set(LS2_PERMS PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

	# XXX Convert this to VERY_RESTRICTED_PERMISSIONS
	# once it is implemented.
	set(LS2_RESTRICTED RESTRICTED_PERMISSIONS)
endif()

# Enable asserts in development release builds
if(NOT ${WEBOS_DISTRO_PRERELEASE} STREQUAL "")
	webos_add_compiler_flags(RELEASE -UNDEBUG -DWEBOS_DISTRO_PRERELEASE_ASSERTS)
endif()

# TODO: Is there a better way than setting all the flags (i.e., built in support)?
if(USE_GCOV)
	message("Building with gcov support")
	webos_add_compiler_flags(ALL -fprofile-arcs -ftest-coverage)
	webos_add_linker_options(ALL -lgcov -fprofile-arcs)
endif()

# Enable security by default
IF(NOT DEFINED WEBOS_LS2_SECURE)
	SET(WEBOS_LS2_SECURE True)
ENDIF(NOT DEFINED WEBOS_LS2_SECURE)

# Debug definitions
webos_add_compiler_flags(ALL -DMEMCHECK -DLSHANDLE_CHECK)


# Specifying this causes "inline function '<function>' declared but never defined" errors.
# webos_add_compiler_flags(ALL -std=c99)

# Turn on all warnings except deprecated declarations and make them into errors
webos_add_compiler_flags(ALL -Wall -Wno-deprecated-declarations)


# Specify which symbols are to be exported
# TODO: Create this
#webos_add_linker_options(ALL --version-script=${PROJECT_SOURCE_DIR}/src/libluna-service2/luna-service2Exports.map )

# Require that all undefined symbols are satisfied by the libraries from target_link_libraries()
webos_add_linker_options(ALL --no-undefined)

# XXX_ The libraries that come with gcc fail when --no-allow-shlib-undefined is specified
#webos_add_linker_options(ALL --no-allow-shlib-undefined)

add_subdirectory(src/libluna-service2)
add_subdirectory(src/ls-hubd)
add_subdirectory(src/ls-monitor)
add_subdirectory(src/luna-send)
add_subdirectory(src/luna-helper)
add_subdirectory(files/conf)

webos_build_pkgconfig()
webos_config_build_doxygen(doc Doxyfile)
webos_build_system_bus_files()

# install the script file
webos_configure_source_files(LS-CONTROL files/scripts/public/ls-control)
install(PROGRAMS ${LS-CONTROL} DESTINATION ${WEBOS_INSTALL_SBINDIR} ${LS2_PERMS})

# Create the permanent service directories. The ones for downloaded services (called
# "dynamic" but not to be confused with services that are started on-demand, which
# are known as "dynamic services") are created at boot time by the ls-hubd-private
# launch script.
#
# The permanent roles directories are created as part of the installation of roles
# files for ls-monitor and luna-send.
install(DIRECTORY DESTINATION ${WEBOS_INSTALL_SYSBUS_PRVSERVICESDIR})
install(DIRECTORY DESTINATION ${WEBOS_INSTALL_SYSBUS_PUBSERVICESDIR})

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
