param([string]$ToolRoot = "$env:LOCALAPPDATA\Programs\STM32SensorTools")
$ErrorActionPreference = 'Stop'
$compiler = Get-ChildItem -LiteralPath $ToolRoot -Recurse -Filter arm-none-eabi-gcc.exe | Select-Object -First 1
$make = Get-ChildItem -LiteralPath $ToolRoot -Recurse -Filter make.exe | Select-Object -First 1
if (!$compiler -or !$make) { throw "Install Arm GNU Toolchain and GNU Make under $ToolRoot, or pass -ToolRoot." }
$oldPath = $env:Path
try {
  $env:Path = "$($compiler.DirectoryName);$($make.DirectoryName);$oldPath"
  & $make.FullName -C (Split-Path $PSScriptRoot -Parent) -j4 'SHELL=sh'
  if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)." }
} finally { $env:Path = $oldPath }
