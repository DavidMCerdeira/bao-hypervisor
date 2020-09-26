#include <config.h>
#include <mem.h>
/**
 * Declare VM images.
 */

VM_IMAGE(vm_enclave, "/home/david/PhD/Research/MSCs_nuno/enclave/bao_guest/build/rock960/bao_bare-metal_guest.bin");

#define VM_ENCLV_BASE (VM_AOSP_LIMIT)
#define VM_ENCLV_ENTRY (VM_ENCLV_BASE + 0x4d8)
#define VM_ENCLV_SIZE (VM_ENCLV_LIMIT - VM_ENCLV_BASE - VM_ENCLV_SHMEM_SIZE)
#define VM_ENCLV_LIMIT (VM_ENCLV_BASE + 0x0300000)
#define VM_ENCLV_SHMEM_BASE (VM_ENCLV_LIMIT - VM_ENCLV_SHMEM_SIZE)
#define VM_ENCLV_SHMEM_SIZE (VM_AOSP_SHMEM_SIZE)
#define VM_ENCLV_SHMEM_IRQ (184)

VM_IMAGE(vm_aosp, "/home/david/PhD/Research/MSCs_nuno/u-boot.bin");

#define VM_AOSP_BASE (0x0)
#define VM_AOSP_SIZE (VM_AOSP_LIMIT - VM_AOSP_SHMEM_SIZE)
#define VM_AOSP_LIMIT (0x7B000000)
#define VM_AOSP_SHMEM_BASE (VM_AOSP_LIMIT - VM_AOSP_SHMEM_SIZE)
#define VM_AOSP_SHMEM_SIZE (0x00006000)
#define VM_AOSP_IMG_BASE (0x200000)
#define VM_AOSP_SHMEM_IRQ (184)

#undef SHMEM

/**
 * Config each VM.
 */

shmem_t shmem = {
    .ppages = {.size = 0x00006000},
};

struct config config = {

    CONFIG_HEADER

        .vmlist_size = 1,
    .vmlist =
        {
            {
                .image =
                    {
                        .base_addr = VM_ENCLV_BASE,
                        .load_addr = VM_IMAGE_OFFSET(vm_enclave),
                        .size = VM_IMAGE_SIZE(vm_enclave),
                    },

                .entry = VM_ENCLV_ENTRY,
                .cpu_affinity = 0b1000,

                .platform =
                    {
                        .cpu_num = 1,

#ifdef SHMEM
                        .region_num = 2,
#else
                        .region_num = 1,
#endif
                        .regions =
                            (struct mem_region[]){
                                {
                                    .base = VM_ENCLV_BASE,
                                    .size = VM_ENCLV_SIZE, /* 2MB */
                                },
#ifdef SHMEM
                                {
                                    .base = VM_ENCLV_SHMEM_BASE,
                                    .size = VM_ENCLV_SHMEM_SIZE,
                                    .shared = &shmem,
                                },
#endif
                            },

                        .dev_num = 2,
                        .devs =
                            (struct dev_region[]){
                                {
                                    /* UART2 */
                                    .pa = 0xFF1A0000,
                                    .va = 0xFF1A0000,
                                    .size = 0x1000,
                                },
                                {
                                    /* Arch timer interrupt */
                                    .interrupt_num = 1,
                                    .interrupts = (uint64_t[]){27},
                                },
                            },

#ifdef SHMEM
                        .ipc_num = 1,
                        .ipc_obj_list =
                            (ipc_obj_config_t[]){
                                {
                                    .ipc_obj =
                                        {
                                            .id = 1,
                                            .shmem = &shmem,
                                        },
                                    .ipc_if =
                                        {
                                            .irq_num = VM_ENCLV_SHMEM_IRQ,
                                        },
                                },
                            },
#endif

			.console = {
			    .base = 0xFF1A0000,
			},

                        .arch =
                            {
                                .gic =
                                    {
                                        .gicd_addr = 0xFEE00000,
                                        .gicr_addr = 0xFEF00000,
                                        .extra_irq = 2, /* extra irq for shmem */
                                    },
				
                            },
                    },
            },
            {
                .image =
                    {
                        .base_addr = VM_AOSP_IMG_BASE,
                        .load_addr = VM_IMAGE_OFFSET(vm_aosp),
                        .size = VM_IMAGE_SIZE(vm_aosp),
                    },

                .entry = VM_AOSP_IMG_BASE,
                .cpu_affinity = 0b111,

                .platform =
                    {
                        .cpu_num = 3,

#ifdef SHMEM
                        .region_num = 2,
#else
                        .region_num = 1,
#endif
                        .regions =
                            (struct mem_region[]){
                                {
                                    .base = VM_AOSP_BASE,
                                    .phys = VM_AOSP_BASE,
                                    .size = VM_AOSP_SIZE,
                                    .place_phys = 1,
                                },
#ifdef SHMEM
                                {
                                    .base = VM_AOSP_SHMEM_BASE,
                                    .phys = VM_AOSP_SHMEM_BASE,
                                    .size = VM_AOSP_SHMEM_SIZE,
                                    .place_phys = 1,
				    .shared = &shmem,
                                }
#endif
			    },

                        .dev_num = 2,
                        .devs =
                            (struct dev_region[]){
                                {
                                    .pa = 0xF8000000,
                                    .va = 0xF8000000,
                                    .size = 0xFEE00000 - 0xF8000000,
                                    .interrupt_num = 1 + 181 - 32,
                                    .interrupts =
                                        (uint64_t[]){
                                            27,  32,  33,  34,  35,  36,  37,
                                            38,  39,  40,  41,  42,  43,  44,
                                            45,  46,  47,  48,  49,  50,  51,
                                            52,  53,  54,  55,  56,  57,  58,
                                            59,  60,  61,  62,  63,  64,  65,
                                            66,  67,  68,  69,  70,  71,  72,
                                            73,  74,  75,  76,  77,  78,  79,
                                            80,  81,  82,  83,  84,  85,  86,
                                            87,  88,  89,  90,  91,  92,  93,
                                            94,  95,  96,  97,  98,  99,  100,
                                            101, 102, 103, 104, 105, 106, 107,
                                            108, 109, 110, 111, 112, 113, 114,
                                            115, 116, 117, 118, 119, 120, 121,
                                            122, 123, 124, 125, 126, 127, 128,
                                            129, 130, 131, 132, 133, 134, 135,
                                            136, 137, 138, 139, 140, 141, 142,
                                            143, 144, 145, 146, 147, 148, 149,
                                            150, 151, 152, 153, 154, 155, 156,
                                            157, 158, 159, 160, 161, 162, 163,
                                            164, 165, 166, 167, 168, 169, 170,
                                            171, 172, 173, 174, 175, 176, 177,
                                            178, 179, 180, 181},
                                },
                                /* hole for gic */
                                {
                                    .pa = 0xFF000000,
                                    .va = 0xFF000000,
                                    .size = 0xFFFFFFFF - 0xFF000000,
                                },
                            },

#ifdef SHMEM
                        .ipc_num = 1,
                        .ipc_obj_list =
                            (ipc_obj_config_t[]){
                                {
                                    .ipc_obj =
                                        {
                                            .id = 1,
                                            .shmem = &shmem,
                                        },
                                    .ipc_if =
                                        {
                                            .irq_num = VM_AOSP_SHMEM_IRQ,
                                        },
                                },
                            },
#endif

                        .arch =
                            {
                                .gic =
                                    {
                                        .gicd_addr = 0xFEE00000,
                                        .gicr_addr = 0xFEF00000,
                                        /* extra irq for shmem */
                                        .extra_irq = 2,
                                    },
                            },
                    },
            },
        },
};

