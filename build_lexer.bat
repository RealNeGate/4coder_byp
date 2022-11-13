@echo off

call ..\custom\bin\build_one_time.bat .\4coder_byp_odin_lexer_gen.cpp ..\ debug
del ..\4coder_byp_odin_lexer_gen.obj
..\one_time.exe
copy ..\custom\generated\lexer_odin.* .\generated\lexer_odin.*


call ..\custom\bin\build_one_time.bat .\4coder_byp_cpp_lexer_gen.cpp ..\ release
del ..\4coder_byp_cpp_lexer_gen.obj
..\one_time.exe
copy ..\custom\generated\lexer_cpp.* .\generated\lexer_cpp.*

pause