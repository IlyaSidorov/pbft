#!/bin/bash
cmake "${0%/*}"
cmake --build "." --config "Debug" --clean-first
