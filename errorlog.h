#ifndef ERRORLOG_H
#define ERRORLOG_H

#include "mincurl.h"
#include <curl/curl.h>
#include <QString>
#include "define.h"

struct curlCall {
	CURL* curl = nullptr;
	CURLcode curlCode;
	QString response = nullptr;
	QString get = nullptr;
	QString post = SQL_NULL;
	char errbuf[CURL_ERROR_SIZE]={0};
};

class ErrorLog
{
public:
	QString logQuery(curlCall* call);

	static QString db;
	static QString table;
	static int truncatedResponseLength;
};

#endif // ERRORLOG_H
