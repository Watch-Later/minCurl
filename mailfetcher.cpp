#include "mailfetcher.h"
#include "QStacker/qstacker.h"
#include "fileFunction/filefunction.h"
#include "magicEnum/magic_from_string.hpp"
#include "minCurl/mincurl.h"
#include <QDebug>
#include <curl/curl.h>

void MailFetcher::logSearchQuery(QString username, QString password, QString folderUrl, QString searchQuery) {
	if (!logExecution) {
		return;
	}

	QString skel = R"(- request -
username = %1
password = %2
folderUrl = %3
searchQuery = %4
)";
	auto    msg  = skel
				   .arg(username)
				   .arg(password)
				   .arg(folderUrl)
				   .arg(searchQuery);
	logWithTime(logFile, msg);
}

void MailFetcher::logSearchResponse(QString curlCode, QString curlError, QString result) {
	if (!logExecution) {
		return;
	}

	QString skel = R"(- response -
curlCode = %1
curlError = %2
result = %3
)";
	auto    msg  = skel
				   .arg(curlCode)
				   .arg(curlError)
				   .arg(result);
	logWithTime(logFile, msg);
}

void MailFetcher::logMailQuery(QString mailUrl) {
	if (!logExecution) {
		return;
	}

	QString skel = R"(- request -
mailUrl = %1
)";
	auto    msg  = skel
				   .arg(mailUrl);
	logWithTime(logFile, msg);
}

void MailFetcher::logMailResponse(QString curlCode, QString curlError, QString mail) {
	if (!logExecution) {
		return;
	}

	QString skel = R"(- response -
curlCode = %1
curlError = %2
mail (truncated) = %3
)";
	auto    msg  = skel
				   .arg(curlCode)
				   .arg(curlError)
				   .arg(mail.left(500));
	logWithTime(logFile, msg);
}

Mail MailFetcher::fetch(bool verbose, bool printError) {
	CurlKeeper     curl;
	CURLcode       res = CURLE_OK;
	CurlCallResult result;
	Mail           emptyMail;
	if (curl) {
		/*
		 * search query
		 * find the id of the mail we want to download
		 */
		logSearchQuery(username, password, folderUrl, searchQuery);

		curl_easy_setopt(curl, CURLOPT_USERNAME, username.toLocal8Bit().data());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.toLocal8Bit().data());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.result);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBWriter);
		curl_easy_setopt(curl, CURLOPT_URL, folderUrl.toLocal8Bit().data());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, searchQuery.toLocal8Bit().data()); /* Perform the fetch */
		res = curl_easy_perform(curl);

		// response
		auto& r = result.result;
		logSearchResponse(asString(res), curl_easy_strerror(res), r);

		// check curl result
		if (res != CURLE_OK) {
			if (printError) {
				//TODO
				// se curl danno errori
				// mettere loop con N tentativi

				QString errSkel = R"(
curl_easy_perform() failed!
res = %1
curl string error = %2
username = %3
password = %4
folderUrl = %5
searchQuery = %6
)";
				auto    err     = errSkel
							   .arg(res)
							   .arg(curl_easy_strerror(res))
							   .arg(username)
							   .arg(password)
							   .arg(folderUrl)
							   .arg(searchQuery);
				qWarning().noquote() << err << QStacker16Light();
			}
			return emptyMail;
		}

		r.replace("* SEARCH", "");
		r = r.trimmed();

		QList<QByteArray> list;
		if (!r.isEmpty()) {
			// we must do this because "split" when "r" is empty returns {" "}
			// (the list containing the string "<separator>")
			list = r.split(' ');
		}

		// we expect to get 1 mail
		switch (list.size()) {
		case 0:
			// no mails
			return emptyMail;
		case 1: {
			// 1 mail

			/*
			 * mail query
			 */
			auto sMailId = list[0];
			bool ok      = false;
			auto mailId  = sMailId.toInt(&ok);
			if (!ok) {
				qWarning().noquote() << "mail id =" << sMailId << "is not a number" << QStacker16Light();
			}

			auto mailUrl = QSL("%1;UID=%2")
							   .arg(folderUrl)
							   .arg(mailId);

			logMailQuery(mailUrl);
			curl_easy_setopt(curl, CURLOPT_URL, mailUrl.toLocal8Bit().data());
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
			CurlCallResult mailResult;
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mailResult.result);
			res = curl_easy_perform(curl);

			// response
			auto mail = mailResult.result;
			logMailResponse(asString(res), curl_easy_strerror(res), mail);
			// check curl result
			if (res != CURLE_OK) {
				if (printError) {
					qWarning().noquote() << QSL("curl_easy_perform() failed: %1").arg(curl_easy_strerror(res)) << QStacker16Light();
				}
				return emptyMail;
			}

			if (verbose) {
				qDebug().noquote() << mail;
			}

			return {mail, true};
		}
		default:
			// many mails (error)
			if (printError) {
				qWarning().noquote() << "too many result for" << searchQuery << QStacker16Light();
			}
			return {QByteArray(), false};
		}
	}
	return emptyMail;
}
