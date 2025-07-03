# Include libfdt
project(
  'targeting',
  'cpp',
  'c',
  version: '0.1',
  meson_version: '>=1.0.1',
  default_options: [
    'warning_level=3',
    'cpp_std=c++23',
  ])

cpp = meson.get_compiler('cpp')

subdir('libfdt')

# Files excluding main.C
targeting_lib_sources = files(
  'targeting/target_service.C',
  'targeting/dtree_loader.C',
  'targeting/common/entitypath.C',
  'targeting/target_devtree_map.C',
  'targeting/predicates/predicatepostfixexpr.C',
)

targeting_inc = include_directories(
  'targeting', 
  'targeting/xmltohb', 
  'targeting/common', 
  'targeting/predicates', 
)

# Build static library (can also use shared_library if needed)
targeting_lib = static_library(
  'targeting',
  targeting_lib_sources,
  include_directories: [libfdt_inc, targeting_inc],
  dependencies: libfdt_dep,
)

# Now define your executable which depends on that lib
executable('targeting-app',
  'main.C',
  include_directories: [libfdt_inc, targeting_inc],
  dependencies: libfdt_dep,
  link_with: targeting_lib,
)

subdir('test')
