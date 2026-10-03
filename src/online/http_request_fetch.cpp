//  SuperTuxKart - a fun racing game with go-kart
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// Browser implementation of HTTPRequest, using emscripten_fetch. Requests
// run on the RequestManager thread (a pthread, i.e. a web worker), where a
// synchronous fetch is allowed, so operation() blocks like the curl version.

#ifdef __EMSCRIPTEN__

#include "online/http_request.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "online/request_manager.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"

#include <emscripten/fetch.h>

#include <cstdio>
#include <cstring>

namespace
{
    // Result codes for failures that are not HTTP status codes. HTTP errors
    // use the (positive) status code itself, success is 0.
    enum FetchResult : int64_t
    {
        FETCH_NETWORK_ERROR = -1,
        FETCH_ABORTED       = -2,
        FETCH_WRITE_ERROR   = -3,
        FETCH_INIT_FAILED   = -4
    };

    // ------------------------------------------------------------------------
    bool shouldAbort(const Online::HTTPRequest* request)
    {
        using namespace Online;
        return RequestManager::isRunning() &&
               (RequestManager::get()->getAbort() ||
                RequestManager::get()->getPaused() ||
                request->isCancelled()) &&
               request->isAbortable();
    }   // shouldAbort
}   // anonymous namespace

// ============================================================================
bool Online::globalHTTPRequestInit()
{
    return true;
}   // globalHTTPRequestInit

// ============================================================================
void Online::globalHTTPRequestCleanup()
{
}   // globalHTTPRequestCleanup

// ----------------------------------------------------------------------------
void Online::HTTPRequest::operation()
{
    m_error_string.clear();

    // All parameters added have a '&' added
    if (m_parameters.size() > 0)
    {
        m_parameters.erase(m_parameters.size() - 1);
    }

    if (m_parameters.size() == 0 && !m_disable_sending_log)
    {
        Log::info("HTTPRequest", "Downloading %s", m_url.c_str());
    }
    else if (Log::getLogLevel() <= Log::LL_INFO && !m_disable_sending_log)
    {
        logMessage();
    }

    // A synchronous fetch cannot be interrupted, so cancellation is only
    // checked before and after the transfer.
    if (shouldAbort(this))
    {
        m_result_code = FETCH_ABORTED;
        m_error_string = "Request aborted";
    }
    else
    {
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        // REPLACE without PERSIST_FILE: plain network request, never read
        // from or written to the IndexedDB fetch cache.
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY |
                          EMSCRIPTEN_FETCH_SYNCHRONOUS    |
                          EMSCRIPTEN_FETCH_REPLACE;
        // Host and User-Agent (set by the curl version) are forbidden
        // headers in browsers and are set by the browser itself.
        // Like curl's CURLOPT_POSTFIELDS, every request that is not an
        // asset download is a urlencoded POST, even with an empty body.
        // That content type keeps it a CORS "simple request" (no preflight).
        const char* const post_headers[] =
        {
            "Content-Type", "application/x-www-form-urlencoded", NULL
        };
        if (m_download_assets_request)
        {
            strcpy(attr.requestMethod, "GET");
        }
        else
        {
            strcpy(attr.requestMethod, "POST");
            attr.requestHeaders = post_headers;
            attr.requestData = m_parameters.data();
            attr.requestDataSize = m_parameters.size();
        }
        // timeoutMSecs is ignored by synchronous XHRs, the browser's own
        // network timeouts apply.

        emscripten_fetch_t* fetch = emscripten_fetch(&attr, m_url.c_str());
        if (!fetch)
        {
            m_result_code = FETCH_INIT_FAILED;
            m_error_string = "emscripten_fetch failed to start the request";
        }
        else if (shouldAbort(this))
        {
            m_result_code = FETCH_ABORTED;
            m_error_string = "Request aborted";
        }
        else if (fetch->status == 0)
        {
            // Network failure, which in a browser includes CORS rejections,
            // blocked mixed content and DNS errors. The details are only
            // shown in the browser console.
            m_result_code = FETCH_NETWORK_ERROR;
            m_error_string = "Network error";
            if (fetch->statusText[0])
                m_error_string += std::string(": ") + fetch->statusText;
        }
        else if (m_filename.size() > 0 && fetch->status >= 400)
        {
            // Unlike curl (without CURLOPT_FAILONERROR) don't store an
            // HTTP error page as the downloaded file.
            m_result_code = fetch->status;
            m_error_string = "HTTP error " +
                             StringUtils::toString(fetch->status);
            if (fetch->statusText[0])
                m_error_string += std::string(" ") + fetch->statusText;
        }
        else
        {
            // As with curl, other HTTP status codes still deliver the body.
            m_result_code = 0;
            setTotalSize((double)fetch->numBytes);
            setProgress(0.99f);
            if (m_filename.size() > 0)
            {
                const std::string part = m_filename + ".part";
                FILE* fout = FileUtils::fopenU8Path(part, "wb");
                if (!fout)
                {
                    Log::error("HTTPRequest",
                               "Can't open '%s' for writing.", part.c_str());
                    m_result_code = FETCH_WRITE_ERROR;
                    m_error_string = "Can't open " + part + " for writing";
                }
                else
                {
                    size_t n = (size_t)fetch->numBytes;
                    bool ok = n == 0 ||
                              fwrite(fetch->data, 1, n, fout) == n;
                    ok = (fclose(fout) == 0) && ok;
                    if (!ok)
                    {
                        m_result_code = FETCH_WRITE_ERROR;
                        m_error_string = "Could not write " + part;
                        file_manager->removeFile(part);
                    }
                }
            }
            else if (fetch->numBytes > 0)
            {
                m_string_buffer.assign(fetch->data,
                                       (size_t)fetch->numBytes);
            }
        }
        emscripten_fetch_close(fetch);
    }

    Request::operation();

    if (m_result_code == 0 && m_filename.size() > 0)
    {
        if (UserConfigParams::logAddons())
            Log::info("HTTPRequest", "Download %s successfully.",
                      m_filename.c_str());

        // The behaviour of rename is unspecified if the target
        // file should already exist - so remove it.
        bool ok = file_manager->removeFile(m_filename);
        if (!ok)
        {
            Log::error("addons",
                       "Could not removed existing addons.xml file.");
            m_result_code = FETCH_WRITE_ERROR;
            m_error_string = "Could not remove " + m_filename;
        }
        int ret = FileUtils::renameU8Path(m_filename + ".part", m_filename);
        if (ret != 0)
        {
            Log::error("addons",
                       "Could not rename downloaded addons.xml file!");
            m_result_code = FETCH_WRITE_ERROR;
            m_error_string = "Could not rename " + m_filename + ".part";
        }
    }

    if (m_result_code != 0)
    {
        Log::error("HTTPRequest", "Request failed with error code %lld: %s",
            (long long)m_result_code, getDownloadErrorMessage());
        setProgress(-1.0f);
    }
    else
        setProgress(1.0f);
}   // operation

// ----------------------------------------------------------------------------
const char* Online::HTTPRequest::getDownloadErrorMessage() const
{
    assert(hadDownloadError());
    return m_error_string.c_str();
}   // getDownloadErrorMessage

#endif
