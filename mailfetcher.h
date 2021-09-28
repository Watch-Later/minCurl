#ifndef MAILFETCHER_H
#define MAILFETCHER_H

#include <QString>

struct MailFetcherConfig {
	QString username;
	QString password;
	// imap query to search the mail that will be downloaded
	QString searchQuery;
	QString folderUrl;
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

	MailFetcher(MailFetcherConfig conf) {
		this->username    = conf.username;
		this->password    = conf.password;
		this->searchQuery = conf.searchQuery;
		this->folderUrl   = conf.folderUrl;
	}
	Mail fetch(bool verbose, bool printError);
};

#endif // MAILFETCHER_H
