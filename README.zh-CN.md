# Agent Night Watch

[English](README.md)

Agent Night Watch 是一个 Raycast 菜单栏开关，让本地 Agent 在 MacBook 合盖后
继续运行。它读取 macOS 的 `SleepDisabled` 真实状态，而不是仅使用普通的空闲
防睡眠断言。

- 无水汽咖啡杯：恢复正常合盖睡眠。
- 带水汽咖啡杯：Agent 守夜已开启。
- 每次开启都会弹出 macOS 管理员授权。
- 关闭本扩展创建的会话无需再次授权。
- 不设置定时、低电量自动关闭、免密 sudo、特权助手或密码存储。

它与 Raycast Coffee 等 `caffeinate` 工具不同：后者主要阻止空闲睡眠，本项目
控制的是合盖睡眠的系统级开关。

## 安装 Raycast 扩展

Raycast Store 版本正在准备审核。在商店上架前，可从源码安装：

```sh
git clone https://github.com/Yuchen07569/raycast-agent-night-watch.git
cd raycast-agent-night-watch/raycast-extension
npm ci
npm run dev
```

在 Raycast 中启用 **Night Watch Menu Bar**；如需沿用快捷键，请把
**Toggle Night Watch** 绑定为 `⌥S`。

## 安装 Codex Skill

告诉 Codex：

> 使用 `$skill-installer` 安装
> `https://github.com/Yuchen07569/raycast-agent-night-watch/tree/main/.agents/skills/agent-night-watch`。

Skill 只负责安装、检查、升级和排障，不会代替用户开启或关闭系统睡眠。

## 安全提示

合盖持续运行会增加发热和耗电。请把 MacBook 放在坚硬、通风的表面，任务结束后
主动关闭。不同硬件与 macOS 版本的行为可能不同，因此每次发布前仍需完成一次
真实合盖测试。

## 开发检查

```sh
cd raycast-extension
npm ci
npm test
npm run lint
npm run build
```

修改电源控制流程前，请阅读 [SECURITY.md](SECURITY.md)。
