# Compila y corre los tests unitarios del PC. Uso (desde cualquier carpeta):
#   .\tests\run_tests.ps1
# Necesita gcc, cmake y ninja. Si no estan en el PATH, se prueban las rutas de MSYS2 y de ESP-IDF.
$ErrorActionPreference = 'Stop'
$extra = @('C:\msys64\ucrt64\bin', 'C:\Espressif\tools\cmake\4.0.3\bin', 'C:\Espressif\tools\ninja\1.12.1')
$env:PATH = (($extra | Where-Object { Test-Path $_ }) + $env:PATH) -join ';'

$tests = $PSScriptRoot
cmake -S $tests -B "$tests\build" -G Ninja -DCMAKE_C_COMPILER=gcc | Out-Null
cmake --build "$tests\build"
ctest --test-dir "$tests\build" --output-on-failure
exit $LASTEXITCODE
