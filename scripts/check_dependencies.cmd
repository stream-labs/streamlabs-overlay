echo "test text echoed from script"
dumpbin /DEPENDENTS %1 > %2/current_dependents_s1.txt
powershell -command "Get-Content %2/current_dependents_s1.txt | Select-String -CaseSensitive -Pattern dependencies -Context 0,999 > %2/current_dependents_s2.txt"
powershell -command "Get-Content %2/current_dependents_s2.txt | Select-String -CaseSensitive -Pattern Summary -Context 999,0 > %2/current_dependents_s3.txt" 
powershell -command "compare-object (get-content %2/current_dependents_s3.txt) (get-content %3/cmake/approved_dependents.txt)"
powershell -command "if( compare-object (get-content %2/current_dependents_s3.txt) (get-content  %3/cmake/approved_dependents.txt) ) { Write-Error 'Alarm: Module dependencies changed. Please review changes.' }"
