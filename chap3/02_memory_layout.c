#include "common.h"

/*
 * 02_memory_layout
 *
 * Purpose:
 *   Print representative addresses for text, global, heap, and stack.
 *   Then pause so you can inspect /proc/<pid>/maps and pmap output.
 *
 * Demo steps:
 *   1) Run: ./02_memory_layout
 *   2) In another terminal:
 *        cat /proc/<pid>/maps | head -n 30
 *        pmap <pid> | head -n 30
 */
static int g_global = 42;

static void dummy_function(void) { }

int main(void) {
    print_ids("start");

    int stack_var = 7;
    int *heap_var = malloc(sizeof(int));
    if (!heap_var) die_errno("malloc");
    *heap_var = 99;

    info("Addresses (note: ASLR means these vary per run):");
    info("  text (main):          %p", (void*)&main);
    info("  text (dummy):         %p", (void*)&dummy_function);
    info("  global (g_global):    %p (value=%d)", (void*)&g_global, g_global);
    info("  heap (heap_var):      %p (value=%d)", (void*)heap_var, *heap_var);
    info("  stack (&stack_var):   %p (value=%d)", (void*)&stack_var, stack_var);

    info("Inspect mappings:");
    info("  cat /proc/%d/maps | head -n 30", (int)getpid());
    info("  pmap %d | head -n 30", (int)getpid());

    info("Keeping process alive for 45 seconds...");
    for (int i = 45; i >= 1; --i) {
        if (i % 5 == 0) info("  %d seconds remaining...", i);
        msleep(1000);
    }

    free(heap_var);
    info("done");
    return 0;
}
