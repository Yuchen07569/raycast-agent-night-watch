# Windows 邀请测试指南

## 测试前

1. 仅从维护者提供的 GitHub Actions 或预发布页面下载 ZIP 与 `.sha256`。
2. 核对 SHA-256；不要关闭 SmartScreen、Defender 或公司安全策略。
3. 记录 Windows 版本、电脑型号，并在管理员终端运行 `powercfg /a`，保存
   “此系统可用的睡眠状态”截图或文字。
4. 在 Windows 高级电源设置中记录当前接电/电池的睡眠时间与合盖动作。
5. 把笔记本放在坚硬、通风表面，合盖测试期间不要放进包里。

## 心跳记录

在普通 PowerShell 窗口运行：

```powershell
$path = "$env:USERPROFILE\Desktop\agent-night-watch-windows-heartbeat-$(Get-Date -Format yyyyMMdd-HHmmss).log"
while ($true) {
  "$(Get-Date -Format o) pid=$PID" | Add-Content -LiteralPath $path
  Start-Sleep -Seconds 5
}
```

结束时按 `Ctrl+C`。测试日志只包含时间和进程号。

## 笔记本接电合盖

1. 启动程序，确认托盘无水汽咖啡杯。
2. 左键开启，确认有水汽；重新检查接电睡眠为“从不”、接电合盖为“不采取
   任何操作”，电池两项保持原值。
3. 启动心跳，合盖 10 分钟后打开。
4. 日志最大间隔应接近 5 秒，不应出现覆盖 10 分钟的空档。
5. 左键关闭，确认接电睡眠和合盖动作准确恢复原值。

## 拔电对照

1. 接电开启后确认有水汽，再关闭上盖。
2. 拔掉电源，等待超过原电池睡眠时间后重新接电并开盖。
3. 电脑应采用原电池策略进入睡眠；程序不得把电池策略改为“从不”。

## 台式机锁屏熄屏

1. 左键开启并启动心跳。
2. 右键选择 **Lock and Turn Off Displays**，等待 10 分钟。
3. 唤醒显示器，确认必须通过密码或 Windows Hello 返回。
4. 日志不应出现覆盖这 10 分钟的空档；关闭后原接电睡眠时间必须恢复。

## 崩溃恢复

开启后在任务管理器结束托盘主进程。等待数秒，再检查原接电电源设置已经恢复。
若出现红色警告咖啡杯，不要删除恢复日志；打开 **View Status** 并把完整错误反馈给
维护者。

## 反馈内容

- 电脑型号、Windows 版本、`powercfg /a` 结果。
- 笔记本或台式机，是否使用扩展坞/外接显示器。
- 三组测试是否通过、日志最大时间间隔。
- `AgentNightWatch.exe` 文件大小、任务管理器内存与 CPU。
- Defender、SmartScreen 或其他杀毒软件是否提示。
- 开关步骤是否比直接修改 Windows 电源设置更方便。
