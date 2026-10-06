param([string]$Compiler = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
if (!$Compiler) {
  $native = Get-Command tcc,gcc,clang -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($native) { $Compiler = $native.Source }
  else {
    $native = Get-ChildItem -LiteralPath "$env:LOCALAPPDATA\Programs\STM32SensorTools\native-tests" -Recurse -Filter tcc.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($native) { $Compiler = $native.FullName }
  }
}
if (!$Compiler) { throw 'Pass -Compiler with a native C compiler (TCC, GCC or Clang).' }
$build = Join-Path $root 'build'
New-Item -ItemType Directory -Path $build -Force | Out-Null
$binary = Join-Path $build 'test_product.exe'
& $Compiler -Wall -Werror "-I$root/App/Inc" "-I$root/BSP/Inc" "$root/tests/test_product.c" "$root/App/Src/sensor_model.c" "$root/App/Src/product_display.c" -o $binary
if ($LASTEXITCODE -ne 0) { throw 'Native test build failed.' }
& $binary
if ($LASTEXITCODE -ne 0) { throw 'Native tests failed.' }
