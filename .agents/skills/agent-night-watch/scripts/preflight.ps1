$ErrorActionPreference = "Stop"

if (-not $IsWindows) {
  [ordered]@{
    supported = $false
    platform = [System.Environment]::OSVersion.Platform.ToString()
    reason = "Agent Night Watch Windows preflight requires Windows."
  } | ConvertTo-Json -Compress
  exit 0
}

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

public static class AgentNightWatchPowerRead {
  [StructLayout(LayoutKind.Sequential)]
  public struct SystemPowerStatus {
    public byte ACLineStatus;
    public byte BatteryFlag;
    public byte BatteryLifePercent;
    public byte SystemStatusFlag;
    public uint BatteryLifeTime;
    public uint BatteryFullLifeTime;
  }

  [DllImport("powrprof.dll")]
  public static extern uint PowerGetActiveScheme(IntPtr rootPowerKey, out IntPtr activePolicyGuid);

  [DllImport("powrprof.dll")]
  public static extern uint PowerReadACValueIndex(
    IntPtr rootPowerKey,
    ref Guid schemeGuid,
    ref Guid subgroupGuid,
    ref Guid settingGuid,
    out uint valueIndex
  );

  [DllImport("kernel32.dll")]
  public static extern IntPtr LocalFree(IntPtr memory);

  [DllImport("kernel32.dll", SetLastError = true)]
  [return: MarshalAs(UnmanagedType.Bool)]
  public static extern bool GetSystemPowerStatus(out SystemPowerStatus status);
}
"@

$activePointer = [IntPtr]::Zero
$activeScheme = $null
$standbyAc = $null
$lidAc = $null
$powerSource = "unknown"
$powerError = $null

try {
  $result = [AgentNightWatchPowerRead]::PowerGetActiveScheme([IntPtr]::Zero, [ref]$activePointer)
  if ($result -ne 0 -or $activePointer -eq [IntPtr]::Zero) {
    throw "PowerGetActiveScheme failed with code $result"
  }
  $schemeGuid = [Runtime.InteropServices.Marshal]::PtrToStructure($activePointer, [type][Guid])
  $activeScheme = $schemeGuid.ToString("B")

  $sleepSubgroup = [Guid]"238c9fa8-0aad-41ed-83f4-97be242c8f20"
  $standbySetting = [Guid]"29f6c1db-86da-48c5-9fdb-f2b67b1f44da"
  [uint32]$standbyValue = 0
  if ([AgentNightWatchPowerRead]::PowerReadACValueIndex(
      [IntPtr]::Zero, [ref]$schemeGuid, [ref]$sleepSubgroup,
      [ref]$standbySetting, [ref]$standbyValue) -eq 0) {
    $standbyAc = $standbyValue
  }

  $buttonSubgroup = [Guid]"4f971e89-eebd-4455-a8de-9e59040e7347"
  $lidSetting = [Guid]"5ca83367-6e45-459f-a27b-476b1d01c936"
  [uint32]$lidValue = 0
  if ([AgentNightWatchPowerRead]::PowerReadACValueIndex(
      [IntPtr]::Zero, [ref]$schemeGuid, [ref]$buttonSubgroup,
      [ref]$lidSetting, [ref]$lidValue) -eq 0) {
    $lidAc = $lidValue
  }

  $status = New-Object AgentNightWatchPowerRead+SystemPowerStatus
  if ([AgentNightWatchPowerRead]::GetSystemPowerStatus([ref]$status)) {
    if ($status.ACLineStatus -eq 1) { $powerSource = "ac" }
    elseif ($status.ACLineStatus -eq 0) { $powerSource = "battery" }
  }
} catch {
  $powerError = $_.Exception.Message
} finally {
  if ($activePointer -ne [IntPtr]::Zero) {
    [void][AgentNightWatchPowerRead]::LocalFree($activePointer)
  }
}

$journalPath = Join-Path $env:LOCALAPPDATA "AgentNightWatch\session-v1.journal"
$processRunning = @(Get-Process -Name "AgentNightWatch" -ErrorAction SilentlyContinue).Count -gt 0

[ordered]@{
  supported = $true
  platform = "Windows"
  architecture = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString()
  processRunning = $processRunning
  powerSource = $powerSource
  activeScheme = $activeScheme
  standbyAcSeconds = $standbyAc
  lidAcAction = $lidAc
  sessionJournal = if (Test-Path -LiteralPath $journalPath -PathType Leaf) { "present" } else { "missing" }
  powerReadError = $powerError
} | ConvertTo-Json -Compress
