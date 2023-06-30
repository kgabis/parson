project('parson', 'c',
    version : '1.5.2',
    license : 'MIT',
    meson_version : '>=0.46.0',
    default_options : [
        'c_std=c89', 'optimization=2',
        'warning_level=2'
        ]
)

parson_sources = ['parson.c']

parson_inc = include_directories('.')

parson_lib = library(
    meson.project_name(),
    sources: parson_sources,
    install: true
)

install_headers('parson.h')

parson = declare_dependency(
    include_directories : parson_inc,
    link_with : parson_lib
)

pkgconfig = import('pkgconfig')

# will create a pkg config
pkgconfig.generate(parson_lib,
    version: meson.project_version(),
    filebase: meson.project_name(),
    name: meson.project_name(),
    description: 'Lightweight JSON library written in C.',
)
