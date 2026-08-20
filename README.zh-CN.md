# Agent Night Watch

[English](README.md)

Agent Night Watch 让本地 Agent 在电脑息屏后继续运行，并针对不同系统保留尽量小
的入口：

- macOS：Raycast 菜单栏咖啡杯，支持 MacBook 合盖守夜。
- Windows：原生便携托盘咖啡杯，支持接电合盖，以及锁屏并熄灭显示器。

两个版本都没有遥测、账户、广告或后台网络服务。

## macOS Raycast 扩展

- 无水汽咖啡杯：恢复正常合盖睡眠。
- 带水汽咖啡杯：Agent 守夜已开启。
- 正常状态下单击咖啡杯即可直接切换，不再弹出二级菜单。
- 每次开启都会弹出 macOS 管理员授权。
- 关闭本扩展创建的会话无需再次授权。
- 不设置定时、低电量自动关闭、免密 sudo、特权助手或密码存储。

它与 Raycast Coffee 等 `caffeinate` 工具不同：后者主要阻止空闲睡眠，本项目
控制的是合盖睡眠的系统级开关。

### 安装

Raycast Store 版本正在准备审核。在商店上架前，可从源码安装：

```sh
git clone https://github.com/Yuchen07569/raycast-agent-night-watch.git
cd raycast-agent-night-watch/raycast-extension
npm ci
npm run dev
```

第一次运行 **Night Watch Menu Bar** 只会把咖啡杯放进菜单栏；之后单击杯子即可
直接切换。如需沿用快捷键，请把 **Toggle Night Watch** 绑定为 `⌥S`。

若睡眠状态由其他工具占用，或扩展正在切换中，点击咖啡杯会显示安全恢复/状态菜单，
不会直接覆盖异常状态。

## Windows 便携 Beta

Windows 版本目前仅用于邀请测试，且尚未签名。它是单个原生 Win32 程序，不使用
Electron、.NET、Python，不安装服务、驱动、更新器或额外运行时。

- 左键托盘咖啡杯直接开关。
- 有水汽：接电状态下正在守夜。
- 暂停标记：已经开启，但当前使用电池；不会修改任何电池策略。
- 警告标记：电源计划、企业策略或恢复日志需要处理。
- 右键选择“锁屏并熄灭显示器”，账户锁定后后台任务继续运行。

Beta 首发支持 Windows 10 22H2、Windows 11 x64。在完成真实 Windows 笔记本
合盖测试前，不会对外宣称合盖能力已经普遍验证。受邀测试者请按照
[Windows 实机测试指南](evals/windows-beta-test-guide.zh-CN.md)操作；构建说明见
[`windows-tray/README.md`](windows-tray/README.md)。

## 安装 Codex Skill

告诉 Codex：

> 使用 `$skill-installer` 安装
> `https://github.com/Yuchen07569/raycast-agent-night-watch/tree/main/.agents/skills/agent-night-watch`。

Skill 负责两个平台的安装、检查、升级和排障，不会代替用户开启或关闭系统睡眠。

## 安全提示

合盖持续运行会增加发热。请把笔记本放在坚硬、通风的表面，任务结束后主动关闭。
Windows 版只修改接电策略，电池策略始终交给 Windows。每个平台发布前仍需完成
一次真实合盖测试。

## macOS 开发检查

```sh
cd raycast-extension
npm ci
npm test
npm run lint
npm run build
```

修改电源控制流程前，请阅读 [SECURITY.md](SECURITY.md)。

## Windows 开发检查

```powershell
cmake -S windows-tray -B out/windows-x64 -A x64 -DBUILD_TESTING=ON
cmake --build out/windows-x64 --config Release
ctest --test-dir out/windows-x64 -C Release --output-on-failure
```
