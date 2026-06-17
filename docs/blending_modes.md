# Ambiq Hardware Blending Modes vs LVGL Software Blending Modes

This document provides a detailed explanation of the calculation methods and differences between Ambiq hardware GPU blending modes and LVGL software blending modes when the source is in ARGB format, under different frame buffer (Frame Buffer) formats.

---

## Overview

In the LVGL port for Ambiq, when the Ambiq hardware draw unit is enabled (via `LV_USE_DRAW_AMBIQ`), the hardware GPU blending modes are selected based on the frame buffer format. The hardware draw unit chooses different GPU blending modes depending on the color format:

- **Hardware GPU Blending**: Uses the Ambiq NemaGFX GPU's hardware blending unit for accelerated rendering
- **Software Blending**: Uses LVGL's built-in software blending algorithms for rendering (used when hardware draw unit is not enabled or when certain operations are not supported by hardware)

**Important Note:** Due to hardware limitations, the Ambiq GPU does not provide a blending mode that exactly matches LVGL's software blending algorithm for 32-bit to 32-bit blending. As a result, we use `NEMA_BL_SRC_OVER` as the blending mode for this scenario, this blending method can produce different rendering results compared to LVGL software blending due to differences in their calculation formulas.

When using the Ambiq hardware draw unit, different GPU blending modes (`NEMA_BL_SRC_OVER` or `NEMA_BL_SIMPLE`) are selected based on the frame buffer format. 

---

## 1. Frame Buffer Format: RGB565 / RGB888

When the frame buffer format is **RGB565** or **RGB888**, only the source (Source) has an Alpha channel, while the destination (Destination) does not have an Alpha channel.

### 1.1 Blending Mode Selection

- **Hardware GPU**: Uses `NEMA_BL_SIMPLE` blending mode
- **LVGL Software**: Uses the same simple blending logic

### 1.2 Calculation Formula

Both hardware GPU and LVGL software use the **same blending formula**:

```
Cout = Cs · Sa + Cd · (1 - Sa)
```

Where:
- `Cout`: Output color (R, G, B channels)
- `Cs`: Source color (Source Color)
- `Cd`: Destination color (Destination Color)
- `Sa`: Source Alpha value (Source Alpha), range [0, 1] (shown in normalized form for formula illustration)

**Note:**
- The formula above shows all values in [0, 1] range for illustration purposes
- Since the destination has no Alpha channel, the blending result does not contain Alpha information
- Although both hardware and software use the same formula, there may be subtle differences in rendering results due to precision issues (e.g., rounding of decimal values during the calculation process)

---

## 2. Frame Buffer Format: ARGB8888

When the frame buffer format is **ARGB8888**, both the source (Source) and destination (Destination) have Alpha channels.

### 2.1 Blending Mode Selection

- **Hardware GPU**: Uses `NEMA_BL_SRC_OVER` blending mode
- **LVGL Software**: Uses a custom Alpha blending algorithm

### 2.2 Hardware GPU Blending Formula (NEMA_BL_SRC_OVER)

The hardware GPU uses the standard "Source Over" blending mode:

**Color Output:**
```
Cout = Cs · 1 + Cd · (1 - As)
```

**Alpha Output:**
```
Aout = As + Ad · (1 - As)
```

Where:
- `Cout`: Output color (R, G, B channels)
- `Aout`: Output Alpha value
- `Cs`: Source color (Source Color)
- `Cd`: Destination color (Destination Color)
- `As`: Source Alpha value (Source Alpha), normalized to [0, 1]
- `Ad`: Destination Alpha value (Destination Alpha), normalized to [0, 1]

**Note:** In the hardware implementation, all color and Alpha values are normalized to the [0, 1] range for calculation.

### 2.3 LVGL Software Blending Formula

LVGL software uses a different Alpha blending algorithm:

**Alpha Output:**
```
Aout = 255 - ((255 - As) · (255 - Ad)) / 255
```

**Ratio Calculation:**
```
ratio = (As · 255) / Aout
```

**Color Output:**
```
Cout = (Cs · ratio + Cd · (255 - ratio)) / 255
```

Where:
- `Cout`: Output color (R, G, B channels)
- `Aout`: Output Alpha value
- `Cs`: Source color (Source Color)
- `Cd`: Destination color (Destination Color)
- `As`: Source Alpha value (Source Alpha), range [0, 255]
- `Ad`: Destination Alpha value (Destination Alpha), range [0, 255]
- `ratio`: Intermediate calculation value used for color blending

**Note:** In the software implementation, all color and Alpha values use the [0, 255] range.

### 2.4 Formula Difference Analysis

Although both methods implement Alpha blending, they use different calculation formulas, which can result differences in certain scenarios:

1. **Alpha Calculation Difference**:
   - Hardware: `Aout = As + Ad · (1 - As)`
   - Software: `Aout = 255 - ((255 - As) · (255 - Ad)) / 255`
   
   These two formulas are **mathematically equivalent**, but may produce subtle differences due to rounding errors in integer arithmetic.

2. **Color Calculation Difference**:
   - Hardware: Directly uses `Cs · 1 + Cd · (1 - As)`, which is the standard Porter-Duff "Source Over" formula
   - Software: First calculates `ratio = (As · 255) / Aout`, then uses `(Cs · ratio + Cd · (255 - ratio)) / 255`
   
   These two formulas are **mathematically not equivalent**, which will cause different color blending results.

---

## 3. Important Notes

### 3.1 Rendering Differences

**Current GPU blending modes do not exactly match the software blending implementation.**

Since the hardware GPU uses NEMA_BL_SRC_OVER blending formula, while LVGL software uses a custom Alpha blending algorithm, they will produce **rendering differences** in ARGB8888 format. These differences are mainly manifested in:

- Color blending results of semi-transparent objects
- Cumulative effects of multi-layer Alpha blending

### 3.2 Recommendations

1. **For RGB565/RGB888 formats**:
   - Hardware and software blending results are identical, hardware acceleration is recommended

2. **For ARGB8888 format**:
   - If the application does not require high rendering precision, hardware acceleration can be used for better performance
   - If the application needs to be completely consistent with standard LVGL behavior, software blending may be required or additional color correction may be needed

---
