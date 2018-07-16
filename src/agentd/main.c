/**
 * \file agentd/main.c
 *
 * \brief Main entry point for the Velo Blockchain Agent.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

int main(int UNUSED(argc), char** UNUSED(argv))
{
    allocator_options_t alloc_opts;
    malloc_allocator_options_init(&alloc_opts);
    dispose((disposable_t*)&alloc_opts);
    return 0;
}
