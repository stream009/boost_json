project('boost_json', 'cpp',
  default_options: ['cpp_std=c++2a', 'warning_level=3'], license: 'GPL3' )

add_project_arguments(['-Wconversion', '-Wsign-conversion'], language: 'cpp')

# external dependencies for external ones and internal ones
# If dependency appear in public header, add it to the public_deps othewise
# private_deps will suffice.
public_deps = []
private_deps = []

# include directories, external ones and internal ones
public_inc = include_directories('include', is_system: true)
private_inc = []

# library targets which user have to link with
project_libs = []

#subdir('src')

# dependency for client to access with get_variable('project_dep')
project_dep = declare_dependency(
  include_directories: public_inc,
  dependencies: public_deps,
  link_with: project_libs,
)

# allow client to do pull in library as dependency('PROJECT NAME')
meson.override_dependency(meson.project_name(), project_dep)
