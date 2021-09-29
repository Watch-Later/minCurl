#include "mailfetcher.h"
#include "QStacker/qstacker.h"
#include "minCurl/mincurl.h"
#include <QDebug>
#include <curl/curl.h>

Mail MailFetcher::fetch(bool verbose, bool printError) {
	CurlKeeper     curl;
	CURLcode       res = CURLE_OK;
	CurlCallResult result;
	Mail           emptyMail;
	if (curl) { /* Set username and password */
		curl_easy_setopt(curl, CURLOPT_USERNAME, username.toLocal8Bit().data());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.toLocal8Bit().data());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.result);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QBWriter);
		curl_easy_setopt(curl, CURLOPT_URL, folderUrl.toLocal8Bit().data());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, searchQuery.toLocal8Bit().data()); /* Perform the fetch */
		res = curl_easy_perform(curl);

		// check curl result
		if (res != CURLE_OK) {
			if (printError) {
				//TODO
				// se curl danno errori
				// mettere loop con N tentativi
				qWarning().noquote() << QSL("curl_easy_perform() failed: %1").arg(curl_easy_strerror(res)) << QStacker16Light();
			}
			return emptyMail;
		}

		auto& r = result.result;
		r.replace("* SEARCH", "");
		r         = r.trimmed();
		auto list = r.split(' ');
		// we expect to get 1 mail
		switch (list.size()) {
		case 0:
			// no mails
			return emptyMail;
		case 1: {
			// 1 mail
			auto mailId  = list[0].toInt();
			auto mailUrl = QSL("%1;UID=%2")
							   .arg(folderUrl)
							   .arg(mailId);

			curl_easy_setopt(curl, CURLOPT_URL, mailUrl.toLocal8Bit().data());
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
			CurlCallResult mailResult;
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mailResult.result);
			res = curl_easy_perform(curl);

			// check curl result
			if (res != CURLE_OK) {
				if (printError) {
					qWarning().noquote() << QSL("curl_easy_perform() failed: %1").arg(curl_easy_strerror(res)) << QStacker16Light();
				}
				return emptyMail;
			}

			auto mail = mailResult.result;
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
