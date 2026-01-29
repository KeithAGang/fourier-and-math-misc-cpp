link_args = []
if host_machine.system() == 'windows'
  link_args += '-static-libstdc++'
endif

exe = executable('fourier', 'main.cpp',
  link_args: link_args,
dependencies: [ imgui_dep, sdl2_dep, internal_deps],
  install : true)