from distutils.core import setup, Extension
from os.path import isfile

try:
    if isfile("MANIFEST"):
        os.unlink("MANIFEST")
except:
    pass

ext_modules = [
    Extension('pymruby', ['src/pymruby.c'],
              extra_compile_args = ['-g', '-fPIC', '-DMRB_ENABLE_DEBUG_HOOK'],
              library_dirs = ['mruby/build/host/lib'],
              include_dirs = ['mruby/include'],
              extra_objects = ['mruby/build/host/lib/libmruby.a'],
              depends = ['src/pymruby.h'])
]

setup(name = 'pymruby',
      version = '0.2',
      description = 'Python binding for mruby',
      author = 'Jason Snell',
      author_email = 'jason@snell.io',
      url = 'http://www.ruby.dj',
      license = 'MIT',
      ext_package = '',
      ext_modules = ext_modules
)
