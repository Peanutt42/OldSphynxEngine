#!/bin/bash

echo "Compiling Engine Shaders..."

mkdir -p Embedded

for f in *.vert; do
	glslangValidator -g -V -x -o "Embedded/$f.embed" "$f"
done

for f in *.frag; do
	glslangValidator -g -V -x -o "Embedded/$f.embed" "$f"
done

echo "Finished compiling Engine Shaders"