/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "acpi.h"
#include "multiboot2.h"
#include "serial.h"
#include "sysinfo.h"
#include "utils.h"
#include "vtd.h"

/* Global system information structure. */
struct sysinfo sysinfo;

/* Global program options, with default values. */
struct options options = {
	.serial_port = CONFIG_SERIAL_PORT,
	.on_exit     = OPTION_ON_EXIT_HANG,
};

/*
 * Output the machine file in json format over the serial line.
 */
static void dump_machine_file(void)
{
	/* Opening tag. */
	serial_printf("\n"
	              "-----BEGIN MACHINE FILE BLOCK-----\n"
	              "{\n");

	/* Memory regions. */
	serial_printf("    \"memory\": [\n");
	for (int i = 0; i < sysinfo.memory.count; i++) {
		serial_printf("        {\n");
		serial_printf("            \"base\": %d,\n", sysinfo.memory.list[i].addr);
		serial_printf("            \"size\": %d\n",  sysinfo.memory.list[i].size);
		serial_printf("        }%s\n", i + 1 == sysinfo.memory.count ? "" : ",");
	}
	serial_printf("    ],\n");

	/* Kernel devices. */
	serial_printf("    \"kdevs\": [\n");
	serial_printf("        {\n");
	serial_printf("            \"name\": \"apic\",\n");
	serial_printf("            \"base\": %d,\n", sysinfo.apic.addr);
	serial_printf("            \"size\": 4096\n");
	serial_printf("        }");
	for (int i = 0; i < sysinfo.ioapic.count; i++) {
		serial_printf(",\n"
		              "        {\n");
		serial_printf("            \"name\": \"ioapic.%d\",\n", (uint64_t) i);
		serial_printf("            \"base\": %d,\n", sysinfo.ioapic.addr[i]);
		serial_printf("            \"size\": 4096\n");
		serial_printf("        }");
	}
	for (int i = 0; i < sysinfo.drhu.count; i++) {
		serial_printf(",\n"
		              "        {\n");
		serial_printf("            \"name\": \"drhu.%d\",\n", (uint64_t) i);
		serial_printf("            \"base\": %d,\n", sysinfo.drhu.addr[i]);
		serial_printf("            \"size\": 4096\n");
		serial_printf("        }");
	}
	serial_printf("\n    ],\n");

	/* RMRRs. */
	serial_printf("    \"rmrrs\": [\n");
	if (sysinfo.rmrr.count > 0) {
		for (int i = 0; i < sysinfo.rmrr.count; i++) {
			serial_printf("        {\n");
			serial_printf("            \"devid\": %d,\n", sysinfo.rmrr.list[i].devid);
			serial_printf("            \"base\":  %d,\n", sysinfo.rmrr.list[i].addr);
			serial_printf("            \"limit\": %d\n",  sysinfo.rmrr.list[i].limit);
			serial_printf("        }%s\n", i + 1 == sysinfo.rmrr.count ? "" : ",");
		}
	}
	serial_printf("    ],\n");

	/* Bootinfo. */
	serial_printf("    \"bootinfo\": {\n");
	serial_printf("        \"numIOPTLevels\": %d\n", sysinfo.vtd.num_iopt_levels);
	serial_printf("    }\n");

	/* Closing tag. */
	serial_printf("}\n");
	serial_printf("-----END MACHINE FILE BLOCK-----\n");
}

/*
 * Discover the hardware and print the machine file.
 */
int main(uint32_t multiboot_magic, uint32_t multiboot_info)
{
	memset((char *) &sysinfo, 0, sizeof (sysinfo));
	options.serial_port = CONFIG_SERIAL_PORT;

	/* Initialise the default serial port to have some early output. */
	serial_init();

	/* Parse the multiboot info structure. */
	if (multiboot_magic == MULTIBOOT2_BOOT_MAGIC) {
		serial_puts("[*] Multiboot2 boot loader detected\n");
		if (multiboot2_parse_info(multiboot_info) == false)
			return options.on_exit;
	} else {
		serial_puts("[X] Error: unsupported boot loader (not multiboot2 compliant)!\n");
		return options.on_exit;
	}

	/* Parse the ACPI tables. */
	if (acpi_parse_tables() == false)
		return options.on_exit;

	/* Look up the number of VT-D IOPT levels. */
	if (vtd_scan() == false)
		return options.on_exit;

	/* Output the json file. */
	dump_machine_file();

	return options.on_exit;
}
