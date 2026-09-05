# Lunar Client 1.8.9 本地饰品解锁

开发者：ColdEternity Team

当前版本：`v1.1.0`

## v1.1.0 更新

- 修复选择状态重复保存造成的游戏卡顿：后台仅在完整状态实际变化时写入配置，并降低周期性 JVM 检查开销。
- 修复部分用户无法注入的问题：运行时成员解析增加按描述符和字段泛型的回退，目标进程检测与组件状态验证更加稳健。

原始仓库：<https://github.com/LongCold-ColdEternity-Team/Lunar-Local-Cosmetics-Unlocker>

作者 QQ：`3442312505`

如果这个项目对你有帮助，可以点一个 Star，谢谢！

该工具只修改当前 Lunar Client 1.8.9 Java 进程中的本地 catalog/owned 状态，不修改账号或服务端拥有状态，其他玩家看不到这些变化。

## 已覆盖分类

当前 Lunar Client `v2.22.33-2636` 实测：

| 分类 | 本地目录数量 |
| --- | ---: |
| Cosmetics | 5512 |
| Emotes | 182 |
| Jams | 97 |
| Sprays | 5 |
| Badges | 42 |
| Lunar+ colors | 36 |

`Fits` 显示本地 Outfit 方案数量，`Skin Changer` 显示本地保存皮肤数量；两者不是购买型 catalog，因此本程序不会伪造或覆盖用户现有本地数据。

## 使用

1. 在 Lunar Client 中启动 Minecraft 1.8.9，等待游戏主菜单完整出现。
2. 双击运行 `dist\LunarUnlockUI.exe`。
3. UI 检测到 `Lunar Client 1.8.9` 后，点击“注入并解锁”。界面会等待代理完成运行时校验，只有全部分类验证成功后才显示“注入成功”。
4. 打开游戏内 Locker，检查各分类。

发布版已将 DLL 嵌入 EXE。实际使用只需要 `dist\LunarUnlockUI.exe`，界面不提供 DLL 选择入口。

运行日志位于：

```text
%TEMP%\LunarUnlockInjector\lunar_unlock_agent.log
```

每次重启游戏后需要重新注入；同一个 JVM 会阻止重复注入。

## 本地选择保存

注入器只保存当前客户端的装备选择，不会修改账号购买状态或服务端数据。首次注入没有配置时，Cosmetics 保持全部关闭；之后在 Locker 中装备或取消装备 Cosmetics、Emotes、Sprays，或更换 Lunar+ 颜色，后台会在约 750ms 内自动写入配置。

配置文件位置：

```text
%LOCALAPPDATA%\ColdEternityTeam\LunarLocalCosmetics\selection-v1.txt
```

再次启动游戏并注入后，保存的选择会自动恢复。删除 `selection-v1.txt` 可清除本地选择；配置文件仅包含饰品 ID、Emote/喷涂槽位、Jam ID 和颜色值，不包含登录信息。旧版 `emote=<id>` 配置会被兼容读取，并在下次保存时自动迁移为包含槽位和 Jam 参数的新格式。

## 构建

需要 Visual Studio C++ 工具链和 JDK 17 headers：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

构建产物：

- `bin\lunar_unlock_agent.dll`
- `bin\LunarUnlockInjector.exe`，命令行调试器
- `dist\LunarUnlockUI.exe`，内嵌 DLL 的单文件 UI

版本号统一定义在 `resource.h`，当前为 `1.1.0`。

## 二次修改与分发

本项目采用自定义 Source-Available License，具体条款见 [LICENSE](LICENSE)。主要要求：

- 二次修改无需事先授权。
- 二改必须显著标注 `ColdEternity Team` 和原始仓库链接。
- 发布二改可执行文件时，必须同步公开可复现构建的完整源码。
- 原版和二改版均不得收费、付费下载、会员解锁或用于商业获利。
- 不得删除或隐藏作者、许可证及原始仓库信息。

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=LongCold-ColdEternity-Team/Lunar-Local-Cosmetics-Unlocker&type=Date)](https://star-history.com/#LongCold-ColdEternity-Team/Lunar-Local-Cosmetics-Unlocker&Date)
