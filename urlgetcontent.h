#ifndef URLGETCONTENT_H
#define URLGETCONTENT_H

#include <QByteArray>
#include <curl/curl.h>
#include <QString>

class UrlGetContent
{
public:
	UrlGetContent();
	UrlGetContent(const QByteArray& url, bool quiet, int category, int timeOut = 60, bool enableLog = false, CURL *curl = nullptr);
	QByteArray execute();
	QString getSqlLog();

private:
	QByteArray url;
	bool quiet = false;
	int category = 0;
	int timeOut = 0;
	bool enableLog = false;
	CURL *curl = nullptr;

	QString sqlLog; 
};

#endif // URLGETCONTENT_H
