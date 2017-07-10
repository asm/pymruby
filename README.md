# pymruby
This is a quick and dirty Python module that embeds mruby.  It has basic constraints around mruby's `eval()` call including a memory and execution time watchdog.

## Building
Building this module requires some [slight tweaks](https://github.com/asm/mruby/commit/10cd3499b0a4e871c466df8de7485faad0e1aaa1) to mruby.  After cloning the repository, pull in the submodule by running

    git submodule update --init

Next, compile mruby

    cd mruby && ./minirake

Now you're ready to build and install the module

    python setup.py build
    python setup.py install

If all goes well, you should be able to run a few minimal tests

    ./test.py
