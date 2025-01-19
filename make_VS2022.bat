@echo off

pushd .
md build
cd build
cmake -G "Visual Studio 17 2022" ..\ -Dgtest_force_shared_crt=on
popd
pause