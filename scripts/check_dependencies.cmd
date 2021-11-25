Set BinaryFilename=%1
For %%A in ("%BinaryFilename%") do (
    Set BinaryFolder=%%~dpA
    Set BinaryName=%%~nxA
)
Set depsFile=%3/cmake/%BinaryName%.txt
Set depsCurrent=%2/%BinaryName%

dumpbin /DEPENDENTS %BinaryFilename% > %depsCurrent%_1.txt
powershell -command "Get-Content %depsCurrent%_1.txt | Select-String -CaseSensitive -Pattern dependencies -Context 0,999 > %depsCurrent%_2.txt"
powershell -command "Get-Content %depsCurrent%_2.txt | Select-String -CaseSensitive -Pattern Summary -Context 999,0 > %depsCurrent%_3.txt" 
powershell -command "compare-object (get-content %depsCurrent%_3.txt) (get-content %depsFile%)"
powershell -command "if( compare-object (get-content %depsCurrent%_3.txt) (get-content  %depsFile%) ) { Write-Error 'Alarm: Module dependencies changed. Please review changes.' }"
del %depsCurrent%_1.txt %depsCurrent%_2.txt