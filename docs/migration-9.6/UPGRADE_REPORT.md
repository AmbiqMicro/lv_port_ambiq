_What LVGL 9.4–9.6 requires of the Ambiq GPU draw unit (`LVGL/src/draw/ambiq/`)
beyond compiling. From a review of the core draw pipeline (`v9.3.0` →
`release/v9.6`) and of how upstream's own GPU units (NemaGFX above all, the same
GPU family) adapted over the same range. Upstream references are at
`release/v9.6`, Ambiq references at `feat-lvgl-v9.6` `48f81fcad`. Nothing here
has been fixed or exercised on hardware yet._

_All four demos set `LV_USE_DRAW_SW 0` and `LV_AMBIQ_CPU_GPU_ASYNC 1`, so the
Ambiq unit is the only renderer: a task it declines in `evaluate_cb` has no
software fallback. Since 9.6 (`dec721c89`) such a task is marked
`LV_DRAW_TASK_STATE_FAILED`, is not drawn and logs a warning and an error per
task per frame; in 9.3 it was dropped silently. Several items below matter more
because of that._

# REQUIRED

- **Line tasks with a point array** — upstream `d99d2a553` (#9269), 9.5
  - Description: `lv_draw_line_dsc_t` gained `points` / `point_cnt`
    (`include/lvgl/draw/lv_draw_line.h:47,52`), with `LV_DRAW_LINE_POINT_NONE`
    marking gaps; when `points` is set, `p1` / `p2` are unused. Every upstream
    unit dispatches lines through `lv_draw_line_iterate(t, dsc, cb)`
    (`src/draw/lv_draw_line.c:135`). The Ambiq unit calls
    `lv_draw_ambiq_line(t, t->draw_dsc)` directly (`lv_draw_ambiq.c:498`), and
    `lv_draw_ambiq_line.c:130` returns early when `p1 == p2`. Fix: one line,
    `lv_draw_line_iterate(t, t->draw_dsc, lv_draw_ambiq_line);` — the callback
    signature already matches.
  - Consequences: `lv_chart` line series (`lv_chart.c:1107`) now draw this way,
    so they render nothing. Any other caller of the point-array API likewise.

- **A failed layer-buffer allocation must fail the task** — upstream `357cd5844` (#10369), 9.6
  - Description: when `lv_draw_layer_alloc_buf()` returns NULL, every upstream
    unit now sets `t->state = LV_DRAW_TASK_STATE_FAILED` before returning (e.g.
    `nema_gfx/lv_draw_nema_gfx.c:298`); the core then finalizes the task. The
    Ambiq dispatch only returns `LV_DRAW_UNIT_IDLE` (`lv_draw_ambiq.c:384-387`).
    Fix: set the state before the return.
  - Consequences: on out-of-memory, or at `LV_DRAW_LAYER_MAX_MEMORY`, the task
    is retried forever and the dispatcher busy-loops instead of skipping it.

- **Assertions are compiled out by default** — upstream `6339dcb9e` (#10513), 9.6
  - Description: `LV_ASSERT`, `LV_ASSERT_MSG` and `LV_ASSERT_FORMAT_MSG` are now
    gated by `LV_USE_ASSERT`, default 0
    (`include/lvgl/debugging/lv_assert.h:59-70`); the demos' `lv_conf.h` don't
    set it. Fix: `LV_USE_ASSERT 1` at least in debug builds. Enabling it exposes
    a pre-existing bug: `lv_draw_ambiq_buffer_copy` ends in
    `else { LV_ASSERT_MSG(false, "Should never come to this!"); }`
    (`lv_draw_ambiq_buffer.c:284-286`), reached after a correct blit for every
    plain RGB565 / ARGB copy (double-buffer sync in `lv_refr.c:782-784`, canvas,
    image decoder). That branch should be a no-op.
  - Consequences: since the upgrade, the unit's NEMA pool-membership checks
    (`lv_draw_ambiq_buffer.c:100,139,153`), the copy size check (`:250`), the
    colour-format check (`lv_draw_ambiq_private.c:709`) and the core's
    "task still used by a draw unit" check are all silently off, so faults
    they used to catch now surface as corruption instead. `LV_ASSERT_NULL` and
    `LV_ASSERT_MALLOC` still work.

# RECOMMENDED

- **Clip and cache-maintain on `t->_real_area`, not `t->area`** — pre-existing; upstream NemaGFX fixed it in `9e61721bb` (#10295)
  - Description: the core enlarges `_real_area` beyond `area` for box shadow
    (`lv_draw_rect.c:248-250`), outline (`:383-384`) and transformed
    image/LAYER tasks (`lv_draw_image.c:84-86`, `:148-150`). The Ambiq unit
    recomputes a transformed area only for `LV_DRAW_TASK_TYPE_IMAGE` and uses
    `t->area` for everything else (`lv_draw_ambiq.c:444-473`). Fix:
    `lv_area_intersect(&draw_area, &t->_real_area, &t->clip_area)`, then
    translate by the layer's `buf_area`.
  - Consequences: rotated or scaled LAYER tasks (widget transforms) are clipped
    to their untransformed box; a box shadow whose object lies outside the
    refresh clip is skipped entirely (early return at `:472`); the cache
    flush/invalidate at `:479` / `:544` covers too small a region.

- **Vector tasks must apply the layer opacity `t->opa`** — upstream `64a14311f` (#8294), 9.4
  - Description: `lv_draw_task_t` gained `opa`, filled from `layer->opa`
    (`lv_draw.c:107`). Widget descriptors already mix layer opacity in, but
    `lv_draw_vector` descriptors (SVG, Lottie, user vector code) do not, so the
    SW unit (`sw/lv_draw_sw_vector.c:515`) and VG-Lite
    (`vg_lite/lv_draw_vg_lite_vector.c:277,326`) multiply `t->opa` into fill,
    stroke, gradient and pattern opacity. Nothing in `lv_draw_ambiq_vector.c`
    reads it (`:392,504,526`). Upstream NemaGFX ignores it too, so it is not a
    reference here.
  - Consequences: vector content inside a semi-transparent parent or layer
    renders at full opacity.

- **Image colorkey** — upstream `b3bd89d1d` (#8628), 9.4
  - Description: `lv_draw_image_dsc_t.colorkey` was added
    (`include/lvgl/draw/lv_draw_image.h:84`), and `bg_image_colorkey` on rect
    descriptors is copied into the image task. SW and VG-Lite
    (`vg_lite/lv_draw_vg_lite_img.c:96`) honour it. The Ambiq IMAGE evaluate
    (`lv_draw_ambiq.c:315-331`) claims every image and never reads it. Fix:
    decline in `evaluate_cb` when `colorkey != NULL`, or implement it
    (`NEMA_BLOP_SRC_CKEY` keys a single colour; LVGL's key is a low..high
    range — unverified for this SDK).
  - Consequences: keyed pixels draw opaque instead of transparent.

- **Blur and drop shadow** — upstream `e54d66863` (#9223), `bb3233b79` (#9331), 9.5
  - Description: new `LV_DRAW_TASK_TYPE_BLUR`, used by the `blur_radius` style
    (incl. backdrop blur, `core/lv_obj.c:1640-1734`) and by drop shadows: the
    core draws the shape into an **A8** layer, adds a BLUR task, then composites
    it as a LAYER with `recolor` = shadow colour (`lv_draw.c:574-619`). Only SW
    and NanoVG (`f92d9d282`) implement BLUR; NemaGFX has no native blur, so a
    GPU version means multiple passes. The Ambiq A8+recolor composite looks
    compatible (`lv_draw_ambiq_img.c:187,262-270`), but drawing *into* an A8
    destination is unverified: fill uses `NEMA_BL_SIMPLE` for any non-ARGB8888
    destination (`lv_draw_ambiq_fill.c:95`).
  - Consequences: with no SW unit, BLUR tasks FAIL — drop shadows composite as
    sharp silhouettes and `blur` styles do nothing.

- **Linear, radial and conical gradients** — upstream `d493a5d02` (#8834); NemaGFX implementation `e6a1031c0` (#8696)
  - Description: complex gradients are core features; upstream NemaGFX draws
    linear and radial ones with NemaVG (`lv_draw_nema_gfx_fill.c:119-170`;
    extend modes PAD→`NEMA_TEX_CLAMP`, REPEAT→`NEMA_TEX_REPEAT`,
    REFLECT→`NEMA_TEX_MIRROR`). The Ambiq FILL / TRIANGLE evaluate declines them
    (`lv_draw_ambiq.c:262-296`). Porting upstream's code is the shortest path.
  - Consequences: with no SW unit these gradients are not drawn (FAILED, with
    log spam each frame).

- **FreeType vector-font text outline** — upstream `e11332935` (#9725), the 9.5 vector-font work
  - Description: FreeType no longer produces a separate border path
    (`LV_FREETYPE_OUTLINE_BORDER_START` is gone); units stroke the glyph path
    themselves with `outline_stroke_width` / `_color` / `_opa`, then fill it
    (`sw/lv_draw_sw_letter.c:281-293`; VG-Lite `lv_draw_vg_lite_label.c:515-530`,
    round cap/join, width ÷ scale, bounding box grown by the stroke). Ambiq's
    stroke branch still needs `glyph_border`, now always NULL
    (`lv_draw_ambiq_vector_font.c:195-207`). Fix: stroke `outline->glyph_path`
    with `NEMA_VG_STROKE` before the fill, grow the early clip test (`:152-153`)
    by the stroke width, drop the `glyph_border` plumbing.
  - Consequences: text outlines on FreeType vector fonts are silently not drawn.
    No current demo is affected (all have `LV_USE_FREETYPE 0`).

- **Dynamically loaded binfont glyphs** — upstream `f57962a93`, 9.6
  - Description: with `lv_binfont_create_ex(.dynamic_glyph_load = true)`, a
    raw-bitmap request returns a scratch buffer that the next glyph overwrites
    (`lv_font_fmt_txt.c:104-113`); upstream NemaGFX clears `req_raw_bitmap` for
    such fonts (`lv_draw_nema_gfx_label.c:836`). Ambiq starts from
    `lv_font_has_static_bitmap()` but then forces `is_plain = true` for any
    PLAIN fmt_txt font and hands the pointer to a command list submitted after
    the whole label (`lv_draw_ambiq_letter.c:185-191`). Fix: add
    `&& !fdsc->are_glyphs_dynamic_loaded`. Whether `lv_ambiq_draw_bitmap_glyph`
    (closed `gpu_patch` library) copies the bitmap could not be checked.
  - Consequences: if dynamic glyph loading is used, glyphs in a label likely all
    show the last glyph's bitmap or garbage. Not used by the current demos.

- **`wait_for_finish_cb` must wait for the draw thread** — upstream `fee343d59` (#10556), `329655b90`, 9.6
  - Description: `lv_draw_layer_deinit()` (`lv_draw.c:621-641`) now calls
    `lv_draw_wait_for_finish()` when one of its tasks is in progress, then frees
    the tasks, relying on each unit's callback to block until it is idle; the SW
    unit gained exactly that. Ambiq registers the callback only when
    `LV_AMBIQ_CPU_GPU_ASYNC == 1` (`lv_draw_ambiq.c:143`), and it only submits
    and waits on the command list (`:551-575`), not for `task_act == NULL` on
    the "ambiqdraw" thread; in sync mode with an OS there is none.
  - Consequences: possible use-after-free when a layer is torn down mid-draw
    (canvas or snapshot abandoned, display deleted). Rare paths, and the core's
    guarding assert is compiled out by default (see REQUIRED).

# LOW PRIORITY

- **QUEUED state for asynchronous units** — upstream `b11ac069f` (#8512), 9.4 (optional)
  - Description: `LV_DRAW_TASK_STATE_QUEUED` means "in the unit's command
    queue"; a unit's own queued tasks don't block each other (`lv_draw.c:711`),
    so a unit can batch dependent tasks and mark them FINISHED from
    `wait_for_finish_cb`. Ambiq marks tasks FINISHED as soon as they are
    recorded (`lv_draw_ambiq.c:248`). No upstream unit uses QUEUED yet.
  - Consequences: none today. The core frees what a task references (layer
    buffers, line point arrays, label text) once it is FINISHED, possibly before
    the GPU runs the command list — the layer-buffer case already applied in 9.3.

- **`event_cb` on draw units** — upstream `0045018f2` (#8891), 9.4 (optional)
  - Description: `lv_draw_unit_t.event_cb` (`lv_draw_private.h:176`) receives
    `LV_EVENT_CHILD_CREATED` / `_DELETED` for every layer (`lv_draw.c:405,487`),
    replacing the removed `disp->layer_init` / `layer_deinit`. VG-Lite uses it
    for cache events. Ambiq does not, and lv_port never used the old hooks.
  - Consequences: none; a place to manage per-layer GPU resources if needed.

- **Matrix / perspective image transforms** — `LV_DRAW_TRANSFORM_USE_MATRIX`, VG-Lite `bb8ef34b1` (#10123) (optional)
  - Description: with that option, tasks carry a `matrix`; Ambiq (like upstream
    NemaGFX) ignores it.
  - Consequences: only matters if the option is enabled.

- **Display `sync_cb`** — 9.6 (optional)
  - Description: an alternative to `buf_copy_cb` for double-buffered
    direct-mode sync (`lv_refr.c:770-784`).
  - Consequences: none; Ambiq's `buf_copy_cb` already covers it.

- **Range-based cache maintenance** — upstream NemaGFX `dc9e9dee5` (#10062); dma2d `f23802343` (#9366), `fc2ea9a05` (#9642); PXP `6f4627186` (#8764) (worth checking)
  - Description: upstream cleans and invalidates the D-cache per row over
    (`_real_area` ∩ clip) and aligns buffers and cache operations to the cache
    line. Ambiq flushes and invalidates the whole buffer, only
    `#if LV_USE_DRAW_SW`. `flush_core` skips buffers without
    `LV_IMAGE_FLAGS_ALLOCATED` (`lv_draw_ambiq_buffer.c:125-128`) and
    `invalidate_core` asserts the buffer is in a NEMA pool (`:153-154`).
    `lv_draw_ambiq_buffer_clean` (the clear hook) does no cache maintenance.
    `LV_DRAW_BUF_ALIGN` should be a multiple of the M55's 32-byte line.
  - Consequences: correct but costlier than needed. Possible stale-cache issues
    with static display buffers, or with CPU-written buffers the GPU then clears
    (qrcode, barcode, gif, tiny_ttf) — unverified.

- **Vector path data freed before the GPU runs** — compare upstream NemaGFX `75efb0a12` (#8938) (worth checking)
  - Description: upstream submits and waits on the command list before freeing
    the translated path-op array. Ambiq frees `vg_path_seg` right after
    `nema_vg_draw_path` and closes the paint's image decoder before the GPU runs
    (`lv_draw_ambiq_vector.c:644-652`, with its own TODO).
  - Consequences: safe only if NemaVG consumes the path data on the CPU during
    `draw_path`; otherwise corrupted vector shapes or patterns. Unverified.

- **Vector scissor offset** — pre-existing (worth checking)
  - Description: `scissor_area` moves by `+buf_area`
    (`lv_draw_ambiq_vector.c:567-570`) while the global matrix translates by
    `-buf_area`.
  - Consequences: possibly wrong clipping of vector content in partial-mode or
    layer rendering, where `buf_area` is not at the origin.

- **Glyph fill rule** — pre-existing (worth checking)
  - Description: Ambiq fills FreeType glyph outlines `NEMA_VG_FILL_EVEN_ODD`
    (`lv_draw_ambiq_vector_font.c:180`); VG-Lite uses non-zero.
  - Consequences: fonts with overlapping contours (common in variable fonts)
    may render with holes.

- **Glyph data contract** — documented in 9.6, `include/lvgl/font/lv_font.h:112-131` (worth checking)
  - Description: `get_glyph_bitmap`'s return value is the authoritative glyph
    data, valid only until `release_glyph`, and `_draw_buf` is internal. Ambiq's
    non-raw path reads `_draw_buf->data` instead of the returned pointer
    (`lv_draw_ambiq_letter.c:238,255`). `lv_ambiq_ttf` sets `static_bitmap = 1`
    (`lv_ambiq_ttf.c:261`) while returning cache entries (`:350`) that the core
    releases right after the call.
  - Consequences: wrong glyphs for fonts that return their own buffer (tiny_ttf,
    FreeType bitmap mode); in async mode the GPU may read an evicted
    `lv_ambiq_ttf` cache entry.

- **One-pixel clip areas rejected** — pre-existing (worth checking)
  - Description: `lv_draw_ambiq_common_start` tests `x2 - x1 <= 0`
    (`lv_draw_ambiq_private.c:379`), so a clip 1 px wide or high is treated as
    empty and the task is marked finished.
  - Consequences: 1-pixel-wide or -high draws (thin lines, separators, edge
    slivers in partial refresh) are silently skipped.

- **Image `clip_radius`** — pre-existing (worth checking)
  - Description: only the tiled image path warns about it; the normal path
    ignores it (`lv_draw_ambiq_img.c:127,375`). Decline in `evaluate_cb` until
    implemented.
  - Consequences: images with rounded clipping draw with square corners.
