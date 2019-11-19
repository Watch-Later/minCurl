#ifndef MINCURL_H
#define MINCURL_H
using CURL = void;
class QString;
using size_t = unsigned long int;
class QByteArray;

struct CURLTiming {
	double  totalTime      = 0;
	double  dnsTime        = 0;
	double  connTime       = 0;
	double  appConnect     = 0;
	double  preTransfer    = 0;
	double  startTtransfer = 0;
	double  speed          = 0;
	QString print() const;
};
//In theory CURL will write to STDOUT if nothing defined
size_t FakeCurlWriter(void* contents, size_t size, size_t nmemb, void* userp);

/* Use like this:
 * QByteArray response;
 * curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
 * curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBWriter);
 * finish!
 */
size_t QBWriter(void* contents, size_t size, size_t nmemb, void* userp);
//same but with std::string
size_t     STDWriter(void* contents, size_t size, size_t nmemb, void* userp);
CURLTiming curlTimer(CURLTiming& timing, CURL* curl);
CURLTiming curlTimer(CURL* curl);
void       printTime(const CURLTiming& timing, QString& response);

/**
 * @brief urlGetContent
 * @param url
 * @param quiet
 * @param curl let use an already bootstrapped curl instance (header / cookie)
 * @return
 */
QByteArray urlGetContent(const QByteArray& url, bool quiet = false, CURL* curl = nullptr);
#endif // MINCURL_H
