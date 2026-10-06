param([string]$JLinkPath = 'C:\Program Files\SEGGER\JLink\JLink.exe', [int]$Serial = 0)
$ErrorActionPreference = 'Stop'
$firmware = Join-Path (Split-Path $PSScriptRoot -Parent) 'build\stm32_sensor_display.hex'
if (!(Test-Path -LiteralPath $firmware)) { throw 'Run tools/build.ps1 first.' }
if (!(Test-Path -LiteralPath $JLinkPath)) { throw 'Install the official SEGGER J-Link Software and Documentation Pack.' }
$script = Join-Path ([IO.Path]::GetTempPath()) ('stm32-sensor-' + [guid]::NewGuid() + '.jlink')
try {
  @('r', 'h', ('loadfile "' + $firmware + '"'), 'r', 'g', 'exit') | Set-Content -LiteralPath $script -Encoding ascii
  $arguments = @('-NoGui','1','-ExitOnError','1','-AutoConnect','1','-Device','STM32C562CE','-If','SWD','-Speed','1000','-CommandFile',$script)
  if ($Serial -ne 0) { $arguments += @('-USB', "$Serial") }
  & $JLinkPath @arguments
  if ($LASTEXITCODE -ne 0) { throw "J-Link programming failed ($LASTEXITCODE)." }
} finally { Remove-Item -LiteralPath $script -ErrorAction SilentlyContinue }
