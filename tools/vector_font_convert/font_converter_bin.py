#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import freetype
import re
import math
import struct
import ctypes
from dataclasses import dataclass
from typing import List, Dict, Tuple, Optional

# --- Binary File Format Constants ---
FILE_MAGIC = 0x4E464F4E  # "NFON"
FILE_VERSION = 6

# --- Nema VG Primitive Constants ---
NEMA_VG_PRIM_MOVE, NEMA_VG_PRIM_LINE = 0x01, 0x02
NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_CUBIC = 0x05, 0x06

# --- Data Structures ---
@dataclass
class GlyphInfo:
    codepoint: int; data: List[float]; segments: List[int]
    x_advance: float; bbox: freetype.FT_BBox

# --- Ctypes Definitions for FreeType Interop ---
MoveToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
LineToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
ConicToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
CubicToFunc = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(freetype.FT_Vector), ctypes.POINTER(freetype.FT_Vector), ctypes.POINTER(freetype.FT_Vector), ctypes.c_void_p)
class FT_Outline_Funcs(ctypes.Structure):
    _fields_ = [("move_to", MoveToFunc), ("line_to", LineToFunc), ("conic_to", ConicToFunc), ("cubic_to", CubicToFunc), ("shift", ctypes.c_long), ("delta", ctypes.c_long)]
class DecomposerData:
    def __init__(self): self.glyph_data, self.glyph_segments = [], []
@MoveToFunc
def _move_to_c(to, ptr): (d:=ctypes.cast(ptr, ctypes.POINTER(ctypes.py_object)).contents.value).glyph_segments.append(NEMA_VG_PRIM_MOVE); d.glyph_data.extend([float(to.contents.x), float(to.contents.y)]); return 0
@LineToFunc
def _line_to_c(to, ptr): (d:=ctypes.cast(ptr, ctypes.POINTER(ctypes.py_object)).contents.value).glyph_segments.append(NEMA_VG_PRIM_LINE); d.glyph_data.extend([float(to.contents.x), float(to.contents.y)]); return 0
@ConicToFunc
def _conic_to_c(c, to, ptr): (d:=ctypes.cast(ptr, ctypes.POINTER(ctypes.py_object)).contents.value).glyph_segments.append(NEMA_VG_PRIM_BEZIER_QUAD); d.glyph_data.extend([float(c.contents.x), float(c.contents.y), float(to.contents.x), float(to.contents.y)]); return 0
@CubicToFunc
def _cubic_to_c(c1, c2, to, ptr): (d:=ctypes.cast(ptr, ctypes.POINTER(ctypes.py_object)).contents.value).glyph_segments.append(NEMA_VG_PRIM_BEZIER_CUBIC); d.glyph_data.extend([float(c1.contents.x), float(c1.contents.y), float(c2.contents.x), float(c2.contents.y), float(to.contents.x), float(to.contents.y)]); return 0
outline_funcs = FT_Outline_Funcs(move_to=_move_to_c, line_to=_line_to_c, conic_to=_conic_to_c, cubic_to=_cubic_to_c)

class FontConverter:
    def __init__(self, font_path: str, ranges: Optional[List[Tuple[int, int]]], output_file: str):
        self.font_path, self.ranges, self.output_file = font_path, ranges, output_file
        try:
            self.face = freetype.Face(self.font_path)
            print(f"Successfully loaded font: {self.face.family_name.decode(errors='ignore')} {self.face.style_name.decode(errors='ignore')}")
        except freetype.ft_errors.FT_Exception as e:
            print(f"Error: Could not load font file '{font_path}'. Is it a valid font file?")
            print(f"Details: {e}")
            raise
        self.space_advance_width: Optional[float] = None

    def process_and_generate(self):
        print("Processing glyphs and preparing data...")
        
        codepoints_to_process = set()
        if self.ranges:
            print(f"Processing {len(self.ranges)} user-defined character range(s)...")
            for start, end in self.ranges: codepoints_to_process.update(range(start, end + 1))
        else:
            print("No ranges specified. Processing all available characters in the font...")
            codepoints_to_process = {code for code, index in self.face.get_chars() if index != 0}
        
        print(f"A total of {len(codepoints_to_process)} unique codepoints will be processed.")
        
        all_glyphs_map: Dict[int, GlyphInfo] = {}
        for codepoint in sorted(list(codepoints_to_process)):
            try:
                self.face.load_char(codepoint, freetype.FT_LOAD_NO_BITMAP | freetype.FT_LOAD_NO_SCALE)
            except freetype.ft_errors.FT_Exception: continue

            if codepoint == 0x20 and self.space_advance_width is None:
                self.space_advance_width = float(self.face.glyph.metrics.horiAdvance)
            
            decomposer = DecomposerData()
            
            # --- CRITICAL FIX: Check if the glyph actually has a vector outline ---
            if self.face.glyph.format == freetype.FT_GLYPH_FORMAT_OUTLINE:
                c_outline = self.face.glyph._FT_GlyphSlot.contents.outline
                c_outline_ptr = ctypes.byref(c_outline)
                
                if c_outline.n_points > 0: # Extra safety check
                    original_bbox = self.face.glyph.outline.get_bbox()
                    
                    user_data = ctypes.py_object(decomposer)
                    freetype.FT_Outline_Decompose(c_outline_ptr, ctypes.byref(outline_funcs), ctypes.pointer(user_data))
                else:
                    original_bbox = self.face.glyph.outline.get_bbox()
            else:
                # For non-outline glyphs (like space), just get the bbox.
                original_bbox = self.face.glyph.outline.get_bbox()
            # ----------------------------------------------------------------------

            # We add a glyph if it has drawing data OR if it's the special space character
            if decomposer.glyph_data or codepoint == 0x20:
                all_glyphs_map[codepoint] = GlyphInfo(
                    codepoint=codepoint, data=decomposer.glyph_data, segments=decomposer.glyph_segments,
                    x_advance=float(self.face.glyph.metrics.horiAdvance), bbox=original_bbox)
        
        sorted_glyphs = [all_glyphs_map[cp] for cp in sorted(all_glyphs_map.keys())]
        
        print(f"Writing {len(sorted_glyphs)} glyphs to binary file: {self.output_file}")
        with open(self.output_file, "wb") as f:
            header_format = '<IIffffIhhIII'
            f.write(b'\x00' * struct.calcsize(header_format))
            
            geometry_offsets = []
            for glyph in sorted_glyphs:
                geometry_offsets.append(f.tell())
                coord_count = len(glyph.data)
                coord_data_ints = [int(max(-32768, min(32767, round(val)))) for val in glyph.data]
                data_bytes = struct.pack(f'<{coord_count}h', *coord_data_ints)
                segment_bytes = struct.pack(f'<{len(glyph.segments)}B', *glyph.segments)
                f.write(struct.pack('<II', coord_count, len(segment_bytes)))
                f.write(data_bytes)
                f.write(segment_bytes)
            end_of_data_offset = f.tell()

            ult_offset = f.tell()
            f.write(struct.pack(f'<{len(sorted_glyphs)}I', *[g.codepoint for g in sorted_glyphs]))
            
            gmt_offset = f.tell()
            gmt_meta_format = '<fhhhhI'
            for i, glyph in enumerate(sorted_glyphs):
                f.write(struct.pack(gmt_meta_format, glyph.x_advance,
                                    glyph.bbox.xMin, glyph.bbox.yMin, glyph.bbox.xMax, glyph.bbox.yMax,
                                    geometry_offsets[i]))
            f.write(struct.pack(gmt_meta_format, 0.0, 0, 0, 0, 0, end_of_data_offset))
            
            f.seek(0)
            asc, desc = float(self.face.ascender), float(self.face.descender)
            x_adv = self.space_advance_width if self.space_advance_width is not None else float(self.face.max_advance_width)
            final_header = struct.pack(header_format,
                                       FILE_MAGIC, FILE_VERSION, asc - desc, x_adv, asc, desc,
                                       self.face.units_per_EM, self.face.underline_position,
                                       self.face.underline_thickness, len(sorted_glyphs),
                                       ult_offset, gmt_offset)
            f.write(final_header)

        print("Binary font file generation complete.")

def parse_ranges(range_str: str) -> List[Tuple[int, int]]:
    if not range_str: return []
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
        else: raise ValueError(f"Range '{part}' needs '0xSTART-0-END' format.")
    return ranges

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Python Font Converter to Custom Binary Format (v5).", formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('font_file', type=str, help='Path to the input TTF font file.')
    parser.add_argument('output_file', type=str, help='Path for the output binary font file.')
    parser.add_argument('--ranges', type=str, default=None, help="Optional. Comma-separated Unicode ranges.\nIf omitted, all characters from the font will be processed.")
    args = parser.parse_args()
    try:
        parsed_ranges = parse_ranges(args.ranges) if args.ranges else None
        converter = FontConverter(font_path=args.font_file, ranges=parsed_ranges, output_file=args.output_file)
        converter.process_and_generate()
    except Exception as e:
        print(f"An unexpected error occurred: {e}")