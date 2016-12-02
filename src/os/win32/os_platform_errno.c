/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2013 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/

#include "os/os.h"

/* WSAGetLastError, GetLastError and errno

Windows supports errno (The Microsoft c Run-Time Library for Windows CE
does so since version 15 (Visual Studio 2008)). Error codes set by the
Windows Sockets implementation, however, are NOT made available via the
errno variable.

WSAGetLastError used to be the thread-safe version of GetLastError, but
nowadays is just an an alias for GetLastError as intended by Microsoft:
http://www.sockets.com/winsock.htm#Deviation_ErrorCodes

There is no relationship between GetLastError and errno.
GetLastError gets the last error that occurred in a Windows API function (for the current thread).
errno contains the last error that occurred in the C runtime library
So if you call a winapi function like CreateFile you check GetLastError
(assuming that the function call failed), while if you call a C runtime library function
like fopen you check errno (again assuming that the call failed).
*/

int
os_getErrno(void)
{
	DWORD err = GetLastError();
	if (err != 0) {
		errno = (int)err;
	}
	return errno;
}

void
os_setErrno(int err)
{
	SetLastError(err);
	errno = err;
}

int
os_strerror_r(int err, char *str, size_t len)
{
	int res = 0;
	DWORD cnt;

	assert(str != NULL);

	cnt = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)str,
		(DWORD)len,
		NULL);

	if (cnt == 0) {
		if (os_getErrno() == ERROR_MORE_DATA) {
			res = ERANGE;
		} else {
			res = EINVAL;
		}
	}
	
	return res;
}

