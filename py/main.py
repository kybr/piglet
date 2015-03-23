#!/usr/bin/pypy

import piglet

def setup():
  print("setup")

def draw():
  print("draw")
  # need opengl now...

p = piglet.Create(setup, draw)
raw_input("Press Enter to continue...")
piglet.Destroy(p)
