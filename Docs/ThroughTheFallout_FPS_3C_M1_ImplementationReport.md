# ThroughTheFallout FPS 3C M1 实现报告

日期：2026-11-13  
状态：实现已落地，未完成验收。按技术策划要求停止后续测试与验证，删除本轮新增的自动化测试、验证脚本及其输出；运行行为和手感由技术策划自行确认。

## 运行入口与交付

在 UE 内容浏览器打开 `/Game/ThroughTheFallout/Level/L_TTFFPS3C_M1`，由技术策划自行启动 Play。

也可以从工程根目录运行：

```powershell
.\Scripts\LaunchFPS3C.ps1 -EngineRoot 'C:\Users\Admin\GameEngine\UE\UE_5.8'
```

此入口不修改 BasicMap、默认 GameMode 或默认启动关卡。专用关卡保留作手动体验入口，包括移动跑道、距离标尺、40/70/90 cm 跳台、130 cm 低顶通道、掩体和阴影观察区域。

操作：WASD 移动；鼠标观察；Space 单次跳跃；按住左 Ctrl 下蹲。松开 Ctrl 后尝试起身，头顶受阻时保持蹲姿，离开障碍后自动站起。蹲姿不能起跳，不提供连跳、冲刺、滑铲、ADS 或射击。

HUD 提供中心准星。控制台命令 `ShowDebug FPS3C` 可显示水平速度、地面/空中/蹲姿状态及视觉眼高。

## 实现结构

### 角色与 ASC

`AFalloutCharacter` 继承 `AGastroCharacterWithExternalASC`，通过原框架绑定组件获取 PlayerState 的 ASC。FPS GameMode 指向 `BP_TTFCharacter`、`BP_TTFFPSController` 和现有 `BP_TTFPlayerState`，角色自身不创建 ASC。

现有 FalloutPlayerCharacter、FalloutPlayerController、FalloutPlayerCameraManager 空壳类和原有蓝图保留。新蓝图使用 TTF 标识，C++ 游戏类使用 Fallout 前缀。

### 输入与技能

`UFalloutFPSInputConfig` 扩展框架输入配置，增加二维 Move/MouseLook 及 Mapping Context 引用。Move/Look 由角色直接绑定 Enhanced Input；Jump/Crouch 通过 Controller 上的 `UGastroAbilityInputComponent` 路由到两项轻量能力。

按钮链路：InputAction → Gastro InputConfig/InputTag → PlayerState ASC → Fallout Ability → CharacterMovement。

Jump/Crouch 由 `DA_TTFFPSAbilities` 配置在 AvatarAbilitySets 中，随 Avatar 生命周期授予和回收。能力激活采用 LocalPredicted，但本轮不作多人可用性承诺。释放、取消、失焦和 Avatar 解绑会清理动作意图。

WASD 的动作值采用 Cumulative 累加，再将二维向量限幅至 1。鼠标直接使用 Enhanced Input 的原始键值路径，仅乘灵敏度，不乘 DeltaTime。Pitch 在增量累加后直接截断，再由 CameraManager 限制，避免极大鼠标增量先归一化而跨越边界。

### 移动、下蹲与相机

沿用 UE CharacterMovement 的地面、跳跃、空中控制与碰撞处理；不添加自定义移动预测或逐帧强写位置。胶囊尺寸使用标准 Crouch/UnCrouch 切换。

相机挂在胶囊体系下，以脚底为基准计算视觉眼高。胶囊中心变化时重新放置相机，从而将碰撞尺寸变化与视觉高度变化分离。眼高采用与帧率无关的指数收敛，约 0.16 秒完成 99% 过渡；移动、跳落和鼠标旋转不经过该平滑。

相机从胶囊内部安全点向目标眼位做小球 Sweep，在低顶下限制视觉位置。遮挡情况下可能比正常过渡更快压低镜头，这是防穿顶与视觉连续性之间的取舍。

### 双视图

1P 网格挂在相机下，仅拥有者可见，使用原生 FirstPerson 渲染、独立 FOV/缩放并关闭碰撞及投影。3P 网格使用 WorldSpaceRepresentation，对拥有者隐藏并保留世界投影。

复用现有 Manny 与持枪待机姿态作为占位。第一人称隐藏头颈和腿部骨骼，避免这些部位进入镜头。本轮未新增动画蓝图，也没有完整移动、跳跃或下蹲动画；没有武器射击逻辑。占位姿态的构图与阴影效果仍需人工确认。

## 调参位置与当前数值

在 `BP_TTFCharacter` 的 CharacterMovement、Capsule、FirstPersonCamera 和 FPS|Camera 分类下调整角色参数；在 `BP_TTFFPSController` 的 FPS|Input 分类下调整鼠标灵敏度。

| 参数 | 当前设置 |
|---|---|
| 地面速度 / 蹲行速度 | 600 / 300 cm/s |
| 最大加速度 / 地面刹车减速度 | 12000 / 12000 cm/s² |
| GroundFriction / BrakingFrictionFactor | 8 / 1 |
| JumpZVelocity / GravityScale | 700 cm/s / 2.5 |
| AirControl / AirControlBoostMultiplier | 0.8 / 1 |
| BrakingDecelerationFalling | 0 |
| 胶囊半径 / 站姿半高 / 蹲姿半高 | 34 / 96 / 56 cm |
| 站姿 / 蹲姿视觉眼高（距脚底） | 168 / 96 cm |
| 下蹲视觉过渡 | 约 0.16 秒到达 99% |
| 世界 FOV / 1P FOV / 1P Scale | 100° / 70° / 0.6 |
| 鼠标灵敏度 / Pitch 范围 | 0.08°/原始单位；-85°～85° |

以上为实现基线，未经过最终手感调优。约 100 cm 跳高、0.57 秒滞空来自参数推算，不能当作实测结果。

## 实现过程中遇到的问题与取舍

1. **原 GameMode 尚未接入游戏类。** 原 `BP_TTFGameMode` 的 Pawn、Controller、PlayerState 默认值指向 UE 基类。新增专用 FPS GameMode 配置完整链路，避免改变旧关卡的入口。
2. **轴输入与技能按钮职责不同。** 框架只接受布尔输入，无法直接携带二维移动或鼠标增量。通过项目层派生配置组合两个通道，避免扩展或修改框架。
3. **不能把全局鼠标平滑配置直接视为 FPS 输入结论。** 本机 UE 5.8 Enhanced Input 源码使用 RawValue 处理映射。FPS 使用独立无平滑修饰器的动作资产，因此保留项目原有全局输入配置。
4. **低顶下重新按住下蹲。** 标准 CanCrouch 在已蹲姿时可能返回 false。能力允许在实际已蹲姿时重新设置 Crouch 意图，避免释放后受阻、再次按住却未恢复下蹲意图。
5. **双网格采用本机引擎的原生投影。** UE 5.8 的 WorldSpaceRepresentation 会内部处理拥有者隐藏和 VSM 阴影，不叠加旧式 HiddenShadow 方案。效果待人工确认。
6. **构建时 DLL 被编辑器占用。** 曾出现 LNK1104；通过正常关闭编辑器后重建处理，没有修改 GastroFramework。另修正了项目代码的变量遮蔽及临时测试代码中的数值重载问题。
7. **资产脚本的版本差异。** 本机 Python API 不提供旧式 InputActionFactory；改用 DataAssetFactory。输入映射使用 UE 5.8 的 DefaultKeyMappings，FKey 通过其文本导入接口构造。

`CreateFPS3CAssets.py` 用于重新生成本轮独立资产，不是验证脚本。再次执行会覆盖生成资产的配置和专用关卡布局；手工调整后不要随意重跑。脚本不操作原有资产或框架路径。

## GastroFramework 使用反馈

本轮没有确认并复现可独立归因于 GastroFramework 的 BUG。以下属于设计限制或易用性建议，不应作为已确认缺陷：

| 分类 | 现象及影响 | 当前适配 / 建议 |
|---|---|---|
| 设计限制 | UGastroInputConfig 只支持 Boolean 能力按钮，Move/Look 不能放入同一原生配置 | 派生 FalloutFPSInputConfig 组合轴与按钮；可由框架提供通用轴输入扩展点 |
| 集成成本 | 配置输入组件、Enhanced Input 上下文、PlayerState ASC 和 AvatarAbilitySets 需要消费者分别装配 | 游戏 Controller 统一管理输入；建议提供可复用装配示例与生命周期说明 |
| 易用性 | 角色绑定组件和 Controller 输入组件分别负责 Ready/Uninitializing、输入跟随与释放，集成时容易遗漏清理时机 | 项目层增加 FlushPressedKeys/换 Pawn/结束处理；建议补充明确的集成清单 |
| 诊断建议 | 无效、重复动作和重复 Tag 复用较笼统的警告，较难直接定位具体配置条目 | 项目配置增加轴类型检查；建议框架诊断携带动作名、Tag 和条目索引 |

受保护目录 `Plugins/GastroFramework/` 没有本任务的源码、测试、配置或资产变更。原框架自带的测试与验证脚本不属于本轮新增内容，保持原状。

## 未完成的验收与交接

用户要求停止验证前，临时自动化尚未全部通过；移动、跳跃、下蹲场景没有得到预期结果，其根因未完成定位。不能据此认定运行功能通过，也不能将失败归因为框架 BUG。相关临时测试、验证脚本和输出已按要求删除，未继续追查或执行 PIE 验收。

需要技术策划自行确认的项目包括：实际键鼠输入方向与响应、启停距离和 ADAD 手感、跳跃轨迹与空中转向、低顶蹲站、视角过渡、占位双网格构图、阴影，以及失焦/换 Pawn/重复进入关卡后的输入状态。

本轮未执行最终多人、打包、性能或目标帧率验收。后续由技术策划反馈具体场景、预期与实际表现，再在游戏工程侧迭代。
