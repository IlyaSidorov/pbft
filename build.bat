cd /d %cd%
cmake -G "Visual Studio 15 2017" %~dp0
cmake --build "." --config "Debug" --clean-first