#!/bin/bash
command="../build/Mono/lib/mono/4.5/mcs.exe -nowarn:\"CS8029\" -codepage:65001 --stacktrace -debug- -L \"../build/FalcoEngine.dll\" -r:\"../build/FalcoEngine.dll\" -target:library -o \"../build/FalcoEngine.dll\" -recurse:\"./*.cs\""
eval "$command"
