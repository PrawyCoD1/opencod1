/*
 * @fidelity: verified   (layouts only -- there is no code in this file)
 */

#ifndef FX_TYPES_H
#define FX_TYPES_H

#define FX_EFFECT_TEMPLATE_NAME_CAPACITY       64
#define FX_EFFECT_TEMPLATE_PRIMITIVE_CAPACITY  24
#define FX_EFFECT_TEMPLATE_SIZE                0xA8

#define FX_EFFECT_TEMPLATE_COUNT               128

#define FX_EFFECT_ID_NONE                      0
#define FX_EFFECT_ID_FIRST                     1

#define FX_SCHEDULER_TEMPLATES_OFFSET          0x0000
#define FX_SCHEDULER_EFFECT_IDS_MAP_OFFSET     0x5400
#define FX_SCHEDULER_SCHEDULED_LIST_OFFSET     0x540C
#define FX_SCHEDULER_SCHEDULED_HEAD_OFFSET     0x5410
#define FX_SCHEDULER_SCHEDULED_SIZE_OFFSET     0x5414
#define FX_SCHEDULER_SIZE                      0x5418

#define FX_SCHEDULED_EFFECT_SIZE               0x44

#define FX_PRIMITIVE_TEMPLATE_SIZE   0x25C

#define FX_PRIMITIVE_TEMPLATE_RANGE_SIZE       0x08

#define FX_PRIMITIVE_TEMPLATE_NAME_OFFSET      0x08
#define FX_PRIMITIVE_TEMPLATE_DELAY_OFFSET     0x4C
#define FX_PRIMITIVE_TEMPLATE_COUNT_OFFSET     0x54
#define FX_PRIMITIVE_TEMPLATE_CULLRADIUS_OFFSET 0x64
#define FX_PRIMITIVE_TEMPLATE_FLAGS_OFFSET     0xB8   /* int              */
#define FX_PRIMITIVE_TEMPLATE_SPAWNFLAGS_OFFSET 0xBC  /* int, bit 0x2000  */
#define FX_PRIMITIVE_TEMPLATE_VECTOR_STRIDE    0x10

#define FX_HELPER_BASE   ((unsigned char *)&theFxHelper)

#undef  fxMTime
#define fxMTime   (*(int   *)(FX_HELPER_BASE + 0x04))
#undef  dword_1407510
#define dword_1407510   (*(int   *)(FX_HELPER_BASE + 0x08))
#undef  dword_1407514
#define dword_1407514   (*(int   *)(FX_HELPER_BASE + 0x0C))
#undef  dword_1407518
#define dword_1407518   (*(int   *)(FX_HELPER_BASE + 0x10))
#undef  flt_140751C
#define flt_140751C     (*(float *)(FX_HELPER_BASE + 0x14))
#undef  flt_1407520
#define flt_1407520     (*(float *)(FX_HELPER_BASE + 0x18))
#undef  flt_1407524
#define flt_1407524     (*(float *)(FX_HELPER_BASE + 0x1C))
#undef  flt_1407528
#define flt_1407528     (*(float *)(FX_HELPER_BASE + 0x20))
#undef  flt_140752C
#define flt_140752C     (*(float *)(FX_HELPER_BASE + 0x24))
#undef  flt_1407530
#define flt_1407530     (*(float *)(FX_HELPER_BASE + 0x28))
#undef  dword_1407588
#define dword_1407588   (*(int   *)(FX_HELPER_BASE + 0x80))

#define FX_ACTIVE_SLOT_SIZE   12
#define FX_ACTIVE_MAX         1800
#define FX_ACTIVE_BASE        ((unsigned char *)&fxActiveEffects)
#define FX_ACTIVE_BYTES       ( FX_ACTIVE_SLOT_SIZE * FX_ACTIVE_MAX )

#undef  byte_CA22C0
#define byte_CA22C0     ( FX_ACTIVE_BASE + FX_ACTIVE_BYTES )
#undef  dword_C9CE68
#define dword_C9CE68    (*(int      *)(FX_ACTIVE_BASE + 0x0008))
#undef  unk_C9CE6C
#define unk_C9CE6C      (*(unsigned int *)(FX_ACTIVE_BASE + 0x000C))
#undef  dword_CA22BC
#define dword_CA22BC    (*(int      *)(FX_ACTIVE_BASE + 0x545C))

#endif
