# lv\_port\_ambiq

**lv\_port\_ambiq** is a repository for porting LVGL to Ambiq Apollo series SoCs. It provides GPU-accelerated LVGL drawing interface implementations, along with all necessary drivers, detailed documentation,  and ready-to-use demo projects. This repository aims to help developers efficiently integrate and run LVGL in real-world customer applications on Ambiq platforms.

## Repository Overview

This repository covers the following key components:

### 1. Ambiq GPU Hardware Acceleration

Provides Ambiq GPU-based hardware acceleration for a wide range of LVGL graphic rendering interfaces. This includes—but is not limited to—rectangle filling, image rendering, layer blending, text rendering, line drawing, arc rendering, and other complex vector graphics operations. These enhancements significantly improve the rendering performance and visual experience of LVGL applications.

### 2. Integration of Graphics-Related LVGL Functionalities

Demonstrates how to integrate graphics-related peripheral drivers provided by AmbiqSuite to enable key LVGL functionalities on Apollo5 EVB, including:

* Display refresh(see `lv_ambiq_display.c`)
* Touch input handling(see `lv_ambiq_touch.c`)
* File system access(see `lv_ambiq_fs.c`)
* External memory usage (e.g., PSRAM for texture and buffer storage)(see `am_common.c` and `am_mem.c`)

Helper functions are provided to show how to initialize AmbiqSuite’s existing drivers and register utility functions to LVGL to support above graphical functions.

### 3. Memory Management Model

We provide a well-designed memory heap management model that offers fine-grained control over various types of memory used in graphics applications. This includes dedicated heap regions for:

* Draw buffers(frame buffers)
* Stencil  buffers
* Texture buffers
* Font bitmap buffers
* GPU command list memory
* NemaSDK internal used buffers
* CPU-only memory used internally by LVGL

This structure ensures optimal performance and efficient memory usage in resource-constrained embedded environments.

### 4. Ready-to-Use Detailed Demo Examples

* The repository provides complete and ready-to-use LVGL demo projects. These demos clearly illustrate the full integration process of LVGL and associated hardware components in actual customer projects.

## Quick Start

Follow these steps to compile and run the lvgl\_smartwatch demo on the Apollo5 EVB:

### Prerequisites

Make sure you have installed:

* AmbiqSuite build environment
* arm-none-eabi cross-compiler toolchain
* JLink toolset

Ensure you can compile and run examples provided in the AmbiqSuite package.

### Step-by-Step Guide

1. **Download and extract AmbiqSuite**

   * Obtain the AmbiqSuite release package from the official channel.
   * Extract it into your current repository directory (`lv_port_ambiq`). Your directory structure should look like this:

```
lv_port_ambiq
├── AmbiqSuite
│   ├── CMSIS
│   ├── boards
│   ├── debugger_updates
│   ├── devices
│   ├── docs
│   ├── makedefs
│   ├── mcu
│   ├── pack
│   ├── third_party
│   ├── tools
│   └── utils
├── LVGL
│   ├── demos
│   ├── docs
│   ├── env_support
│   ├── examples
│   ├── libs
│   ├── scripts
│   ├── src
│   ├── tests
│   ├── xmls
│   └── zephyr
├── lvgl_ambiq_demo
│   ├── lvgl_smartwatch
│   └── lvgl_test
├── lvgl_ambiq_porting
│   ├── NemaGFX_hal
│   └── gpu_patch
├── modules
    └── tlsf
```

2. **Build the demo**

   * Navigate to the smartwatch demo directory:

```shell
cd lvgl_ambiq_demo/lvgl_smartwatch/Apollo510_evb
```

* Run the following command to compile the demo:

```shell
make
```

3. **Flash the demo**

   * Connect your Apollo5 EVB via JLink.
   * Use the following command to flash the compiled binary to the board:

```shell
make flash
```

4. **Run and verify**

   * After flashing, the demo should automatically run on your Apollo5 EVB, displaying the smartwatch UI powered by LVGL.

## Learn More

To gain a deeper understanding of how to integrate Ambiq GPU-accelerated LVGL into your own project, and to explore our recommended memory heap management strategies, please refer to the following files and directories:

* `docs/integration_guide.md`: Describes how to integrate display refresh, touch input, file system access, and related functionalities into a customer project..
* `docs/memory_management.md`: Describes best practices for dynamic memory allocation and heap configuration in resource-constrained environments.

These resources are designed to help developers confidently integrate LVGL into their own embedded applications on the Ambiq platform.

## Contribution and Feedback

Feel free to submit Issues and Pull Requests. We welcome your feedback and contributions to help continually enhance the LVGL and Ambiq platform ecosystem.

## License

This repository is licensed under the MIT License. For more details, please see the [LICENSE](LICENSE) file.
