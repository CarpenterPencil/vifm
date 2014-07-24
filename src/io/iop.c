/* vifm
 * Copyright (C) 2013 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "iop.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/stat.h> /* mkdir() */
#include <sys/types.h> /* mode_t */
#include <unistd.h> /* rmdir() symlink() */

#include <errno.h> /* EEXIST errno */
#include <stddef.h> /* NULL */
#include <stdio.h> /* FILE fclose() fopen() remove() snprintf() */
#include <stdlib.h> /* free() */
#include <string.h> /* strchr() */

#include "../utils/fs.h"
#include "../utils/fs_limits.h"
#include "../utils/log.h"
#include "../utils/macros.h"
#include "../utils/path.h"
#include "../utils/str.h"
#include "../background.h"
#include "ioc.h"

int
iop_mkfile(io_args_t *const args)
{
	const char *const path = args->arg1.path;

#ifndef _WIN32
	FILE *const f = fopen(path, "wb");
	return (f == NULL) ? -1 : fclose(f);
#else
	HANDLE file;

	file = CreateFileA(path, 0, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	CloseHandle(file);
	return 0;
#endif
}

int
iop_mkdir(io_args_t *const args)
{
	const char *const path = args->arg1.path;
	const int create_parent = args->arg3.process_parents;

#ifndef _WIN32
	const int path_prefix_len = 0;
#else
	const int path_prefix_len = 2;
#endif

	if(create_parent)
	{
		char *sep_pos;
		char sep;
		char *const path_copy = strdup(path);

		sep_pos = until_first(path_copy + path_prefix_len, '/');
		do
		{
			sep = *sep_pos;
			*sep_pos = '\0';

			if(!is_dir(path_copy))
			{
#ifndef _WIN32
				if(mkdir(path_copy, 0755) != 0)
#else
				if(!CreateDirectory(path_copy, NULL))
#endif
				{
					free(path_copy);
					return -1;
				}
			}

			*sep_pos = sep;
			sep_pos = until_first(sep_pos + 1, '/');
		}
		while(sep != '\0');

		free(path_copy);
		return 0;
	}
	else
	{
#ifndef _WIN32
		return mkdir(path, 0755);
#else
		return CreateDirectory(path, NULL) == 0;
#endif
	}
}

int
iop_rmdir(io_args_t *const args)
{
	const char *const path = args->arg1.path;

#ifndef _WIN32
	return rmdir(path);
#else
	return RemoveDirectory(path) == 0;
#endif
}

int
iop_ln(io_args_t *const args)
{
	const char *const path = args->arg1.path;
	const char *const target = args->arg2.target;
	const int overwrite = args->arg3.overwrite;

	int result;

#ifdef _WIN32
	char cmd[6 + PATH_MAX*2 + 1];
	char *escaped_path, *escaped_target;
	char base_dir[PATH_MAX + 2];
#endif

#ifndef _WIN32
	result = symlink(path, target);
	if(result != 0 && errno == EEXIST && overwrite && is_symlink(target))
	{
		result = remove(target);
		if(result == 0)
		{
			result = symlink(path, target);
		}
	}
#else
	escaped_path = escape_filename(path, 0);
	escaped_target = escape_filename(target, 0);
	if(escaped_path == NULL || escaped_target == NULL)
	{
		free(escaped_target);
		free(escaped_path);
		return -1;
	}

	(void)overwrite;
	if(GetModuleFileNameA(NULL, base_dir, ARRAY_LEN(base_dir)) == 0)
	{
		free(escaped_target);
		free(escaped_path);
		return -1;
	}

	break_atr(base_dir, '\\');
	snprintf(cmd, sizeof(cmd), "%s\\win_helper -s %s %s", base_dir, escaped_path,
			escaped_target);
	result = system(cmd);

	free(escaped_target);
	free(escaped_path);
#endif

	return result;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */