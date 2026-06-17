#ifndef AM_MEM_H
#define AM_MEM_H

#include "am_mcu_apollo.h"
#include "tlsf.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    SemaphoreHandle_t mutex;
    tlsf_t tlsf;
    size_t cur_used;
    size_t max_used;
    size_t pool_size;
    uint32_t start_addr;
    bool cacheable;
} am_mem_control_t;

typedef struct {
    size_t total_size;  /**< Total heap size */
    size_t free_cnt;
    size_t free_size;   /**< Size of available memory */
    size_t free_biggest_size;
    size_t used_cnt;
    size_t max_used;    /**< Max size of Heap memory used */
    uint8_t used_pct;   /**< Percentage used */
    uint8_t frag_pct;   /**< Amount of fragmentation */
} am_mem_monitor_t;

extern am_mem_control_t dtcm_heap;
extern am_mem_control_t ssram_heap;
extern am_mem_control_t psram_heap;



extern void am_mem_heap_init(void);
extern void* am_mem_heap_malloc(am_mem_control_t *heap, size_t size);
extern void* am_mem_heap_malloc_align(am_mem_control_t *heap, size_t size, size_t align);
extern void* am_mem_heap_realloc(am_mem_control_t *heap, void * p, size_t new_size);
extern void am_mem_heap_free(am_mem_control_t *heap, void *p);
extern void am_mem_heap_monitor(am_mem_control_t *heap, am_mem_monitor_t * mon_p);

#define am_mem_dtcm_malloc(size) am_mem_heap_malloc(&dtcm_heap, size)
#define am_mem_dtcm_malloc_align(size, align) am_mem_heap_malloc_align(&dtcm_heap, size, align)
#define am_mem_dtcm_free(p) am_mem_heap_free(&dtcm_heap, p)

#define am_mem_ssram_malloc(size) am_mem_heap_malloc(&ssram_heap, size)
#define am_mem_ssram_malloc_align(size, align) am_mem_heap_malloc_align(&ssram_heap, size, align)
#define am_mem_ssram_free(p) am_mem_heap_free(&ssram_heap, p)

#define am_mem_psram_malloc(size) am_mem_heap_malloc(&psram_heap, size)
#define am_mem_psram_malloc_align(size, align) am_mem_heap_malloc_align(&psram_heap, size, align)
#define am_mem_psram_free(p) am_mem_heap_free(&psram_heap, p)

#define am_mem_dtcm_monitor(mon_p) am_mem_heap_monitor(&dtcm_heap, mon_p)
#define am_mem_ssram_monitor(mon_p) am_mem_heap_monitor(&ssram_heap, mon_p)
#define am_mem_psram_monitor(mon_p) am_mem_heap_monitor(&psram_heap, mon_p)

#endif /* AM_MEM_H */