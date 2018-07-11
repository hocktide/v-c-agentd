#include <stdio.h>
#include <vpr/allocator/malloc_allocator.h>

int main(int argc, char* argv[])
{
    allocator_options_t alloc_opts;
    malloc_allocator_options_init(&alloc_opts);
    dispose((disposable_t*)&alloc_opts);
    return 0;
}
