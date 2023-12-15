/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/*
 * Maximum number of memory regions to accept from the multiboot information
 * structure. This is arbitrary limit and exceeding it will throw an error.
 */
#define MAX_MEMORY_REGIONS      16

/*
 * Maximum number of I/O APICs to register. This is set to 1 to match seL4. All
 * other units will be ignored (with a warning).
 */
#define MAX_NUM_IOAPICS         1

/*
 * Maximum number of DMA Remapping Hardware units. This is arbitrary limit and
 * exceeding it will throw an error.
 */
#define MAX_NUM_DRHUS           16

/*
 * Maximum number of Reserved memory Region Reporting structures. This is
 * arbitrary limit and exceeding it will throw an error.
 */
#define MAX_NUM_RMRRS           16
