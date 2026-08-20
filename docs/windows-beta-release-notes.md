# Agent Night Watch Windows 0.2.0-beta.1

> Invitation Beta / 邀请测试版

This is the first native Windows tray build of Agent Night Watch. It is an
unsigned x64 portable Beta for a small physical-device test group, not a
generally verified Windows release.

这是 Agent Night Watch 的首个原生 Windows 托盘版本。它是面向少量实机测试者的
x64 未签名便携 Beta，不代表 Windows 合盖能力已经普遍验证。

## What is included / 当前能力

- One native `AgentNightWatch.exe`; no installer, Electron, .NET, Python,
  service, driver, updater, telemetry, or network connection.
- Left-click the tray coffee cup to toggle; right-click for status, recovery,
  exit, and **Lock and Turn Off Displays**.
- On AC power, the app temporarily sets automatic sleep to Never and, when a
  lid is present, the lid-close action to Do Nothing.
- Battery/DC values are never written. Unplugging pauses the power request and
  returns control to the existing battery policy.
- Original AC values are journaled, verified, restored on disable/exit, and
  guarded by a same-executable crash watchdog.

## Before running / 运行前

1. Download the ZIP and `.sha256` file from this release.
2. Verify the checksum. In PowerShell:

   ```powershell
   (Get-FileHash .\AgentNightWatch-0.2.0-beta.1-windows-x64.zip -Algorithm SHA256).Hash.ToLower()
   Get-Content .\AgentNightWatch-0.2.0-beta.1-windows-x64.zip.sha256
   ```

3. The two hashes must match. Do not run the file if they differ.
4. SmartScreen may warn because this invitation Beta is not code-signed. Do not
   disable SmartScreen, Defender, Smart App Control, or company security policy.
5. Keep a laptop on a hard, ventilated surface during lid-close testing.

## Required test / 必测项目

Follow the
[Windows physical-device test guide](../evals/windows-beta-test-guide.zh-CN.md).
The release gate requires two x64 laptops and one desktop, including Modern
Standby and traditional S3 where available. A successful build and automated
test do not replace the ten-minute physical lid-close test.

Please report the Windows version, device model, `powercfg /a` result, heartbeat
gap, original-setting restoration result, CPU/memory use, executable size, and
any SmartScreen or antivirus message. Do not publish private logs or account
information.

## Verified before packaging / 发布前已验证

- MSVC x64 Release compilation with warnings-as-errors and static analysis.
- Power transaction unit tests, macOS regression tests, Skill validation, and
  secret scan.
- Microsoft Defender scan of the generated executable.
- SHA-256 verification and SPDX 2.3 SBOM generation.

Physical Windows tray interaction, lid-close thermals, Modern Standby behavior,
and crash restoration remain invitation-test items.
