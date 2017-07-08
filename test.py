#!/usr/bin/env python

import pymruby
import time
import sys

# TODO: move these over to Nose

foo = pymruby.Pymruby()
print foo.eval("n=''; n += 'a' * 10**5; 'hi'")
print foo.eval("__FILE__")
print foo.eval("loop {}")
print foo.eval("while true; end;")
#foo.eval("while 1 do puts 'woah' end")
#foo.eval("def spinner(n); ['|', '\\\\', '-', '/'][n % 4]; end");
#i = 0

#sys.stdout.write("\n")
#while True:
#  sys.stdout.write("\033[1G")
#  sys.stdout.write(foo.eval("spinner (%s) \r" % i))
#  sys.stdout.flush()
#  i= i + 1
#  time.sleep(0.2)

