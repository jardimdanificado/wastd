#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* 
 * Um alocador simples (First-Fit, Coalescência Imediata) para WebAssembly.
 * Memória layout: [header (8b)] [payload] [footer (4b)]
 * Alinhamento: 8 bytes.
 */

/* Entry point do heap (vindo do linker ou definido aqui) */
extern char __heap_base;

#define WASM_PAGE_SIZE 65536

uint32_t heap_pointer = __heap_base;

static inline uint32_t wasm_memory_size(void) {
    return __builtin_wasm_memory_size(0);
}

static inline uint32_t wasm_memory_grow(uint32_t pages) {
    return __builtin_wasm_memory_grow(0, pages);
}


static inline uint32_t palloc(uint32_t size)
{
    uint32_t result = heap_pointer;
    if(heap_pointer + size >= __heap_base + (wasm_memory_size() * WASM_PAGE_SIZE))
    {
        int page_count = 0;
        while(heap_pointer + (size*(page_count+1)) >= __heap_base + (wasm_memory_size() * WASM_PAGE_SIZE))
            page_count++;
        wasm_memory_grow(page_count);
    }
    heap_pointer += size;
    return result;
}