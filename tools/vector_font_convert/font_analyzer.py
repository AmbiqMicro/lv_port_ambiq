#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import freetype
from typing import List, Tuple

# A dictionary containing names for common Unicode ranges for cosmetic output.
# Data source: Wikipedia, Unicode Standard
UNICODE_RANGE_NAMES = {
    (0x0020, 0x007E): "Basic Latin (ASCII)",
    (0x00A0, 0x00FF): "Latin-1 Supplement",
    (0x0100, 0x017F): "Latin Extended-A",
    (0x0180, 0x024F): "Latin Extended-B",
    (0x0250, 0x02AF): "IPA Extensions",
    (0x0370, 0x03FF): "Greek and Coptic",
    (0x0400, 0x04FF): "Cyrillic",
    (0x0500, 0x052F): "Cyrillic Supplement",
    (0x2000, 0x206F): "General Punctuation",
    (0x20A0, 0x20CF): "Currency Symbols",
    (0x2190, 0x21FF): "Arrows",
    (0x2200, 0x22FF): "Mathematical Operators",
    (0x2500, 0x257F): "Box Drawing",
    (0x2580, 0x259F): "Block Elements",
    (0x2E80, 0x2EFF): "CJK Radicals Supplement",
    (0x3000, 0x303F): "CJK Symbols and Punctuation",
    (0x4E00, 0x9FFF): "CJK Unified Ideographs",
    (0xAC00, 0xD7AF): "Hangul Syllables",
    (0xF900, 0xFAFF): "CJK Compatibility Ideographs",
    (0xFF00, 0xFFEF): "Halfwidth and Fullwidth Forms",
}

def get_range_name(start: int, end: int) -> str:
    """
    Attempts to name a Unicode range based on its start and end codepoints.
    """
    # Check for an exact match
    if (start, end) in UNICODE_RANGE_NAMES:
        return UNICODE_RANGE_NAMES[(start, end)]
    # Check for a partial match (if the range is a subset of a known block)
    for (r_start, r_end), name in UNICODE_RANGE_NAMES.items():
        if r_start <= start and end <= r_end:
            return f"{name} (subset)"
    return "Custom or Unnamed Range"

def get_codepoints_from_font(font_path: str) -> List[int]:
    """
    Extracts all supported Unicode codepoints from a font file.
    """
    try:
        face = freetype.Face(font_path)
        print(f"Successfully loaded font: {face.family_name.decode()} {face.style_name.decode()}")
    except freetype.ft_errors.FT_Exception as e:
        print(f"Error: Could not load or process font file '{font_path}'.")
        print(f"Details: {e}")
        return []

    # face.get_chars() is an efficient method to get all (codepoint, glyph_index)
    # pairs defined in the font's character map (cmap).
    chars = face.get_chars()
    codepoints = [code for code, index in chars]
    return codepoints

def consolidate_ranges(codepoints: List[int]) -> List[Tuple[int, int]]:
    """
    Consolidates a sorted list of codepoints into continuous ranges.
    """
    if not codepoints:
        return []

    # Ensure the list is sorted and unique
    sorted_points = sorted(list(set(codepoints)))
    
    ranges = []
    range_start = sorted_points[0]

    for i in range(1, len(sorted_points)):
        # If the current codepoint is not the previous one + 1, the range is broken.
        if sorted_points[i] != sorted_points[i-1] + 1:
            # Record the previous range
            range_end = sorted_points[i-1]
            ranges.append((range_start, range_end))
            # Start a new range
            range_start = sorted_points[i]
    
    # Don't forget to add the last range after the loop finishes
    ranges.append((range_start, sorted_points[-1]))
    
    return ranges

def main():
    parser = argparse.ArgumentParser(
        description="Analyzes a TTF font file to find all supported Unicode character ranges.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument('font_file', type=str, help='Path to the input TTF font file.')
    args = parser.parse_args()

    codepoints = get_codepoints_from_font(args.font_file)
    
    if not codepoints:
        print("No character mappings (cmaps) found or font could not be read.")
        return

    print(f"\nFound {len(codepoints)} individual character mappings in the font.")
    
    ranges = consolidate_ranges(codepoints)
    
    print("\nConsolidated Unicode Ranges:")
    print("-" * 60)
    print(f"{'Start':<10} {'End':<10} {'Count':<10} {'Range Name'}")
    print(f"{'='*8:<10} {'='*8:<10} {'='*8:<10} {'='*30}")

    total_chars_in_ranges = 0
    for start, end in ranges:
        count = end - start + 1
        total_chars_in_ranges += count
        range_name = get_range_name(start, end)
        print(f"0x{start:04X}     0x{end:04X}     {count:<10} {range_name}")

    print("-" * 60)
    print(f"Total characters covered by these {len(ranges)} ranges: {total_chars_in_ranges}")


if __name__ == '__main__':
    main()