#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import struct
from dataclasses import dataclass
from typing import List, Optional

# --- Constants and Structs for v5 (revised) ---
FILE_MAGIC_EXPECTED = 0x4E464F4E
FILE_VERSION_EXPECTED = 5

@dataclass
class FontHeaderV5:
    magic: int; version: int; size: float; xAdvance: float; ascender: float; descender: float
    units_per_em: int; underline_position: int; underline_thickness: int
    glyph_count: int; ult_offset: int; gmt_offset: int

@dataclass
class GlyphMetadata:
    xAdvance: float
    bbox_xmin: int; bbox_ymin: int; bbox_xmax: int; bbox_ymax: int
    geometry_offset: int

def read_and_unpack(f, fmt):
    size = struct.calcsize(fmt)
    data = f.read(size)
    if len(data) < size: raise IOError(f"Unexpected end of file.")
    return struct.unpack(fmt, data)

def binary_search_unicode(unicode_table: List[int], target_unicode: int) -> Optional[int]:
    low, high = 0, len(unicode_table) - 1
    while low <= high:
        mid = (low + high) // 2
        mid_val = unicode_table[mid]
        if mid_val == target_unicode: return mid
        elif mid_val < target_unicode: low = mid + 1
        else: high = mid - 1
    return None

def main():
    parser = argparse.ArgumentParser(description="Inspector for v5 (revised) custom binary font files.")
    parser.add_argument('font_bin_file', type=str, help='Path to the v5 binary font file.')
    parser.add_argument('--unicode', type=str, help='The Unicode codepoint to inspect (in hex).')
    parser.add_argument('--all', action='store_true', help='Print all tables in the file.')
    args = parser.parse_args()

    try:
        with open(args.font_bin_file, "rb") as f:
            print(f"--- Analyzing file: {args.font_bin_file} ---")
            
            header_format = '<IIffffIhhIII'
            header = FontHeaderV5(*read_and_unpack(f, header_format))

            if header.magic != FILE_MAGIC_EXPECTED or header.version != FILE_VERSION_EXPECTED:
                print(f"Error: Invalid file magic or version!")
                return

            print("\n[+] File Header (v5 revised):")
            for field, value in header.__dict__.items():
                print(f"  - {field:<28}: {value}")

            f.seek(header.ult_offset)
            unicode_table = list(read_and_unpack(f, f'<{header.glyph_count}I'))
            
            f.seek(header.gmt_offset)
            gmt_table: List[GlyphMetadata] = []
            gmt_meta_format = '<fhhhhI'
            for _ in range(header.glyph_count + 1):
                gmt_table.append(GlyphMetadata(*read_and_unpack(f, gmt_meta_format)))
            
            if args.all:
                print(f"\n[+] Unicode Lookup Table ({len(unicode_table)} entries, showing first 20):")
                for i, code in enumerate(unicode_table[:20]): print(f"  - Entry {i:<4}: Unicode=0x{code:04X}")
                
                print(f"\n[+] Glyph Metadata Table ({len(gmt_table)} entries, showing first 20):")
                for i, entry in enumerate(gmt_table[:20]):
                    print(f"  - Entry {i:<4}: BBox=({entry.bbox_xmin},{entry.bbox_ymin})-({entry.bbox_xmax},{entry.bbox_ymax}), Adv={entry.xAdvance:.2f}, GeoOffset={entry.geometry_offset}")

            if args.unicode:
                target_unicode = int(args.unicode, 16)
                print(f"\n--- Searching for Unicode 0x{target_unicode:X} ---")

                found_index = binary_search_unicode(unicode_table, target_unicode)
                if found_index is None:
                    print(f"Error: Unicode not found.")
                    return
                
                print(f"[+] Found at index {found_index} in ULT.")
                
                glyph_meta = gmt_table[found_index]
                next_glyph_meta = gmt_table[found_index + 1]
                
                geometry_offset = glyph_meta.geometry_offset
                geometry_total_size = next_glyph_meta.geometry_offset - geometry_offset
                
                print("\n[+] Glyph Metadata from GMT:")
                for field, value in glyph_meta.__dict__.items(): print(f"  - {field:<20}: {value}")
                print(f"  - {'Calculated Geo Size':<20}: {geometry_total_size} bytes")

                print("\n--- Reading and Parsing Glyph Geometry Data ---")
                f.seek(geometry_offset)
                
                data_len, seg_len = read_and_unpack(f, '<II')
                print(f"[+] Geometry Block Header: data_length={data_len}, segment_length={seg_len}")
                
                num_floats = data_len // 4
                coord_data = struct.unpack(f'<{num_floats}f', f.read(data_len))
                segment_data = struct.unpack(f'<{seg_len}B', f.read(seg_len))

                print("\n[+] Coordinate Data (first 24 values):")
                print("  ", coord_data[:24])
                print("\n[+] Segment Data (first 24 values):")
                print("  ", segment_data[:24])

    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == '__main__':
    # Simplified main guard
    main()