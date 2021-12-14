rem ##################
rem Script to check changes in dependencies
rem It compares dependencies of module after a build to saved list

rem ##################
rem Prepare file names
Set BinaryFilename=%1
For %%A in ("%BinaryFilename%") do (
  Set BinaryName=%%~nxA
  Set depsFile=%3/scripts/%%~nxA.txt
  Set depsCurrent=%2/%%~nxA
)

rem ##################
rem Get dependencies from current binary
dumpbin /DEPENDENTS %BinaryFilename% > %depsCurrent%_1.txt

rem ##################
rem Clean up parts of a dependency report what could change
powershell -command "Get-Content %depsCurrent%_1.txt | Select-String -CaseSensitive -Pattern dependencies -Context 0,999 > %depsCurrent%_2.txt"
powershell -command "Get-Content %depsCurrent%_2.txt | Select-String -CaseSensitive -Pattern Summary -Context 999,0 > %depsCurrent%.txt"
powershell -command "Remove-Item %depsCurrent%_1.txt"
powershell -command "Remove-Item %depsCurrent%_2.txt"

IF EXIST %depsFile% (
  rem ##################
  rem Compare current dependency list with saved
  rem Write-Error should stop cmake with an error
  powershell -command "compare-object (get-content %depsCurrent%.txt) (get-content %depsFile%)"
  powershell -command "if( compare-object (get-content %depsCurrent%.txt) (get-content  %depsFile%) ) { Write-Error 'Alarm: Module dependencies changed. Please review changes.' }"
) ELSE (
  rem ##################
  echo ## ## ## ## ## ##
  echo File with depndencies was not found
  echo Check current dependencies in %depsCurrent%.txt and save it as %depsFile%
  echo ## ## ## ## ## ##
  powershell -command "Write-Error 'Alarm: Module dependencies was not initialized.'"
)


