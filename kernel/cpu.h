#ifndef _CPU_H
#define _CPU_H

#include "config.h"

#define FLAGS_INT_ENABLE (1 << 9)

struct MP_proc_entry
{
	uint8 type;
	uint8 lapic_id;
	uint8 lapic_ver;
	uint8 cpu_flags;
	uint8 cpu_sig[4];
	uint32 cpu_feat_flags;
};

struct MP_ioapic_entry
{	
	uint8 type;
	uint8 id;
	uint8 ver;
	uint8 enabled;
	uint32 addr;
};

struct MP_config
{
	uint8 signature[4];
	uint16 base_tab_len;
	uint8 spec_rev;
	uint8 checksum;
	uint8 oem_id[8];
	uint8 prod_id[12];
	uint32 oem_table_ptr;
	uint16 oem_table_size;
	uint16 entry_count;
	uint32 lapic_addr;
	uint16 ext_tab_len;
	uint8 ext_tab_checksum;
};

struct MP_float_ptr
{
	uint8 signature[4];
	struct MP_config *config;
	uint8 length;
	uint8 version;
	uint8 checksum;
	uint8 features[2];
	uint8 reserved[3];
};


struct MP_float_ptr *mp_find();


void cpu_find();
void cpu_wake_all();

int cpu_count();
uint32 cpuid(void);
uint32 esp(void);

#endif
