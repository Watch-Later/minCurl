#ifndef ERRORLOG_H
#define ERRORLOG_H

#include "mincurl.h"
#include <QString>
#include <curl/curl.h>

struct curlCall {
	CURL*      curl = nullptr;
	CURLcode   curlCode;
	QByteArray response;
	QByteArray get;
	QByteArray post;
	char       errbuf[CURL_ERROR_SIZE] = {0};
	int        category                = 0;
};

class ErrorLog {
      public:
	QString logQuery(const curlCall* call);

	static QString db;
	static QString table;
	static int     truncatedResponseLength;
};

#endif // ERRORLOG_H
