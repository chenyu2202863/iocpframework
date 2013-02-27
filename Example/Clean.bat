@echo Off
del /s /a *.txt *.exe *.suo *.ncb *.user *.dll *.pdb *.xml *.netmodule *.aps *.ilk *.bak *.sdf *.old2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\x64" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Debug" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Release" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Bin" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Obj" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\.svn" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\ipch" 2>nul

rem If the Properties directory is empty, remove it
FOR /R . %%d in (.) do rd /q "%%d\Properties" 2> nul
