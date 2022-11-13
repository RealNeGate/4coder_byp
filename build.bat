@echo off
rem call ..\custom\bin\buildsuper_x64-win.bat ..\4coder_byp\4coder_byp.cpp debug

pushd
cd ..\custom\
call prebuild.bat
popd

call ..\custom\bin\tree_sitter-buildsuper_x64-win.bat ..\4coder_byp\4coder_byp.cpp debug
copy .\custom_4coder.* ..\testbed\custom_4coder.*
popd
pause
