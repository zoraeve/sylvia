#pragma once

#include <string>

typedef enum _LIBSYLVIA_METHOD_
{
	_LIBSYLVIA_METHOD_HTTP_ = 1,
	_LIBSYLVIA_METHOD_HTTPS_ = 2,
	_LIBSYLVIA_METHOD_FTP_ = 3,
	_LIBSYLVIA_METHOD_SFTP_ = 4
}LIBSYLVIA_METHOD;

typedef struct _LIBSYLVIA_TASK_ 
{
	std::string URI;
	std::string SaveAs;
	LIBSYLVIA_METHOD method;
}LIBSYLVIA_TASK;
