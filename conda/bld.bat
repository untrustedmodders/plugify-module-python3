@echo off
REM bld.bat - For Windows builds

REM Create the target directories
if not exist "%PREFIX%\bin" mkdir "%PREFIX%\bin"
if not exist "%PREFIX%\lib" mkdir "%PREFIX%\lib"
if not exist "%PREFIX%\python3.12" mkdir "%PREFIX%\python3.12"
if not exist "%PREFIX%" mkdir "%PREFIX%"

REM Copy the DLL and module file
copy bin\plugify-module-dotnet.dll "%PREFIX%\bin\" || exit 1
xcopy lib "%PREFIX%\lib" /E /Y /I
xcopy python3.12 "%PREFIX%\python3.12" /E /Y /I
copy plugify-module-dotnet.pmodule "%PREFIX%\" || exit 1

REM Create activation scripts
if not exist "%PREFIX%\etc\conda\activate.d" mkdir "%PREFIX%\etc\conda\activate.d"
if not exist "%PREFIX%\etc\conda\deactivate.d" mkdir "%PREFIX%\etc\conda\deactivate.d"

REM Create activation script
echo @echo off > "%PREFIX%\etc\conda\activate.d\plugify-module-python3.bat"
echo set "PLUGIFY_PY3_MODULE_PATH=%%CONDA_PREFIX%%;%%PLUGIFY_PY3_MODULE_PATH%%" >> "%PREFIX%\etc\conda\activate.d\plugify-module-python3.bat"

REM Create deactivation script  
echo @echo off > "%PREFIX%\etc\conda\deactivate.d\plugify-module-python3.bat"
echo set "PLUGIFY_PY3_MODULE_PATH=%%PLUGIFY_PY3_MODULE_PATH:%%CONDA_PREFIX%%;=%%" >> "%PREFIX%\etc\conda\deactivate.d\plugify-module-python3.bat"

exit 0
