#include "urlgetcontent.h"
#include "errorlog.h"
#include "mincurl.h"
#include <QDebug>

UrlGetContent::UrlGetContent() {
}

// timeOut in seconds
UrlGetContent::UrlGetContent(const QByteArray& url, bool quiet, int category, int timeOut, CURL* curl) {
	this->url      = url;
	this->quiet    = quiet;
	this->category = category;
	this->curl     = curl;
	this->timeOut  = timeOut;
}

QByteArray UrlGetContent::execute(ErrorLog* eLog) {
	QByteArray response;
	CURL*      useMe = curl;
	if (!useMe) {
		useMe = curl_easy_init();
		curl_easy_setopt(useMe, CURLOPT_TIMEOUT, timeOut);
	}
	char errbuf[CURL_ERROR_SIZE] = {0};

	//all those are needed
	curl_easy_setopt(useMe, CURLOPT_POST, false);
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);
	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, errbuf);

	callPerformed = true;
	curlCode = curl_easy_perform(useMe);

	if (curlCode != CURLE_OK && !quiet) {
		qDebug().noquote() << "For:" << url << "\n " << errbuf;
	}

	if (eLog) {
		curlCall call;
		call.response = response;
		call.curl     = useMe;
		call.get      = url;
		call.category = category;
		call.curlCode = curlCode;
		strcpy(call.errbuf, errbuf);

		sql = eLog->logQuery(&call);
	}

	if (!curl) { //IF a local instance was used
		curl_easy_cleanup(useMe);
	}

	return response;
}

bool UrlGetContent::curlOk() const {
	if (callPerformed && curlCode == CURLcode::CURLE_OK) {
		return true;
	}
	return false;
}
