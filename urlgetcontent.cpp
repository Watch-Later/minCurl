#include "urlgetcontent.h"
#include "mincurl.h"
#include "errorlog.h"
#include <QDebug>

UrlGetContent::UrlGetContent()
{

}

// timeOut in seconds
UrlGetContent::UrlGetContent(const QByteArray& url, bool quiet, int category, int timeOut, bool enableLog, CURL *curl)
{
	this->url = url;
	this->quiet = quiet;
	this->category = category;
	this->curl = curl;
	this->timeOut = timeOut;
	this->enableLog = enableLog;
}

QByteArray UrlGetContent::execute()
{
	QByteArray response;
	CURL*      useMe = curl;
	if (!useMe) {
		useMe = curl_easy_init();
		curl_easy_setopt(useMe, CURLOPT_TIMEOUT, timeOut);
	}

	curlCall call;

	ErrorLog l;
	l.db      = "turboProp";
	l.table   = "curlCalls";
	call.curl = useMe;
	call.get  = url;
	call.category = category;

	//all those are needed
	curl_easy_setopt(useMe, CURLOPT_POST, false);
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);
	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &call.response);
	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, call.errbuf);

	call.curlCode = curl_easy_perform(useMe);

#warning "test sql log"
	auto log = l.logQuery(&call);

	if (call.curlCode != CURLE_OK && !quiet) {
		qDebug().noquote() << "For:" << url << "\n " << call.errbuf;
	}

	if (!curl) { //IF a local instance was used
		curl_easy_cleanup(useMe);
	}

	return call.response;
}

QString UrlGetContent::getSqlLog()
{
	return sqlLog;
}





