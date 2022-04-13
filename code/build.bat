@echo off

set files=..\code\win32_game_of_life.c
set debug_exe_name=debug_win32_game_of_life_x64.exe
set release_exe_name=release_win32_game_of_life_x64.exe

set common_compiler_flags=-nologo -FC -Z7 -EHa- -GR- -MT -Oi -W4 -WX -wd4100
set common_linker_flags=-subsystem:windows,6.00 -opt:ref -incremental:no user32.lib gdi32.lib winmm.lib

set debug_defines=-DDEBUG
set debug_compiler_flags=-Od
set debug_linker_flags=-out:%debug_exe_name%

set release_compiler_flags=-Ot -O2
set release_linker_flags=-out:%release_exe_name%
set release_defines=-DRELEASE

set debug_build_config=%debug_defines% %common_compiler_flags% %debug_compiler_flags% %files% -link %debug_linker_flags% %common_linker_flags%
set release_build_config=%release_defines% %common_compiler_flags% %release_compiler_flags% %files% -link %release_linker_flags% %common_linker_flags%
if not exist ..\build mkdir ..\build

pushd ..\build

echo 	(debug build)
cl %debug_build_config%

rem new line with echo
echo.

echo 	(release build)
cl %release_build_config%

popd
