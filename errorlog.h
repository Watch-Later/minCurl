#ifndef ERRORLOG_H
#define ERRORLOG_H

#include "mincurl.h"
#include <QString>
#include <curl/curl.h>

struct curlCall {
	CURL*    curl = nullptr;
	CURLcode curlCode;
	QString  response;
	QString  get;
	QString  post;
	char     errbuf[CURL_ERROR_SIZE] = {0};
	int      category                = 0;
};

class ErrorLog {
      public:
	QString logQuery(curlCall* call);

	static QString db;
	static QString table;
	static int     truncatedResponseLength;
};

#endif // ERRORLOG_H
