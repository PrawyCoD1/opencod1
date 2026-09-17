/*
 * @fidelity: verified
 */
#ifndef TR_SHADERREGISTRY_H
#define TR_SHADERREGISTRY_H

typedef struct {
    int numShaders;             /* 0x016CB848            */
    int shaders[4096];          /* 0x016CB84C  shader_t* */
    int sortedShaders[4096];    /* 0x016CF84C  shader_t* */
} tr_shaderRegistry_t;

extern tr_shaderRegistry_t tr_shaderRegistry;

#define tr_numShaders   ((int *)(void *)&tr_shaderRegistry)
#define tr_shaders   (tr_shaderRegistry.shaders)
#define dword_16CF84C   (tr_shaderRegistry.sortedShaders)
#define dword_16CF850   (tr_shaderRegistry.sortedShaders + 1)

#endif
