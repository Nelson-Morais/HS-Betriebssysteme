#pragma once
#include <exception>
#include <stdexcept>
#include <string>
#include "webreq.h"

/**
 * C++ wrapper for the webreq library
 *
 * Oliver Kästner
 * Version: 2019-09-14
 */

class WebRequest {
private:
public:
	WebRequest(int argc, char* argv[]) { webreq_init(argc, argv); }

	~WebRequest() { webreq_cleanup(); }

	int download(const std::string& url, const std::string& filename) {
		int rc = webreq_download(url.c_str(), filename.c_str());

		if (rc < 0)
			throw std::runtime_error(webreq_error(rc));

		return rc;
	}

	int downloadViaProxy(const std::string& url, const std::string& filename) {
		int rc = webreq_download_via_proxy(url.c_str(), filename.c_str());

		if (rc < 0)
			throw std::runtime_error(webreq_error(rc));

		return rc;
	}
};