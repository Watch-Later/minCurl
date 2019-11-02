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
	QString logQuery(curlCall* call);

	QString db                      = "set me";
	QString table                   = "set me";
	int     truncatedResponseLength = 100;
};

#endif // ERRORLOG_H
