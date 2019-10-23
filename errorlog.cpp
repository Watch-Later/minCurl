#include "errorlog.h"
#include "funkz.h"

int            ErrorLog::truncatedResponseLength = 100;
QString ErrorLog::db                                = "db";
QString ErrorLog::table                             = "table";

QString ErrorLog::logQuery(curlCall* call) {
	auto curl = call->curl;
	auto response = call->response;
	auto get = call->get;
	auto post = call->post;
	qint64     now         = getCurrentTS();
	CURLTiming timing      = curlTimer(curl);
	// seconds
	double     totalTime   = timing.totalTime;
	double     preTransfer = timing.preTransfer;

	long httpCode;
	auto res      = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	if (res != CURLE_OK) {
		qWarning().noquote() << "curl_easy_getinfo() didn't return the curl code.\n"
				   << QStacker();
	}

	QString truncatedResp = response.left(truncatedResponseLength);

	QString sErrBuf;
	if (call->errbuf[0] == '\0') {
		sErrBuf = SQL_NULL;
	} else {
		sErrBuf = call->errbuf;
	}

	// query
	static const QString skel = R"EOD(
	INSERT INTO %1.%2 SET
	ts = %3,
	totalTime = %4,
	preTransfer = %5,
	curlCode = %6,
	httpCode = %7,
	get = '%8',
	post = '%9',
	response = '%10',
	errBuf = '%11',
	category = %12
)EOD";

	auto sql = skel.arg(db)
			.arg(table)
			.arg(now)
			.arg(totalTime)
			.arg(preTransfer)
			.arg(call->curlCode)
			.arg(httpCode)
			.arg(get)
			.arg(post)
			.arg(truncatedResp)
			.arg(sErrBuf)
			.arg(call->category);

	return sql;
}
