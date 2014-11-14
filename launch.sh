#!/bin/sh

# This script is expecting a gray-on-black terminal
# Terminals other than urxvt should be usable, as long as they’re
# VT100-compatible and support 256 colors codes.

size=33

fonts=xft:UbuntuMono-$size,xft:Terminus-$size

urxvt \
	-fn $fonts \
	-fb $fonts \
	-shading 0 \
	-color0 "#444444444444" \
	-e ./rsq

