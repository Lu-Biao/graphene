/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* Copyright (C) 2014 Stony Brook University
   This file is part of Graphene Library OS.

   Graphene Library OS is free software: you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   Graphene Library OS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/*
 * shim_sched.c
 *
 * Implementation of system call "sched_yield".
 */

#include <shim_internal.h>
#include <shim_table.h>
#include <api.h>
#include <pal.h>

#include <errno.h>

int shim_do_sched_yield (void)
{
    DkThreadYieldExecution();
    return 0;
}


/* Size definition for CPU sets.  */
# define __OCALL_CPU_SETSIZE	1024
# define __OCALL_NCPUBITS	    (8 * sizeof (__ocall_cpu_mask))

/* Type for array elements in 'cpu_set_t'.  */
typedef unsigned long int __ocall_cpu_mask;

/* Basic access functions.  */
# define __OCALL_CPUELT(cpu) ((cpu) / __OCALL_NCPUBITS)
# define __OCALL_CPUMASK(cpu) \
    ((__ocall_cpu_mask) 1 << ((cpu) % __OCALL_NCPUBITS))

# define __OCALL_CPU_ISSET_S(cpu, setsize, cpusetp) \
    (__extension__ \
     ({ size_t __cpu = (cpu); \
      __cpu < 8 * (setsize) \
      ? ((((const __ocall_cpu_mask *) ((cpusetp)->__bits))[__OCALL_CPUELT (__cpu)] \
          & __OCALL_CPUMASK (__cpu))) != 0 \
      : 0; }))
# define __OCALL_CPU_ISSET(cpu, cpusetp) \
    __OCALL_CPU_ISSET_S (cpu, sizeof (ms_ocall_cpu_set_t), cpusetp)

/* Data structure to describe CPU mask.  */
typedef struct
{
  __ocall_cpu_mask __bits[__OCALL_CPU_SETSIZE / __OCALL_NCPUBITS];
} ms_ocall_cpu_set_t;


int shim_do_sched_getaffinity (pid_t pid, size_t len,
                               __kernel_cpu_set_t * user_mask_ptr)
{
#if 0
    int ncpus = PAL_CB(cpu_info.cpu_num);
    // Check that user_mask_ptr is valid; if not, should return -EFAULT
    if (test_user_memory(user_mask_ptr, len, 1))
        return -EFAULT;
    memset(user_mask_ptr, 0, len);
    for (int i = 0 ; i < ncpus ; i++)
        ((uint8_t *) user_mask_ptr)[i / 8] |= 1 << (i % 8);
    return ncpus;
#endif
    int retval;
    retval = DkThreadGetAffinity(pid, len, user_mask_ptr);

    int i;
    debug("LUB shim_do_sched_getaffinity user_mask_ptr=%p, retval=%d, pid=%d, mask=",
           user_mask_ptr, retval, pid);
    for (i = 0; i < 4; i++) {
        debug("%d ", __OCALL_CPU_ISSET(i, (ms_ocall_cpu_set_t *)user_mask_ptr));
    }
    debug("\n");

    return retval;

}

int shim_do_sched_setaffinity (pid_t pid, size_t len,
                               __kernel_cpu_set_t * user_mask_ptr)
{
    return DkThreadSetAffinity(pid, len, user_mask_ptr);
}
