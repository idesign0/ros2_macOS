@echo off
REM %~dp0 is the directory in which this bat script is, see
REM https://stackoverflow.com/questions/112055/what-does-d0-mean-in-a-windows-batch-file
REM %* redirects the arguments to the output, see
REM https://serverfault.com/questions/22443/do-windows-batch-files-have-a-construction/22541#22541
ruby %~dp0\gz %*
