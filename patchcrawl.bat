patch -p 0 < dc400b26j070.pat
pause
copy m* src_o
copy ouch.* src_o
copy acr.cc src_o
copy libwt.cc src_o
pause
cd src_o
make all
copy *.exe ..
REM pause
make clean
copy makefile_console makefile
make all
copy *.exe ..
pause
