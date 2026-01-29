project('fourier', 'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++23', 'warning_level=3'])

# Import dependancies
sdl2_dep = dependency('sdl2', fallback:['sdl2', 'sdl2_dep'])
imgui_dep = dependency('imgui', fallback:['imgui', 'imgui_dep'])




subdir('internals')
subdir('src')

test('basic', exe)
