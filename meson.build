project('parson', 'c',
    version : '1.3.1',
    license : 'MIT',
    meson_version : '>=0.54.0',
    default_options : [
        'c_std=c89', 'optimization=2', 
        'warning_level=2'
        ]
)

lib_so_version = '0'

parson_sources = ['parson.c']

parson_inc = include_directories('.')

# defaults to a share library. pass -Ddefault_library=static for a static build
parson_lib = library(
    meson.project_name(), # library's name. becomes libparson
    parson_sources,
    version: meson.project_version(),
    soversion: lib_so_version,
    install: not meson.is_subproject())

parson = declare_dependency(
    link_with : parson_lib,
    include_directories : parson_inc
)

install_headers('parson.h', subdir : 'parson')


meson.override_dependency('parson', parson)

pkgconfig = import('pkgconfig')
pkgconfig.generate(parson_lib,
    version: meson.project_version(),
    filebase: meson.project_name(),
    name: meson.project_name(),
    description: 'Lightweight JSON library written in C.',
)