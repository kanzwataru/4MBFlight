// migrated from FSNN, change C99 -> C++
// TODO: Should "pre-app" settings be defined in a cleaner way somehow???
#pragma once
//#include "macros.h"
#include "common.h"

enum ShaderStage {
	SS_Fragment,
    SS_Vertex,

	SS_Total
};

struct ShaderStageSrc {
	uint32_t size;		/* if zero, stage is not present */
	uint32_t offset;
};

struct ShaderEntry {
	struct ShaderStageSrc stages[SS_Total];
};

/* ---- "per-app" config ---- */
enum ShaderPrograms {
	SP_VertCol,
    SP_Grid,
    SP_Lit,
    SP_Sky,

	SP_Total
};

struct ShaderStorageHeader {
	uint32_t total_size_in_bytes;
	uint32_t shader_entry_count;
	struct ShaderEntry toc[SP_Total];	/* hard-coded count, based on above enum */

	/* beyond this lie the shader sources... */
};

#if WITH_SHADER_COMPILER
static const char *g_shader_paths[SP_Total][SS_Total] = {
	[SP_VertCol] = {
		[SS_Fragment] 	= "vertcol_frag.glsl",
		[SS_Vertex]		= "vertcol_vert.glsl"
	},
    [SP_Grid] = {
        [SS_Fragment]   = "grid_frag.glsl",
        [SS_Vertex]     = "grid_vert.glsl"
    },
    [SP_Lit] = {
        [SS_Fragment]   = "lit_frag.glsl",
        [SS_Vertex]     = "lit_vert.glsl"
    },
    [SP_Sky] = {
        [SS_Fragment]   = "quad_sky_frag.glsl",
        [SS_Vertex]     = "quad_sky_vert.glsl",
    }
};
#endif
