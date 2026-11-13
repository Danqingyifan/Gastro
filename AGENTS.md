# ThroughTheFallout — Agent 工程规范

## 适用范围与职责

- 本仓库承载 ThroughTheFallout 游戏工程，当前 Unreal 工程文件为 `Gastro.uproject`。
- 本文件适用于所有参与 ThroughTheFallout 工作的 AI Agent，以及其委派的 Agent、脚本和自动化工具。
- 工程 Agent 负责游戏工程侧的实现、集成和验证；`GastroFramework` 是独立维护的底层框架依赖，不属于游戏工程 Agent 的修改范围。

## 第一条红线：禁止修改 GastroFramework

**属于 ThroughTheFallout 工程的 Agent 不得修改 GastroFramework 的任何代码。**

- 受保护路径：`Plugins/GastroFramework/`。其中源码、测试、构建规则（`*.Build.cs`）、插件描述（`*.uplugin`），以及框架维护的配置、资产和文档均视为只读。
- 禁止在受保护内容中新增、修改、删除、移动、重命名或格式化文件；禁止通过补丁、代码生成、批量替换、编辑器保存或其他工具间接改写。
- 禁止以修复游戏功能、消除编译错误、通过测试、接口适配或重构为由修改框架。任务文档赋予的技术自主权仅适用于游戏工程侧，不能扩大到框架内部。
- 禁止通过工程脚本、构建步骤或复制后替换插件的方式绕过本规则；也不得让其他 Agent 代为修改。
- 正常编译产生的被 Git 忽略的 `Binaries/`、`Intermediate/` 等构建产物不属于框架源文件修改；不得借此修改框架源码或受版本管理的文件。

## 第二条规范：游戏类命名

- 在 Unreal Editor 中创建的游戏蓝图类，统一使用 `TTF` 作为项目标识，命名格式为“资产类型前缀 + `_TTF` + 语义名称”。例如普通蓝图类为 `BP_TTFCharacter`，动画蓝图类为 `ABP_TTFCharacter`。
- 在 C++ 中创建的游戏类，统一使用 `Fallout` 作为项目名前缀，例如 `FalloutCharacter`。保留 Unreal 要求的类型前缀（如 `A`、`U`），将 `Fallout` 紧接在类型前缀之后：Actor 类为 `AFalloutCharacter`，UObject 派生类可为 `UFalloutCharacterComponent`；不需要 Unreal 类型前缀的普通 C++ 类则直接命名为 `Fallout...`。
- C++ 头文件和实现文件使用不带 Unreal 类型前缀的类名，例如 `FalloutCharacter.h`、`FalloutCharacter.cpp`。
- 本规范用于游戏工程侧新增类；不得以统一命名为由修改或重命名 GastroFramework 中的任何类型或文件。

## 游戏工程侧的集成方式

- 可以阅读框架源码和文档、调用公开接口、继承允许扩展的类，并组合框架提供的组件。
- 游戏专属功能、适配逻辑和测试应放在游戏工程自己的模块及目录中，通过现有扩展点实现。
- 如果现有接口无法满足需求，或发现框架缺陷，应记录复现方式、预期行为、实际行为、影响及所需接口，交由框架维护方在独立框架任务中处理。当前游戏任务继续完成不依赖框架修改的工作，并明确说明受阻部分。

## 变更检查与交付

- 开工时记录工作区已有改动；交付前检查暂存、未暂存及新增文件，确认本任务没有改动受保护内容。
- 新增类的命名必须符合上述蓝图 `TTF`、C++ `Fallout` 的约定。
- 如发现他人已有的框架改动，应保留并说明，不得自行覆盖、回滚或纳入本任务提交。
- 子目录的 Agent 规范和具体任务方案不得放宽上述框架边界。
