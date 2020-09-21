#include "mincurl.h"
#include "QStacker/qstacker.h"
#include "qstringtokenizer.h"
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <curl/curl.h>

size_t QBWriter(void* contents, size_t size, size_t nmemb, QByteArray* userp) {
	size_t realsize = size * nmemb;
	userp->append(static_cast<char*>(contents), realsize);
	return realsize;
}

size_t QSWriter(void* contents, size_t size, size_t nmemb, QString* userp) {
	size_t realsize = size * nmemb;
	userp->append(static_cast<char*>(contents));
	return realsize;
}

size_t FakeCurlWriter(void* contents, size_t size, size_t nmemb, void* userp) {
	(void)contents;
	(void)userp;
	size_t realsize = size * nmemb;
	return realsize;
}

size_t STDWriter(void* contents, size_t size, size_t nmemb, std::string* userp) {
	size_t realsize = size * nmemb;
	userp->append(static_cast<char*>(contents), realsize);
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
	auto res = urlGetContent2(url, quiet, curl);
	return res.result;
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

/**
  why this stuff is not built in inside curl is a mistery...
 * @brief parseHeader
 * @param headers
 * @return 
 */
[[nodiscard]] Header parseHeader(const QStringView headers) {
	Header header;
	auto   lines = QStringTokenizer{headers, u"\r\n"};
	auto   c     = lines.toContainer();
	//qDebug() << c;
	for (auto& line : lines) {
		QString c = line.toString();
		if (line.length() > 0) {
			auto found = line.indexOf(u":");
			if (found > 0) {
				auto value  = line.mid(found + 1);
				auto key    = line.left(found);
				auto k      = key.toString();
				header[key] = value;
			}
		}
	}

	return header;
}

CurlCallResult urlPostContent(const QByteArray& url, const QByteArray post, bool quiet, CURL* curl) {
	CurlCallResult result;
	char           errbuf[CURL_ERROR_SIZE] = {0};
	CURL*          useMe                   = curl;
	if (!useMe) {
		useMe = curl_easy_init();
		curl_easy_setopt(useMe, CURLOPT_TIMEOUT, 60); //1 minute, if you do not like use you own curl
	}

	//all those are needed
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());

	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &result.result);
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);

	curl_easy_setopt(useMe, CURLOPT_HEADERFUNCTION, QSWriter);
	curl_easy_setopt(useMe, CURLOPT_HEADERDATA, &result.headerRaw);

	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, errbuf);

	curl_easy_setopt(useMe, CURLOPT_POSTFIELDS, post.constData());

	result.errorCode = curl_easy_perform(useMe);
	result.errorMsg  = errbuf;
	curlTimer(result.timing, useMe);
	if (result.errorCode == CURLE_OK) {
		result.ok     = true;
		result.header = parseHeader(result.headerRaw);
	} else if (!quiet) {
		qCritical().noquote() << "For:" << url << "\n " << errbuf;
	}

	if (!curl) { //IF a local instance was used
		curl_easy_cleanup(useMe);
	}

	return result;
}

CurlCallResult urlGetContent2(const QByteArray& url, bool quiet, CURL* curl) {
	CurlCallResult result;
	char           errbuf[CURL_ERROR_SIZE] = {0};
	CURL*          useMe                   = curl;
	if (!useMe) {
		useMe = curl_easy_init();
		curl_easy_setopt(useMe, CURLOPT_TIMEOUT, 60); //1 minute, if you do not like use you own curl
	}
	curl_easy_setopt(useMe, CURLOPT_URL, url.constData());

	curl_easy_setopt(useMe, CURLOPT_WRITEDATA, &result.result);
	curl_easy_setopt(useMe, CURLOPT_WRITEFUNCTION, QBWriter);

	curl_easy_setopt(useMe, CURLOPT_HEADERFUNCTION, QSWriter);
	curl_easy_setopt(useMe, CURLOPT_HEADERDATA, &result.headerRaw);

	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(useMe, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt(useMe, CURLOPT_ERRORBUFFER, errbuf);

	result.errorCode = curl_easy_perform(useMe);
	result.errorMsg  = errbuf;
	curlTimer(result.timing, useMe);
	if (result.errorCode == CURLE_OK) {
		result.ok     = true;
		result.header = parseHeader(result.headerRaw);
	} else if (!quiet) {
		qCritical().noquote() << "For:" << url << "\n " << errbuf;
	}

	if (!curl) { //IF a local instance was used
		curl_easy_cleanup(useMe);
	}

	return result;
}

bool CaseInsensitiveCompare::operator()(QStringView a, QStringView b) const noexcept {
	//we have to provide the operator<=
	return a.compare(b, Qt::CaseInsensitive) < 1;
}

CurlCallResult::CurlCallResult() {
	//quite hard to have smaller header nowadays
	headerRaw.reserve(512);
}

QString CurlCallResult::getError() const {
	return curl_easy_strerror(errorCode);
}
