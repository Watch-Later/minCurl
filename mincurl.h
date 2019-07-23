#ifndef MINCURL_H
#define MINCURL_H
typedef void CURL;
class QString;
typedef unsigned long int size_t;
struct CURLTiming {
	double totalTime = 0;
	double dnsTime = 0;
	double connTime = 0;
	double appConnect = 0;
	double preTransfer = 0;
	double startTtransfer = 0;
	double speed = 0;
	QString print() const;
};
/* Use like this:
 * QByteArray response;
 * curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
 * curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBWriter);
 * finish!
 */ 
size_t QBWriter(void* contents, size_t size, size_t nmemb, void* userp);
//same but with std::string
size_t STDWriter(void* contents, size_t size, size_t nmemb, void* userp);
CURLTiming curlTimer(CURLTiming& timing, CURL *curl);
CURLTiming curlTimer(CURL *curl);
void printTime(const CURLTiming& timing, QString& response);

#endif // MINCURL_H
