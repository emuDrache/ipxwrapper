/* IPXWrapper - Common functions
 * Copyright (C) 2011 Daniel Collins <solemnwarning@solemnwarning.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <windows.h>
#include <iphlpapi.h>

#include "common.h"
#include "config.h"

HKEY regkey = NULL;

/* Convert a windows error number to an error message */
const char *w32_error(DWORD errnum) {
	static char buf[1024] = {'\0'};
	
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errnum, 0, buf, 1023, NULL);
	buf[strcspn(buf, "\r\n")] = '\0';
	return buf;	
}

BOOL reg_open(REGSAM access) {
	int err = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\IPXWrapper", 0, access, &regkey);
	
	if(err != ERROR_SUCCESS) {
		log_printf("Could not open registry: %s", w32_error(err));
		regkey = NULL;
		
		return FALSE;
	}
	
	return TRUE;
}

void reg_close(void) {
	if(regkey) {
		RegCloseKey(regkey);
		regkey = NULL;
	}
}

char reg_get_char(const char *val_name, char default_val) {
	if(!regkey) {
		return default_val;
	}
	
	char buf;
	DWORD size = 1;
	
	int err = RegQueryValueEx(regkey, val_name, NULL, NULL, (BYTE*)&buf, &size);
	
	if(err != ERROR_SUCCESS) {
		log_printf("Error reading registry value: %s", w32_error(err));
		return default_val;
	}
	
	return size == 1 ? buf : default_val;
}

DWORD reg_get_bin(const char *val_name, void *buf, DWORD size) {
	if(!regkey) {
		return 0;
	}
	
	int err = RegQueryValueEx(regkey, val_name, NULL, NULL, (BYTE*)buf, &size);
	
	if(err != ERROR_SUCCESS) {
		log_printf("Error reading registry value: %s", w32_error(err));
		return 0;
	}
	
	return size;
}

/* Load a system DLL */
HMODULE load_sysdll(const char *name) {
	char path[1024];
	
	GetSystemDirectory(path, sizeof(path));
	
	if(strlen(path) + strlen(name) + 2 > sizeof(path)) {
		log_printf("Path buffer too small, cannot load %s", name);
		return NULL;
	}
	
	strcat(path, "\\");
	strcat(path, name);
	
	HMODULE dll = LoadLibrary(path);
	if(!dll) {
		log_printf("Error loading %s: %s", path, w32_error(GetLastError()));
	}
	
	return dll;
}
