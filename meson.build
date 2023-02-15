project('parson', 'c',
    version : '1.5.1',
    license : 'MIT',
    meson_version : '>=0.46.0',
    default_options : [
        'c_std=c89', 'optimization=2', 
        'warning_level=2'
        ]
)

parson_sources = ['parson.c']

parson_inc = include_directories('.')

lib_so_version = '0'

parson_shared_lib = shared_library(
    meson.project_name(),
    sources: parson_sources,
    soversion: lib_so_version,
    install: true
)

parson_static_lib = static_library(
    meson.project_name(),
    sources: parson_sources,
    install: true
)

install_headers('parson.h', subdir : 'parson')

parson_shared_dep = declare_dependency(
    include_directories : parson_inc,
    link_with : parson_shared_lib
)

parson_static_dep = declare_dependency(
    include_directories : parson_inc,
    link_with : parson_static_lib
)

pkgconfig = import('pkgconfig')

# will create a pkg config for the shared lib only
pkgconfig.generate(parson_shared_lib,
    version: meson.project_version(),
    filebase: meson.project_name(),
    name: meson.project_name(),
    description: 'Lightweight JSON library written in C.',
)