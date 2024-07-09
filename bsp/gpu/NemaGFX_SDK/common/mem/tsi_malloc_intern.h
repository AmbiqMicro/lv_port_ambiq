/* TSI 2023.xmo */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon Single Member PC be liable for any claim, damages or other
 *  liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings
 *  in the software.
 ******************************************************************************/

#ifndef TSI_MALLOC_INTERN_H__
#define TSI_MALLOC_INTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_MEM_POOLS
#define MAX_MEM_POOLS 8
#endif // MAX_MEM_POOLS

#define FLAG_EMPTY    0xf1fa1U
#define FLAG_NONEMPTY 0xf1fa2U

#define IS_LAST(c) ( (c)->next_offset == 0U )

#define DEFAULT_ALIGNMENT 16

typedef struct cell {
    int size;
    unsigned flags;
    uintptr_t next_offset;
} cell_t;

typedef struct pool {
    uintptr_t base_phys;
    uintptr_t base_virt;
    uintptr_t end_virt;
    cell_t   *head_of_empty_list;
    int size;
    int alignment;
} pool_t;

#ifdef UNIT_TEST

void *test_malloc(int pool, int size);
void test_free(void *ptr);
int test_get_allocated_bytes(int pool);

int free_entire_pool(int pool);
int validate_final_state(int pool);

#endif // UNIT_TEST


#ifdef __cplusplus
}
#endif

#endif
