/*
 * @fidelity: verified   (layout only -- there is no code here)
 */

typedef struct {
    int numShaders;             /* 0x016CB848 */
    int shaders[4096];          /* 0x016CB84C */
    int sortedShaders[4096];    /* 0x016CF84C */
} tr_shaderRegistry_t;

tr_shaderRegistry_t tr_shaderRegistry;   /* 0x016CB848, 0x8004 bytes */

typedef char tr_sr_assert_shaders_off[
    ((int)(long)&((tr_shaderRegistry_t *)0)->shaders - 4) == 0 ? 1 : -1];
typedef char tr_sr_assert_sorted_off[
    ((int)(long)&((tr_shaderRegistry_t *)0)->sortedShaders - 0x4004) == 0 ? 1 : -1];
typedef char tr_sr_assert_size[
    sizeof(tr_shaderRegistry_t) == 0x8004 ? 1 : -1];
