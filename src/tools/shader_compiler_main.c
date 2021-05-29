#define WITH_SHADER_COMPILER 1

#include "common.h"
#include "shaders/shader_toc.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static const char version_string_arr[] = "#version 330 core\n";

static char buffer[MEGABYTES(16)];
static const size_t max_size = sizeof(buffer);
static size_t top = 0;

int main(int argc, char **argv)
{
	if(argc != 3) {
		fprintf(stderr, "shader_compiler [path to shaders] [shader pak output]");
		return 1;
	}

	const char *shader_base_path = argv[1];
	const char *shader_pak_path = argv[2];

	struct ShaderStorageHeader *header = (void *)buffer;
	top += sizeof(*header);

	const size_t version_string_size = sizeof(version_string_arr) - 1;

	for(int i = 0; i < SP_Total; ++i) {
		struct ShaderEntry *entry = &header->toc[i];

		for(int j = 0; j < SS_Total; ++j) {
			const char *short_path = g_shader_paths[i][j];
			if(!short_path)
				continue;

			char path[1024];
			snprintf(path, sizeof(path), "%s/%s", shader_base_path, short_path);

			FILE *fp = fopen(path, "r");
			if(!fp) {
				fprintf(stderr, "Could not open shader: %s\n", path);
				return 1;
			}

			fseek(fp, 0, SEEK_END);
			size_t size = ftell(fp);
			rewind(fp);

			if(size == 0) {
				fprintf(stderr, "File is empty: %s\n", path);
				return 1;
			}

			assert(top + size < max_size);
			entry->stages[j].size = size + version_string_size;
			entry->stages[j].offset = top;

			memcpy(&buffer[top], version_string_arr, version_string_size);
			top += version_string_size;

			size_t bytes_read = fread(&buffer[top], 1, size, fp);
			assert(bytes_read == size);
			top += bytes_read + 1;

			fclose(fp);

			printf("\t-> %s\n", short_path);
		}
	}

	header->total_size_in_bytes = top;
	header->shader_entry_count = SP_Total;

	FILE *fp = fopen(shader_pak_path, "wb");
	assert(fp);
	size_t bytes_written = fwrite(buffer, 1, top, fp);
	assert(bytes_written == top);

	return 0;
}
