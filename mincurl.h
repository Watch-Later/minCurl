#pragma once

#include "curl/curl.h"
#include <QByteArray>
#include <QString>
#include <map>

#define QSL(str) QStringLiteral(str)

using CURL = void;
class QString;
using size_t = unsigned long int;

struct CURLTiming {
	double  totalTime      = 0;
	double  dnsTime        = 0;
	double  connTime       = 0;
	double  appConnect     = 0;
	double  preTransfer    = 0;
	double  startTtransfer = 0;
	double  speed          = 0;
	double  trxByte        = 0;
	QString print() const;
	CURLTiming() = default;
	CURLTiming(CURL* curl);
	void read(CURL* curl);
	void printTime(QString& response) const;
};
CURLTiming curlTimer(CURLTiming& timing, CURL* curl);

//In theory CURL will write to STDOUT if nothing defined
size_t FakeCurlWriter(void* contents, size_t size, size_t nmemb, void* userp);

/* Use like this:
 * QByteArray response;
 * curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
 * curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBWriter);
 * finish!
 */
size_t QBWriter(void* contents, size_t size, size_t nmemb, QByteArray* userp);
//same but with std::string
size_t STDWriter(void* contents, size_t size, size_t nmemb, std::string* userp);

class NonCopyable {
      protected:
	NonCopyable()  = default;
	~NonCopyable() = default;

	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

//cry
struct curl_slist;
class CurlHeader : private NonCopyable {
      public:
	~CurlHeader();
	void              add(QString header);
	void              add(QByteArray header);
	void              add(const char* header);
	void              clear();
	const curl_slist* get() const;

	[[deprecated("use get")]] const curl_slist* getChunk() const;

      private:
	struct curl_slist* chunk = nullptr;
};

class CurlForm : private NonCopyable {
      public:
	CurlForm(CURL* _curl);
	;

	operator curl_mime*() const;
	curl_mime* get() const;

	void add(const QString& name, const QString& value);

	void add(const QByteArray& name, const QByteArray& value);
	void connect();
	~CurlForm();

      private:
	//the curl_mimepart do not to be freed as long as they are used
	curl_mime* form = nullptr;
	CURL*      curl = nullptr;
};

class CurlKeeper : private NonCopyable {
      public:
	CurlKeeper();
	~CurlKeeper();

	operator CURL*() const;
	CURL* get() const;

      private:
	CURL* curl = nullptr;
};

//inspired from https://github.com/whoshuu/cpr/blob/master/include/cpr/cprtypes.h
struct CaseInsensitiveCompare {
	bool operator()(QStringView a, QStringView b) const noexcept;
};
using Header = std::map<QStringView, QStringView, CaseInsensitiveCompare>;
struct CurlCallResult {
	CurlCallResult();
	QString  errorMsg;
	QString  getError() const;
	CURLcode errorCode;
	//used to keep alive all the QStringView
	QString headerRaw;
	Header  header;
	//Keep raw as can be binary stuff
	QByteArray result;
	CURLTiming timing;
	bool       ok = false;
};

/**
 * @brief urlGetContent
 * @param url
 * @param quiet
 * @param curl let use an already bootstrapped curl instance (header / cookie)
 * @return
 */
QByteArray     urlGetContent(const QByteArray& url, bool quiet = false, CURL* curl = nullptr);
QByteArray     urlGetContent(const QString& url, bool quiet = false, CURL* curl = nullptr);
CurlCallResult urlGetContent2(const QByteArray& url, bool quiet = false, CURL* curl = nullptr);
CurlCallResult urlGetContent2(const QString& url, bool quiet = false, CURL* curl = nullptr);
CurlCallResult urlGetContent2(const char* url, bool quiet = false, CURL* curl = nullptr);
//TODO rifare la funzione e ritornare un oggetto composito per sapere se è andato a buon fine
CurlCallResult urlPostContent(const QByteArray& url, const QByteArray post, bool quiet = false, CURL* curl = nullptr);

enum Severity {
	none,
	_qDebug,
	_qInfo,
	_qWarning,
	_qCritical
};

//TODO cablare dentro il warn, di modo che non serve mettere quiet true e gestire fuori errori di basso livello, ma hai già dentro tutto nel 98% dei casi
//Buttaci in mezzo tempi risposta e altre cosine carine
class urlGetContentV3 {
	static CurlCallResult get(const QByteArray& url, bool quiet = false, CURL* curl = nullptr);
	Severity              severity = Severity::none;
};
