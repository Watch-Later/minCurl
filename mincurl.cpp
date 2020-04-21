#include "mincurl.h"
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <curl/curl.h>

size_t QBWriter(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	auto   buffer   = static_cast<QByteArray*>(userp);
	buffer->append(static_cast<char*>(contents), static_cast<int>(realsize));
	return realsize;
}

size_t FakeCurlWriter(void* contents, size_t size, size_t nmemb, void* userp) {
	(void)contents;
	(void)userp;
	size_t realsize = size * nmemb;
	return realsize;
}

size_t STDWriter(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	auto   buffer   = static_cast<std::string*>(userp);
	buffer->append(static_cast<char*>(contents), static_cast<unsigned long>(realsize));
	return realsize;
}

CURLTiming curlTimer(CURLTiming& timing, CURL* curl) {
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &timing.totalTime);
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &timing.speed);
	curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &timing.dnsTime);
	curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &timing.connTime);
	curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &timing.appConnect);
	curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &timing.preTransfer);
	curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &timing.startTtransfer);
	return timing;
}

QString CURLTiming::print() const {
	auto line = QStringLiteral("total: %1 \t, sslReady: %2").arg(totalTime).arg(preTransfer);
	return line;
}

CURLTiming curlTimer(CURL* curl) {
	CURLTiming timing;
	return curlTimer(timing, curl);
}

QByteArray urlGetContent(const QByteArray& url, bool quiet, CURL* curl) {
	char       errbuf[CURL_ERROR_SIZE] = {0};
	QByteArray response;
	CURL*      useMe = curl;
	if (!useMe) {
		useMe = curl_easy_init();
		curl_easy_setopt(useMe, CURLOPT_TIMEOUT, 60); //1 minute
		//all those are "needed"
		curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	}
	//Nothing cames to my mind that will ever change those 3
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());
	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, errbuf);
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);
	
	auto res = curl_easy_perform(useMe);
	if (res != CURLE_OK && !quiet) {
		qCritical().noquote() << "For:" << url << "\n " << errbuf;
	}

	if (!curl) { //IF a local instance was used
		curl_easy_cleanup(useMe);
	}

	return response;
}

QByteArray urlGetContent(const QString& url, bool quiet, CURL* curl) {
	auto u = url.toUtf8();
	return urlGetContent(u, quiet, curl);
}

CurlHeader::~CurlHeader() {
	curl_slist_free_all(chunk);
}

void CurlHeader::add(QString header) {
	add(header.toUtf8());
}

void CurlHeader::add(QByteArray header) {
	add(header.constData());
}

void CurlHeader::add(const char* header) {
	chunk = curl_slist_append(chunk, header);
}

curl_slist* CurlHeader::getChunk() const {
	return chunk;
}

CurlKeeper::CurlKeeper() {
	curl = curl_easy_init();
}

CurlKeeper::~CurlKeeper() {
	//todo check usage of header or other stuff ?
	//in theory you should use CurlHeader
	curl_easy_cleanup(curl);
}

CURL* CurlKeeper::get() const {
	return curl;
}

CurlCallResult urlPostContent(const QByteArray& url, const QByteArray post, bool quiet, CURL* curl) {
	CurlCallResult result;
	char       errbuf[CURL_ERROR_SIZE] = {0};
	CURL*      useMe = curl;
	if (!useMe) {
		useMe = curl_easy_init();
		curl_easy_setopt(useMe, CURLOPT_TIMEOUT, 60); //1 minute
	}

	//all those are needed
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);
	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &result.result);
	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, errbuf);
	curl_easy_setopt(useMe, CURLOPT_POSTFIELDS, post.constData());

	auto res = curl_easy_perform(useMe);
	if (res != CURLE_OK && !quiet) {
		qCritical().noquote() << "For:" << url << "\n " << errbuf;
	}

	if (!curl) { //IF a local instance was used
		curl_easy_cleanup(useMe);
	}
	result.ok = true;

	return result;
}
