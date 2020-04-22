#include "urlgetcontent.h"
#include "errorlog.h"
#include "mincurl.h"
#include <QDebug>

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
		//99.9% of the time is what we want
		curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	}
	char errbuf[CURL_ERROR_SIZE] = {0};

	//why not ?
	curl_easy_setopt(useMe, CURLOPT_POST, false);
	
	//Nothing cames to my mind that will ever change those 3
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);
	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, errbuf);

	for (uint i = 0; i < retryNum; ++i) {
		curlCode = curl_easy_perform(useMe);
		callPerformed = true;

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

		if (curlCode == CURLE_OK) {
			break;
		}
	}
	if (curlCode != CURLE_OK && !quiet) {
		qWarning() << "max number (" << retryNum << ") of curl calls failed for " << url;
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
