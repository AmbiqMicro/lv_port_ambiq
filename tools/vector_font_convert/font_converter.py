#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import freetype
import re
import math
import ctypes
from dataclasses import dataclass
from typing import List, Dict, Tuple, Optional

# --- Nema VG Primitive Constants (from the C code) ---
NEMA_VG_PRIM_MOVE = 0x01
NEMA_VG_PRIM_LINE = 0x02
NEMA_VG_PRIM_BEZIER_QUAD = 0x05
NEMA_VG_PRIM_BEZIER_CUBIC = 0x06

# --- Data Structures for storing extracted font info ---
@dataclass
class GlyphInfo:
    codepoint: int
    data_offset: int
    data_length: int
    segment_offset: int
    segment_length: int
    x_advance: float
    kern_offset: int = 0
    kern_length: int = 0
    bbox: freetype.FT_BBox = freetype.FT_BBox()

@dataclass
class KerningPair:
    left: int
    right: int
    x_offset: float

# --- CTYPES DEFINITIONS FOR FT_Outline_Decompose ---

MoveToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
LineToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
ConicToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
CubicToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.POINTER(freetype.FT_Vector), ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)

class FT_Outline_Funcs(ctypes.Structure):
    _fields_ = [
        ("move_to", MoveToFunc), ("line_to", LineToFunc), ("conic_to", ConicToFunc),
        ("cubic_to", CubicToFunc), ("shift", ctypes.c_long), ("delta", ctypes.c_long)
    ]

class DecomposerData:
    def __init__(self):
        self.glyph_data: List[float] = []
        self.glyph_segments: List[int] = []

@MoveToFunc
def _move_to_c(to, user_data_ptr):
    decomposer = ctypes.cast(user_data_ptr, ctypes.POINTER(ctypes.py_object)).contents.value
    decomposer.glyph_segments.append(NEMA_VG_PRIM_MOVE)
    decomposer.glyph_data.extend([float(to.contents.x), float(to.contents.y)])
    return 0
@LineToFunc
def _line_to_c(to, user_data_ptr):
    decomposer = ctypes.cast(user_data_ptr, ctypes.POINTER(ctypes.py_object)).contents.value
    decomposer.glyph_segments.append(NEMA_VG_PRIM_LINE)
    decomposer.glyph_data.extend([float(to.contents.x), float(to.contents.y)])
    return 0
@ConicToFunc
def _conic_to_c(control, to, user_data_ptr):
    decomposer = ctypes.cast(user_data_ptr, ctypes.POINTER(ctypes.py_object)).contents.value
    decomposer.glyph_segments.append(NEMA_VG_PRIM_BEZIER_QUAD)
    decomposer.glyph_data.extend([float(control.contents.x), float(control.contents.y), float(to.contents.x), float(to.contents.y)])
    return 0
@CubicToFunc
def _cubic_to_c(control1, control2, to, user_data_ptr):
    decomposer = ctypes.cast(user_data_ptr, ctypes.POINTER(ctypes.py_object)).contents.value
    decomposer.glyph_segments.append(NEMA_VG_PRIM_BEZIER_CUBIC)
    decomposer.glyph_data.extend([
        float(control1.contents.x), float(control1.contents.y), float(control2.contents.x), float(control2.contents.y),
        float(to.contents.x), float(to.contents.y)
    ])
    return 0

outline_funcs = FT_Outline_Funcs(
    move_to=_move_to_c, line_to=_line_to_c, conic_to=_conic_to_c, cubic_to=_cubic_to_c, shift=0, delta=0
)

class FontConverter:
    def __init__(self, font_path: str, ranges: List[Tuple[int, int]], kerning: bool, output_name: Optional[str], debug: bool = False):
        self.font_path = font_path
        self.ranges = ranges
        self.enable_kerning = kerning
        self.debug = debug
        self.face = freetype.Face(self.font_path)
        print(f"Successfully loaded font: {self.face.family_name.decode()} {self.face.style_name.decode()}")

        base_name = output_name or self.font_path.split('/')[-1].split('\\')[-1].rsplit('.', 1)[0]
        self.font_name = re.sub(r'[\s\W]', '_', base_name)
        if self.enable_kerning: self.font_name += "_kern"
        self.font_name_upper = self.font_name.upper()

        self.data_buffer: List[float] = []
        self.segments_buffer: List[int] = []
        self.glyph_info_by_range: Dict[int, List[GlyphInfo]] = {}
        self.kerning_pairs: List[KerningPair] = []
        self.space_advance_width: Optional[float] = None

    def process_font(self):
        print("Processing glyphs...")
        data_offset, seg_offset, all_codepoints = 0, 0, []

        for r_idx, (start, end) in enumerate(self.ranges):
            self.glyph_info_by_range[r_idx] = []
            for codepoint in range(start, end + 1):
                all_codepoints.append(codepoint)
                if self.debug:
                    try: char = chr(codepoint)
                    except ValueError: char = '?'
                    print(f"\n--- Processing codepoint 0x{codepoint:X} ('{char}') ---")
                
                try:
                    self.face.load_char(codepoint, freetype.FT_LOAD_NO_BITMAP | freetype.FT_LOAD_NO_SCALE)
                except freetype.ft_errors.FT_Exception as e:
                    print(f"Warning: Could not load glyph for 0x{codepoint:X} ({e})")
                    continue
                
                glyph_advance_width = float(self.face.glyph.metrics.horiAdvance)
                
                if codepoint == 0x20 and self.space_advance_width is None:
                    self.space_advance_width = glyph_advance_width
                    if self.debug: print(f"  [Action] Found space. Storing its xAdvance ({glyph_advance_width}) for global font struct.")

                c_outline = self.face.glyph._FT_GlyphSlot.contents.outline
                c_outline_ptr = ctypes.byref(c_outline)
                
                # --- MODIFIED LOGIC ---
                # 1. Get the ORIGINAL BBox. This will be used for reporting.
                original_bbox = self.face.glyph.outline.get_bbox()
                
                if self.debug: print(f"  [Initial] Outline has {c_outline.n_points} points. Original BBox: {original_bbox}")
                
                # 2. Check the original BBox and translate the GEOMETRY if needed.
                if original_bbox.xMin < 0:
                    if self.debug: print(f"  [Action] Original BBox xMin is {original_bbox.xMin}, which is < 0. Translating geometry.")
                    freetype.FT_Outline_Translate(c_outline_ptr, -original_bbox.xMin, 0)
                else:
                    if self.debug: print(f"  [Info] Original BBox xMin is {original_bbox.xMin}, no translation needed.")
                # --- END MODIFIED LOGIC ---

                decomposer = DecomposerData()
                user_data = ctypes.py_object(decomposer)
                err = freetype.FT_Outline_Decompose(c_outline_ptr, ctypes.byref(outline_funcs), ctypes.pointer(user_data))
                if err: print(f"  [Error] FT_Outline_Decompose failed for 0x{codepoint:X} with error {err}")

                if self.debug:
                    print(f"  [Result] Decomposer collected {len(decomposer.glyph_data)} data points and {len(decomposer.glyph_segments)} segments.")
                    print("--- End of codepoint ---")

                glyph_info = GlyphInfo(
                    codepoint=codepoint, data_offset=data_offset, data_length=len(decomposer.glyph_data),
                    segment_offset=seg_offset, segment_length=len(decomposer.glyph_segments),
                    x_advance=glyph_advance_width, 
                    bbox=original_bbox # 3. Report the ORIGINAL BBox, regardless of translation.
                )
                self.glyph_info_by_range[r_idx].append(glyph_info)
                
                self.data_buffer.extend(decomposer.glyph_data)
                self.segments_buffer.extend(decomposer.glyph_segments)
                data_offset += len(decomposer.glyph_data)
                seg_offset += len(decomposer.glyph_segments)

        if self.enable_kerning and self.face.has_kerning: self._extract_kerning(all_codepoints)
            
    def _extract_kerning(self, codepoints: List[int]):
        print("Extracting kerning pairs...")
        kerning_map: Dict[int, List[KerningPair]] = {}
        for right_code in codepoints:
            for left_code in codepoints:
                kerning_vec = self.face.get_kerning(left_code, right_code, freetype.FT_KERNING_UNSCALED)
                if kerning_vec.x != 0:
                    pair = KerningPair(left=left_code, right=right_code, x_offset=float(kerning_vec.x))
                    self.kerning_pairs.append(pair)
                    if right_code not in kerning_map: kerning_map[right_code] = []
                    kerning_map[right_code].append(pair)
        self.kerning_pairs.sort(key=lambda p: p.right)
        for r_idx in self.glyph_info_by_range:
            for glyph in self.glyph_info_by_range[r_idx]:
                if glyph.codepoint in kerning_map:
                    glyph.kern_length = len(kerning_map[glyph.codepoint])
                    try:
                        first_pair_index = next(i for i, pair in enumerate(self.kerning_pairs) if pair.right == glyph.codepoint)
                        glyph.kern_offset = first_pair_index
                    except StopIteration:
                        glyph.kern_offset, glyph.kern_length = 0, 0

    def generate_files(self):
        self._generate_c_file()
        self._generate_h_file()
        print(f"\nSuccessfully generated {self.font_name}.c and {self.font_name}.h")
        self._calculate_memory_footprint()

    def _format_c_array(self, data: list, formatter, columns=12) -> str:
        if not data: return ""
        lines = []
        for i in range(0, len(data), columns):
            chunk = data[i:i+columns]
            lines.append("  " + ", ".join(formatter(item) for item in chunk))
        return ",\n".join(lines)

    def _generate_c_file(self):
        c_path = f"{self.font_name}.c"
        with open(c_path, 'w', encoding='utf-8') as f:
            f.write(f'#ifndef {self.font_name_upper}_C\n')
            f.write(f'#define {self.font_name_upper}_C\n\n')
            f.write(f'#include "{self.font_name}.h"\n')
            f.write('#include "nema_vg_context.h"\n\n')
            f.write(f"static const nema_vg_float_t {self.font_name}_data[] = {{\n")
            f.write(self._format_c_array(self.data_buffer, lambda x: f"{x:.2f}f"))
            f.write("\n};\n\n")
            f.write(f"static const uint8_t {self.font_name}_segments[] = {{\n")
            f.write(self._format_c_array(self.segments_buffer, str))
            f.write("\n};\n\n")
            kern_str = "NULL"
            if self.enable_kerning and self.kerning_pairs:
                kern_array_name = f"kerning_{self.font_name}"
                kern_str = f"&{kern_array_name}[0]"
                f.write(f"nema_vg_kern_pair_t {kern_array_name}[] = {{\n")
                for pair in self.kerning_pairs:
                    comment = f"// 0x{pair.left:X}, 0x{pair.right:X}"
                    f.write(f"  {{ 0x{pair.left:X}, {pair.x_offset:8.2f}f}}, {comment}\n")
                f.write("  { 0, 0} //end of array\n")
                f.write("};\n\n")
            for r_idx, glyph_list in self.glyph_info_by_range.items():
                f.write(f"static const nema_vg_glyph_t {self.font_name}Glyphs{r_idx}[] = {{\n")
                for glyph in glyph_list:
                    char_repr = f" '{chr(glyph.codepoint)}'" if ' ' <= chr(glyph.codepoint) <= '~' else ""
                    f.write(
                        f"  {{ {glyph.data_offset:6d}, {glyph.data_length:6d}, {glyph.segment_offset:6d}, "
                        f"{glyph.segment_length:6d}, {glyph.x_advance:8.2f}f, {glyph.kern_offset:4d}, "
                        f"{glyph.kern_length:4d}, {glyph.bbox.xMin:6d}, {glyph.bbox.yMin:6d}, "
                        f"{glyph.bbox.xMax:6d}, {glyph.bbox.yMax:6d}}},   // 0x{glyph.codepoint:08X}{char_repr}\n"
                    )
                f.write("};\n\n")
            f.write(f"static const nema_vg_font_range_t {self.font_name}_ranges[] = {{\n")
            for r_idx, (start, end) in enumerate(self.ranges):
                f.write(f"  {{0x{start:08x}, 0x{end:08x}, {self.font_name}Glyphs{r_idx}}},\n")
            f.write("  {0, 0, NULL}\n")
            f.write("};\n\n")
            font_ascender, font_descender = float(self.face.ascender), float(self.face.descender)
            default_size = font_ascender - font_descender
            if self.space_advance_width is not None:
                x_advance_font = self.space_advance_width
            else:
                x_advance_font = float(self.face.max_advance_width)
            f.write(f"nema_vg_font_t {self.font_name} = {{\n")
            f.write(f"    -1, // NEMA_VG_FONT_VERSION - To be defined by user\n")
            f.write(f"    {self.font_name}_ranges,\n")
            f.write(f"    {self.font_name}_data,\n")
            f.write(f"    {len(self.data_buffer)},\n")
            f.write(f"    {self.font_name}_segments,\n")
            f.write(f"    {len(self.segments_buffer)},\n")
            f.write(f"    {default_size:6.2f}f, //size\n")
            f.write(f"    {x_advance_font:6.2f}f, //xAdvance\n")
            f.write(f"    {font_ascender:6.2f}f, //ascender\n")
            f.write(f"    {font_descender:6.2f}f, //descender\n")
            f.write(f"    {kern_str},\n")
            f.write(f"    0, //flags\n")
            f.write(f"    {self.face.units_per_EM}, //units_per_em\n")
            f.write("};\n\n")
            f.write(f'#endif //{self.font_name_upper}_C\n')

    def _generate_h_file(self):
        h_path = f"{self.font_name}.h"
        with open(h_path, 'w', encoding='utf-8') as f:
            f.write(f'#ifndef {self.font_name_upper}_H\n')
            f.write(f'#define {self.font_name_upper}_H\n\n')
            f.write('#include "nema_vg_font.h"\n\n')
            f.write(f'extern nema_vg_font_t {self.font_name};\n\n')
            f.write(f'#endif // {self.font_name_upper}_H\n')

    def _calculate_memory_footprint(self):
        nema_vg_glyph_t_size, nema_vg_kern_pair_t_size = 36, 8
        nema_vg_font_range_t_size32, nema_vg_font_range_t_size64 = 12, 16
        nema_vg_font_t_size32, nema_vg_font_t_size64 = 52, 64
        num_glyphs = sum(len(g) for g in self.glyph_info_by_range.values())
        num_ranges = len(self.ranges) + 1
        text32, text64 = num_glyphs * nema_vg_glyph_t_size, num_glyphs * nema_vg_glyph_t_size
        text32 += num_ranges * nema_vg_font_range_t_size32
        text64 += num_ranges * nema_vg_font_range_t_size64
        kern_mem = (len(self.kerning_pairs) + 1) * nema_vg_kern_pair_t_size if self.enable_kerning else 0
        data_mem = len(self.data_buffer) * 4 + len(self.segments_buffer)
        data32, data64 = data_mem + kern_mem + nema_vg_font_t_size32, data_mem + kern_mem + nema_vg_font_t_size64
        total32, total64 = text32 + data32, text64 + data64
        print("---------------------------")
        print("Memory Footprint Estimation")
        print("---------------------------")
        print(f"\n32-bit system:\n\tTotal Memory    : ~{total32/1024:.1f} KB ({total32} bytes)")
        print(f"\t\t(.text) : {text32} bytes")
        if self.enable_kerning and kern_mem > 0: print(f"\t\t(.data) : {data32} bytes ({kern_mem} bytes for kerning)")
        else: print(f"\t\t(.data) : {data32} bytes")
        print(f"\n64-bit system:\n\tTotal Memory    : ~{total64/1024:.1f} KB ({total64} bytes)")
        print(f"\t\t(.text) : {text64} bytes")
        if self.enable_kerning and kern_mem > 0: print(f"\t\t(.data) : {data64} bytes ({kern_mem} bytes for kerning)")
        else: print(f"\t\t(.data) : {data64} bytes")

def parse_ranges(range_str: str) -> List[Tuple[int, int]]:
    ranges = []
    for part in range_str.split(','):
        part = part.strip()
        if '-' in part:
            start_str, end_str = part.split('-')
            try:
                start, end = int(start_str, 16), int(end_str, 16)
                if start > end: raise ValueError(f"Start > end in '{part}'")
                ranges.append((start, end))
            except ValueError as e: raise ValueError(f"Invalid hex in '{part}': {e}")
        else: raise ValueError(f"Range '{part}' needs '0xSTART-0xEND' format.")
    return ranges

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Python Font Converter for NEMA_VG.", formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('font_file', type=str, help='Path to the input TTF font file.')
    parser.add_argument('--ranges', type=str, required=True, help="Comma-separated Unicode ranges.\nExample: '0x20-0x7E,0xA0-0xFF'")
    parser.add_argument('--kerning', action='store_true', help='Enable kerning data extraction.')
    parser.add_argument('--output_name', type=str, default=None, help='Base name for output files (e.g., "my_font").')
    parser.add_argument('--debug', action='store_true', help='Enable detailed debug logging for each glyph.')
    args = parser.parse_args()
    try:
        parsed_ranges = parse_ranges(args.ranges)
        converter = FontConverter(
            font_path=args.font_file, ranges=parsed_ranges, kerning=args.kerning, 
            output_name=args.output_name, debug=args.debug
        )
        converter.process_font()
        converter.generate_files()
    except FileNotFoundError: print(f"Error: Font file not found: '{args.font_file}'")
    except freetype.ft_errors.FT_Exception as e: print(f"Error: FreeType error. Is '{args.font_file}' a valid font?\nDetails: {e}")
    except ValueError as e: print(f"Error parsing ranges: {e}")
    except Exception as e: print(f"An unexpected error occurred: {e}")