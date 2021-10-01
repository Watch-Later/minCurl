#ifndef MAILFETCHER_H
#define MAILFETCHER_H

#include <QString>

struct MailFetcherConfig {
	QString username;
	QString password;
	// imap query to search the mail that will be downloaded
	QString searchQuery;
	QString folderUrl;

	bool    logExecution = false;
	QString logFile;
};

struct Mail {
	QByteArray content;
	bool       ok = false;
};

class MailFetcher {
	  public:
	QString username;
	QString password;
	QString searchQuery;
	QString folderUrl;

	bool    logExecution = false;
	QString logFile;

	MailFetcher(MailFetcherConfig conf) {
		this->username     = conf.username;
		this->password     = conf.password;
		this->searchQuery  = conf.searchQuery;
		this->folderUrl    = conf.folderUrl;
		this->logExecution = conf.logExecution;
		this->logFile      = conf.logFile;
	}
	Mail fetch(bool verbose, bool printError);

	  private:
	void logSearchQuery(QString username, QString password, QString folderUrl, QString searchQuery);
	void logSearchResponse(QString curlCode, QString curlError, QString result);
	void logMailQuery(QString mailUrl);
	void logMailResponse(QString curlCode, QString curlError, QString mail);
};

#endif // MAILFETCHER_H
