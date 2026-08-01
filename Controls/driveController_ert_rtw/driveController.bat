
cd .

if "%1"=="" ("Z:\.Programmi\MATLAB26a\bin\win64\gmake"  -f driveController.mk all) else ("Z:\.Programmi\MATLAB26a\bin\win64\gmake"  -f driveController.mk %1)
@if errorlevel 1 goto error_exit

exit /B 0

:error_exit
echo The make command returned an error of %errorlevel%
exit /B 1