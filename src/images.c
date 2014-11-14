#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include "images.h"
#include "list.h"

/**
 * @return: List* of char*
 */
List*
load_images(char* dirname)
{
	List* images = NULL;
	DIR* dir;
	struct dirent *entry;
	char* filename;

	dir = opendir(dirname);

	while ((entry = readdir(dir)))
	{
		/* Hidden files ignored. Just because. */
		if (entry->d_name[0] == '.')
			continue;

		filename = (char*) malloc(sizeof(char) * (
			strlen(dirname) + strlen(entry->d_name) + 2
		));;
		sprintf(filename, "%s/%s", dirname, entry->d_name);

		printf(" > %s\n", filename);
		list_add(&images, load_image(filename));

		free(filename);
	}

	closedir(dir);

	return images;
}

char**
load_image(char* filename)
{
	size_t line_size;
	char* line_buffer = NULL;
	size_t size = 0;
	char** image = NULL;
	FILE* f;

	f = fopen(filename, "r");

	while (getline(&line_buffer, &line_size, f) > 0)
	{
		image = (char**) realloc(image, sizeof(char*) * (size + 1));
		line_buffer[strlen(line_buffer) - 1] = '\0'; /* No \n, because urk. */
		image[size] = line_buffer;

		line_buffer = NULL;

		size += 1;
	}

	free(line_buffer);

	fclose(f);

	image = (char**) realloc(image, sizeof(char*) * (size + 1));
	image[size] = NULL;

	return image;
}

/* vim: set ts=4 sw=4 cc=80 : */
