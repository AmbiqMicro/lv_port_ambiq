//*****************************************************************************
//
//! @file xml_assets.c
//!
//! @brief The demo's .xml files, linked into the image.
//!
//! Each file under ../xml is pulled in verbatim with .incbin and followed by
//! a NUL, so it can be passed to lv_xml_register_component_from_data() as a
//! C string. The path is relative to the board directory make runs in.
//! demo_common.mk makes this object depend on the .xml files, so editing
//! one rebuilds it.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "xml_assets.h"

#define XML_ASSET(sym, path)                                \
    __asm__(".section .rodata." #sym ", \"a\"\n"            \
            ".global " #sym "\n"                            \
            #sym ":\n"                                      \
            ".incbin \"" path "\"\n"                        \
            ".byte 0\n"                                     \
            ".previous\n")

XML_ASSET(g_pcGridCellXml, "../xml/grid_cell.xml");
XML_ASSET(g_pcGridViewXml, "../xml/grid_view.xml");
