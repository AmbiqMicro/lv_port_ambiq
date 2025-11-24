# BSP 目录说明

## 目录结构

BSP 目录按 **BSP 名称**组织，而不是按 PART 名称，因为不同的 BSP 可能有不同的硬件配置（如 PSRAM 配置）。

```
bsp/
├── apollo510_evb/      # Apollo510 EVB 开发板
│   ├── linker_script.ld
│   ├── startup_gcc.c
│   └── flash.jlink.template
├── apollo510b_evb/     # Apollo510B EVB 开发板
│   ├── linker_script.ld
│   ├── startup_gcc.c
│   └── flash.jlink.template
└── apollo5b_eb_revb/   # Apollo5B EB RevB 开发板
    ├── linker_script.ld
    ├── startup_gcc.c
    └── flash.jlink.template
```

## 文件说明

### linker_script.ld
链接脚本，定义内存布局。不同 BSP 可能有不同的 PSRAM 配置：
- `apollo510_evb` 和 `apollo510b_evb`: PSRAM_TEX = 0x00100000, PSRAM_HEAP = 0x01F00000
- `apollo5b_eb_revb`: PSRAM_TEX = 0x00040000, PSRAM_HEAP = 0x01FC0000

### startup_gcc.c
启动文件，包含中断向量表和初始化代码。所有 apollo510 系列使用相同的启动文件。

### flash.jlink.template
J-Link 烧录脚本模板。使用 `$(TARGET)` 占位符，在构建时会被替换为实际的目标文件名。

## 使用方法

在 board 的 Makefile 中设置 `BSP` 变量：

```makefile
BSP = apollo510_evb
```

`common.mk` 会自动从 `bsp/$(BSP)/` 目录加载相应的文件。

## 添加新的 BSP

1. 在 `bsp/` 目录下创建新的 BSP 目录（如 `bsp/my_new_board/`）
2. 复制或创建以下文件：
   - `linker_script.ld` - 链接脚本
   - `startup_gcc.c` - 启动文件
   - `flash.jlink.template` - 烧录脚本模板
3. 在 board Makefile 中设置 `BSP = my_new_board`

## 注意事项

- BSP 名称必须与 `bsp/` 目录下的子目录名称一致
- 如果某个 board 需要特殊的链接脚本，可以在其 Makefile 中覆盖 `LINKER_FILE` 变量
- `flash.jlink.template` 中的 `$(TARGET)` 会在构建时自动替换

