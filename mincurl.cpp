#include "mincurl.h"
#include <QByteArray>
#include <curl/curl.h>
#include <QString>
#include "common.h"
size_t writeMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	auto   buffer      = static_cast<QByteArray*>(userp);

	buffer->append(static_cast<char*>(contents), static_cast<int>(realsize));

	return realsize;
}


CURLTiming curlTimer(CURLTiming& timing, CURL *curl) {
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &timing.totalTime);
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &timing.speed);
	curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &timing.dnsTime);
	curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &timing.connTime);
	curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME , &timing.appConnect);
	curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME , &timing.preTransfer);
	curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME , &timing.startTtransfer);
	return timing;
}

QString CURLTiming::print() const {
	auto line = QSL("total: %1 \t, sslReady: %2").arg(totalTime).arg(preTransfer);
	return line;
}

CURLTiming curlTimer(CURL *curl) {
	CURLTiming timing;
	return curlTimer(timing,curl);
}
