#ifndef URLGETCONTENT_H
#define URLGETCONTENT_H

#include <QByteArray>
#include <curl/curl.h>
#include <QString>
#include "errorlog.h"

class UrlGetContent
{
public:
	UrlGetContent();
	UrlGetContent(const QByteArray& url, bool quiet, int category, int timeOut = 60, CURL *curl = nullptr);
	QByteArray execute(ErrorLog *eLog = nullptr);
	QString sql;
private:
	QByteArray url;
	bool quiet = false;
	int category = 0;
	int timeOut = 0;
	CURL *curl = nullptr;
};

#endif // URLGETCONTENT_H
