/*
	mcast_xfer is a multicast file transfer utility
	Copyright (C) 2022 Brett Kuskie <fullaxx@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __MULTICAST_XFER_FILEUTILS_H__
#define __MULTICAST_XFER_FILEUTILS_H__

long get_filesize(const char *path, int v);
int is_regfile(const char *path, int v);

#endif	/* __MULTICAST_XFER_FILEUTILS_H__ */
