#pragma once

#include "../commondefs.hpp"
#include "../str_util.hpp"
#include "../net/socket_wrapper.hpp"
#include "../ssl/ssl_ptr.hpp"
#include "../base64.hpp"

namespace http {

    class RequestError final: public std::logic_error {
    public:
        using logic_error::logic_error;
    };

    class ResponseError final: public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    enum class InternetProtocol: std::uint8_t {
        v4,
        v6
    };

    struct Uri final {
        std::string scheme;
        std::string user;
        std::string password;
        std::string host;
        std::string port;
        std::string path;
        std::string query;
        std::string fragment;
    };

    struct HttpVersion final {
        uint16_t major;
        uint16_t minor;
    };

    struct Status final {
        // RFC 7231, 6. Response Status Codes
        enum Code: std::uint16_t {
            Continue = 100,
            SwitchingProtocol = 101,
            Processing = 102,
            EarlyHints = 103,

            Ok = 200,
            Created = 201,
            Accepted = 202,
            NonAuthoritativeInformation = 203,
            NoContent = 204,
            ResetContent = 205,
            PartialContent = 206,
            MultiStatus = 207,
            AlreadyReported = 208,
            ImUsed = 226,

            MultipleChoice = 300,
            MovedPermanently = 301,
            Found = 302,
            SeeOther = 303,
            NotModified = 304,
            UseProxy = 305,
            TemporaryRedirect = 307,
            PermanentRedirect = 308,

            BadRequest = 400,
            Unauthorized = 401,
            PaymentRequired = 402,
            Forbidden = 403,
            NotFound = 404,
            MethodNotAllowed = 405,
            NotAcceptable = 406,
            ProxyAuthenticationRequired = 407,
            RequestTimeout = 408,
            Conflict = 409,
            Gone = 410,
            LengthRequired = 411,
            PreconditionFailed = 412,
            PayloadTooLarge = 413,
            UriTooLong = 414,
            UnsupportedMediaType = 415,
            RangeNotSatisfiable = 416,
            ExpectationFailed = 417,
            MisdirectedRequest = 421,
            UnprocessableEntity = 422,
            Locked = 423,
            FailedDependency = 424,
            TooEarly = 425,
            UpgradeRequired = 426,
            PreconditionRequired = 428,
            TooManyRequests = 429,
            RequestHeaderFieldsTooLarge = 431,
            UnavailableForLegalReasons = 451,

            InternalServerError = 500,
            NotImplemented = 501,
            BadGateway = 502,
            ServiceUnavailable = 503,
            GatewayTimeout = 504,
            HttpVersionNotSupported = 505,
            VariantAlsoNegotiates = 506,
            InsufficientStorage = 507,
            LoopDetected = 508,
            NotExtended = 510,
            NetworkAuthenticationRequired = 511
        };

        HttpVersion httpVersion;
        std::uint16_t code;
        std::string reason;
    };

    using HeaderField = std::pair<std::string, std::string>;
    using HeaderFields = std::vector<HeaderField>;

    struct Response final {
        Status status;
        HeaderFields headerFields;
        std::vector<std::uint8_t> body;
    };

    inline namespace detail {

        constexpr int getAddressFamily(const InternetProtocol internetProtocol) {
            return (internetProtocol == InternetProtocol::v4) ? AF_INET :
                (internetProtocol == InternetProtocol::v6) ? AF_INET6 :
                throw RequestError{"Unsupported protocol"};
        }

        // RFC 7230, 3.2.3. WhiteSpace
        template <typename C>
        constexpr bool isWhiteSpaceChar(const C c) noexcept
        {
            return c == 0x20 || c == 0x09; // space or tab
        };

        // RFC 5234, Appendix B.1. Core Rules
        template <typename C>
        constexpr bool isDigitChar(const C c) noexcept
        {
            return c >= 0x30 && c <= 0x39; // 0 - 9
        }

        // RFC 5234, Appendix B.1. Core Rules
        template <typename C>
        constexpr bool isAlphaChar(const C c) noexcept
        {
            return
                (c >= 0x61 && c <= 0x7A) || // a - z
                (c >= 0x41 && c <= 0x5A); // A - Z
        }

        // RFC 7230, 3.2.6. Field Value Components
        template <typename C>
        constexpr bool isTokenChar(const C c) noexcept
        {
            return c == 0x21 || // !
                c == 0x23 || // #
                c == 0x24 || // $
                c == 0x25 || // %
                c == 0x26 || // &
                c == 0x27 || // '
                c == 0x2A || // *
                c == 0x2B || // +
                c == 0x2D || // -
                c == 0x2E || // .
                c == 0x5E || // ^
                c == 0x5F || // _
                c == 0x60 || // `
                c == 0x7C || // |
                c == 0x7E || // ~
                isDigitChar(c) ||
                isAlphaChar(c);
        };

        // RFC 5234, Appendix B.1. Core Rules
        template <typename C>
        constexpr bool isVisibleChar(const C c) noexcept
        {
            return c >= 0x21 && c <= 0x7E;
        }

        // RFC 7230, Appendix B. Collected ABNF
        template <typename C>
        constexpr bool isObsoleteTextChar(const C c) noexcept
        {
            return static_cast<unsigned char>(c) >= 0x80 &&
                static_cast<unsigned char>(c) <= 0xFF;
        }

        template <class Iterator>
        Iterator skipWhiteSpaces(const Iterator begin, const Iterator end)
        {
            auto i = begin;
            for (i = begin; i != end; ++i)
                if (!isWhiteSpaceChar(*i))
                    break;

            return i;
        }

        // RFC 5234, Appendix B.1. Core Rules
        template <typename T, typename C, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
        constexpr T digitToUint(const C c)
        {
            // DIGIT
            return (c >= 0x30 && c <= 0x39) ? static_cast<T>(c - 0x30) : // 0 - 9
                throw ResponseError{"Invalid digit"};
        }

        // RFC 5234, Appendix B.1. Core Rules
        template <typename T, typename C, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
        constexpr T hexDigitToUint(const C c)
        {
            // HEXDIG
            return (c >= 0x30 && c <= 0x39) ? static_cast<T>(c - 0x30) : // 0 - 9
                (c >= 0x41 && c <= 0x46) ? static_cast<T>(c - 0x41) + T(10) : // A - Z
                (c >= 0x61 && c <= 0x66) ? static_cast<T>(c - 0x61) + T(10) : // a - z, some services send lower-case hex digits
                throw ResponseError{"Invalid hex digit"};
        }

        // RFC 3986, 3. Syntax Components
        template <class Iterator>
        Uri parseUri(const Iterator begin, const Iterator end)
        {
            Uri result;

            // RFC 3986, 3.1. Scheme
            auto i = begin;
            if (i == end || !isAlphaChar(*begin))
                throw RequestError{"Invalid scheme"};

            result.scheme.push_back(*i++);

            for (; i != end && (isAlphaChar(*i) || isDigitChar(*i) || *i == '+' || *i == '-' || *i == '.'); ++i)
                result.scheme.push_back(*i);

            if (i == end || *i++ != ':')
                throw RequestError{"Invalid scheme"};
            if (i == end || *i++ != '/')
                throw RequestError{"Invalid scheme"};
            if (i == end || *i++ != '/')
                throw RequestError{"Invalid scheme"};

            // RFC 3986, 3.2. Authority
            std::string authority = std::string(i, end);

            // RFC 3986, 3.5. Fragment
            const auto fragmentPosition = authority.find('#');
            if (fragmentPosition != std::string::npos)
            {
                result.fragment = authority.substr(fragmentPosition + 1);
                authority.resize(fragmentPosition); // remove the fragment part
            }

            // RFC 3986, 3.4. Query
            const auto queryPosition = authority.find('?');
            if (queryPosition != std::string::npos)
            {
                result.query = authority.substr(queryPosition + 1);
                authority.resize(queryPosition); // remove the query part
            }

            // RFC 3986, 3.3. Path
            const auto pathPosition = authority.find('/');
            if (pathPosition != std::string::npos)
            {
                // RFC 3986, 3.3. Path
                result.path = authority.substr(pathPosition);
                authority.resize(pathPosition);
            }
            else
                result.path = "/";

            // RFC 3986, 3.2.1. User Information
            std::string userinfo;
            const auto hostPosition = authority.find('@');
            if (hostPosition != std::string::npos)
            {
                userinfo = authority.substr(0, hostPosition);

                const auto passwordPosition = userinfo.find(':');
                if (passwordPosition != std::string::npos)
                {
                    result.user = userinfo.substr(0, passwordPosition);
                    result.password = userinfo.substr(passwordPosition + 1);
                }
                else
                    result.user = userinfo;

                result.host = authority.substr(hostPosition + 1);
            }
            else
                result.host = authority;

            // RFC 3986, 3.2.2. Host
            const auto portPosition = result.host.find(':');
            if (portPosition != std::string::npos)
            {
                // RFC 3986, 3.2.3. Port
                result.port = result.host.substr(portPosition + 1);
                result.host.resize(portPosition);
            }

            return result;
        }

        // RFC 7230, 2.6. Protocol Versioning
        template <class Iterator>
        std::pair<Iterator, HttpVersion> parseHttpVersion(const Iterator begin, const Iterator end)
        {
            auto i = begin;

            if (i == end || *i++ != 'H')
                throw ResponseError{"Invalid HTTP version"};
            if (i == end || *i++ != 'T')
                throw ResponseError{"Invalid HTTP version"};
            if (i == end || *i++ != 'T')
                throw ResponseError{"Invalid HTTP version"};
            if (i == end || *i++ != 'P')
                throw ResponseError{"Invalid HTTP version"};
            if (i == end || *i++ != '/')
                throw ResponseError{"Invalid HTTP version"};

            if (i == end)
                throw ResponseError{"Invalid HTTP version"};

            const auto majorVersion = digitToUint<std::uint16_t>(*i++);

            if (i == end || *i++ != '.')
                throw ResponseError{"Invalid HTTP version"};

            if (i == end)
                throw ResponseError{"Invalid HTTP version"};

            const auto minorVersion = digitToUint<std::uint16_t>(*i++);

            return {i, HttpVersion{majorVersion, minorVersion}};
        }

        // RFC 7230, 3.1.2. Status Line
        template <class Iterator>
        std::pair<Iterator, std::uint16_t> parseStatusCode(const Iterator begin, const Iterator end)
        {
            std::uint16_t result = 0;

            auto i = begin;
            while (i != end && isDigitChar(*i))
                result = static_cast<std::uint16_t>(result * 10U) + digitToUint<std::uint16_t>(*i++);

            if (std::distance(begin, i) != 3)
                throw ResponseError{"Invalid status code"};

            return {i, result};
        }

        // RFC 7230, 3.1.2. Status Line
        template <class Iterator>
        std::pair<Iterator, std::string> parseReasonPhrase(const Iterator begin, const Iterator end)
        {
            std::string result;

            auto i = begin;
            for (; i != end && (isWhiteSpaceChar(*i) || isVisibleChar(*i) || isObsoleteTextChar(*i)); ++i)
                result.push_back(static_cast<char>(*i));

            return {i, std::move(result)};
        }

        // RFC 7230, 3.2.6. Field Value Components
        template <class Iterator>
        std::pair<Iterator, std::string> parseToken(const Iterator begin, const Iterator end)
        {
            std::string result;

            auto i = begin;
            for (; i != end && isTokenChar(*i); ++i)
                result.push_back(static_cast<char>(*i));

            if (result.empty())
                throw ResponseError{"Invalid token"};

            return {i, std::move(result)};
        }

        // RFC 7230, 3.2. Header Fields
        template <class Iterator>
        std::pair<Iterator, std::string> parseFieldValue(const Iterator begin, const Iterator end)
        {
            std::string result;

            auto i = begin;
            for (; i != end && (isWhiteSpaceChar(*i) || isVisibleChar(*i) || isObsoleteTextChar(*i)); ++i)
                result.push_back(static_cast<char>(*i));

            // trim white spaces
            result.erase(std::find_if(result.rbegin(), result.rend(), [](const char c) noexcept {
                return !isWhiteSpaceChar(c);
            }).base(), result.end());

            return {i, std::move(result)};
        }

        // RFC 7230, 3.2. Header Fields
        template <class Iterator>
        std::pair<Iterator, std::string> parseFieldContent(const Iterator begin, const Iterator end)
        {
            std::string result;

            auto i = begin;

            for (;;)
            {
                const auto fieldValueResult = parseFieldValue(i, end);
                i = fieldValueResult.first;
                result += fieldValueResult.second;

                // Handle obsolete fold as per RFC 7230, 3.2.4. Field Parsing
                // Obsolete folding is known as linear white space (LWS) in RFC 2616, 2.2 Basic Rules
                auto obsoleteFoldIterator = i;
                if (obsoleteFoldIterator == end || *obsoleteFoldIterator++ != '\r')
                    break;

                if (obsoleteFoldIterator == end || *obsoleteFoldIterator++ != '\n')
                    break;

                if (obsoleteFoldIterator == end || !isWhiteSpaceChar(*obsoleteFoldIterator++))
                    break;

                result.push_back(' ');
                i = obsoleteFoldIterator;
            }

            return {i, std::move(result)};
        }

        // RFC 7230, 3.2. Header Fields
        template <class Iterator>
        std::pair<Iterator, HeaderField> parseHeaderField(const Iterator begin, const Iterator end)
        {
            auto tokenResult = parseToken(begin, end);
            auto i = tokenResult.first;
            auto fieldName = std::move(tokenResult.second);

            if (i == end || *i++ != ':')
                throw ResponseError{"Invalid header"};

            i = skipWhiteSpaces(i, end);

            auto valueResult = parseFieldContent(i, end);
            i = valueResult.first;
            auto fieldValue = std::move(valueResult.second);

            if (i == end || *i++ != '\r')
                throw ResponseError{"Invalid header"};

            if (i == end || *i++ != '\n')
                throw ResponseError{"Invalid header"};

            return {i, {std::move(fieldName), std::move(fieldValue)}};
        }

        // RFC 7230, 3.1.2. Status Line
        template <class Iterator>
        std::pair<Iterator, Status> parseStatusLine(const Iterator begin, const Iterator end)
        {
            const auto httpVersionResult = parseHttpVersion(begin, end);
            auto i = httpVersionResult.first;

            if (i == end || *i++ != ' ')
                throw ResponseError{"Invalid status line"};

            const auto statusCodeResult = parseStatusCode(i, end);
            i = statusCodeResult.first;

            if (i == end || *i++ != ' ')
                throw ResponseError{"Invalid status line"};

            auto reasonPhraseResult = parseReasonPhrase(i, end);
            i = reasonPhraseResult.first;

            if (i == end || *i++ != '\r')
                throw ResponseError{"Invalid status line"};

            if (i == end || *i++ != '\n')
                throw ResponseError{"Invalid status line"};

            return {i, Status{
                httpVersionResult.second,
                statusCodeResult.second,
                std::move(reasonPhraseResult.second)
            }};
        }

        // RFC 7230, 4.1. Chunked Transfer Coding
        template <typename T, class Iterator, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
        T stringToUint(const Iterator begin, const Iterator end)
        {
            T result = 0;
            for (auto i = begin; i != end; ++i)
                result = T(10U) * result + digitToUint<T>(*i);

            return result;
        }

        template <typename T, class Iterator, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
        T hexStringToUint(const Iterator begin, const Iterator end)
        {
            T result = 0;
            for (auto i = begin; i != end; ++i)
                result = T(16U) * result + hexDigitToUint<T>(*i);

            return result;
        }

        // RFC 7230, 3.1.1. Request Line
        inline std::string encodeRequestLine(const std::string& method, const std::string& target)
        {
            return method + " " + target + " HTTP/1.1\r\n";
        }

        // RFC 7230, 3.2. Header Fields
        inline std::string encodeHeaderFields(const HeaderFields& headerFields)
        {
            std::string result;
            for (const auto& headerField : headerFields)
            {
                if (headerField.first.empty())
                    throw RequestError{"Invalid header field name"};

                for (const auto c : headerField.first)
                    if (!isTokenChar(c))
                        throw RequestError{"Invalid header field name"};

                for (const auto c : headerField.second)
                    if (!isWhiteSpaceChar(c) && !isVisibleChar(c) && !isObsoleteTextChar(c))
                        throw RequestError{"Invalid header field value"};

                result += headerField.first + ": " + headerField.second + "\r\n";
            }

            return result;
        }

        inline std::vector<std::uint8_t> encodeHtml(const Uri& uri,
                                                    const std::string& method,
                                                    const std::vector<uint8_t>& body,
                                                    HeaderFields headerFields)
        {
            // RFC 7230, 5.3. Request Target
            const std::string requestTarget = uri.path + (uri.query.empty() ? ""  : '?' + uri.query);

            // RFC 7230, 5.4. Host
            headerFields.push_back({"Host", uri.host});

            // RFC 7230, 3.3.2. Content-Length
            headerFields.push_back({"Content-Length", std::to_string(body.size())});

            // RFC 7617, 2. The 'Basic' Authentication Scheme
            if (!uri.user.empty() || !uri.password.empty())
            {
                std::string userinfo = uri.user + ':' + uri.password;
                headerFields.push_back({"Authorization", "Basic " + lins::data_to_base64_str(userinfo.data(), userinfo.size())});
            }

            const auto headerData = encodeRequestLine(method, requestTarget) +
                encodeHeaderFields(headerFields) +
                "\r\n";

            std::vector<uint8_t> result(headerData.begin(), headerData.end());
            result.insert(result.end(), body.begin(), body.end());

            return result;
        }
    }

    namespace inet {

        class connection {
        public:
            connection() = default;

            ~connection() = default;

            connection(const connection&) = delete;

            connection &operator=(const connection&) = delete;

            connection(connection &&) = delete;

            connection &operator=(connection &&) = delete;

            bool connect(std::string host, int port, bool secure) {
                secure_ = secure;
                bool result{false};
                if(secure_) {
                    try {
                        if(!host.empty()) {
                            long op_res = 1;
                            SSL_METHOD const *method{TLS_client_method()};
                            if(!method) { return false; }

                            ctx = lins::ssl::NEW<SSL_CTX>([&]() { return SSL_CTX_new(method); });
                            if(!ctx) { return false; }

                            SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_NONE, nullptr);

                            long const flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
                            SSL_CTX_set_options(ctx.get(), flags);

                            web = lins::ssl::NEW_BIO_ALL([&]() { return BIO_new_ssl_connect(ctx.get()); });
                            if(!(web != NULL)) { return false; }

                            op_res = BIO_set_conn_hostname(web.get(), (host + ":" + std::to_string(port)).c_str());
                            if(!(1 == op_res)) { return false; }

                            BIO_get_ssl(web.get(), &ssl);
                            if(!(ssl != NULL)) { return false; }

                            static char const PREFERRED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
                            op_res = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
                            if(!(1 == op_res)) { return false; }

                            op_res = SSL_set_tlsext_host_name(ssl, host.c_str());
                            if(!(1 == op_res)) { return false; }

                            op_res = BIO_do_connect(web.get());
                            if(!(1 == op_res)) { return false; }

                            op_res = BIO_do_handshake(web.get());
                            if(!(1 == op_res)) { return false; }

                            result = true;
                        }
                    } catch (...) {
                    }
                } else {
                    if(!sckt_) { sckt_ = std::make_shared<lins::net::socket>(); }
                    if(sckt_->create() && sckt_->connect(host, port)) { result = true; }
                }
                return result;
            }

            size_t send(const void *data, std::int32_t data_size) {
                if(secure_) {
                    return BIO_write(web.get(), data, data_size);
                } else if(sckt_) {
                    return sckt_->send(data, data_size);
                }
                return 0;
            }

            std::vector<std::uint8_t> receive(int len) {
                if(secure_) {
                    std::vector<std::uint8_t> buff{};
                    if(len > 0) {
                        buff.resize(len);
                        int act_rd = BIO_read(web.get(), &buff[0], len);
                        buff.resize(act_rd >= 0 ? act_rd : 0);
                    }
                    return buff;
                } else {
                    if(sckt_) {
                        return sckt_->receive(len);
                    }
                }
                return {};
            }

            void close() {
                if(sckt_) {
                    sckt_->close();
                }
            }

        private:
            std::shared_ptr<lins::net::socket> sckt_{};
            lins::ssl::unique_ptr<SSL_CTX> ctx{};
            ssl_ptr_bio_all web{};
            SSL *ssl{nullptr};
            bool secure_{false};
        };

    }

    class Request final {
        public:
            explicit Request(const std::string& uriString,
                             const InternetProtocol protocol = InternetProtocol::v4):
                internetProtocol{protocol},
                uri{parseUri(uriString.begin(), uriString.end())}
            {
            }

            std::vector<std::uint8_t> compose(std::string const &method = "GET", std::vector<uint8_t> const &body = {}, HeaderFields const &headerFields = {}) {
                return encodeHtml(uri, method, body, headerFields);
            }

            std::vector<std::uint8_t> compose(std::string const &method = "GET", std::string const &body = {}, HeaderFields const &headerFields = {}) {
                return compose(method, std::vector<std::uint8_t>{body.begin(), body.end()}, headerFields);
            }

            Response send(const std::string& method = "GET",
                          const std::string& body = "",
                          const HeaderFields& headerFields = {},
                          const std::chrono::milliseconds timeout = std::chrono::milliseconds{-1})
            {
                return send(method,
                            std::vector<uint8_t>(body.begin(), body.end()),
                            headerFields,
                            timeout);
            }

            Response send(
                const std::string& method,
                const std::vector<uint8_t>& body,
                const HeaderFields& headerFields = {},
                const std::chrono::milliseconds timeout = std::chrono::milliseconds{-1}
            ) {
                Response response{};

                const auto requestData = encodeHtml(uri, method, body, headerFields);

                inet::connection conn{};
                std::vector<std::uint8_t> res{};
                if(conn.connect(host(), port(), secure())) {
                    if(conn.send(requestData.data(), requestData.size()) == requestData.size()) {
                        std::vector<std::uint8_t> tempBuffer;
                        constexpr std::array<std::uint8_t, 2> crlf = {'\r', '\n'};
                        constexpr std::array<std::uint8_t, 4> headerEnd = {'\r', '\n', '\r', '\n'};
                        std::vector<std::uint8_t> responseData;
                        bool parsingBody = false;
                        bool contentLengthReceived = false;
                        std::size_t contentLength = 0U;
                        bool chunkedResponse = false;
                        std::size_t expectedChunkSize = 0U;
                        bool removeCrlfAfterChunk = false;

                        // read the response
                        for(;;) {
                            std::size_t size = 0;

                            // socket.recv(tempBuffer.data(), tempBuffer.size(), (timeout.count() >= 0) ? getRemainingMilliseconds(stopTime) : -1);
                            try {
                                tempBuffer = conn.receive(1'000'000);
                                size = tempBuffer.size();
                            } catch(...) {
                                size = 0;
                            }

                            if(size <= 0) { // disconnected
                                break;
                            }

                            responseData.insert(responseData.end(), tempBuffer.begin(), tempBuffer.end());

                            if(!parsingBody) {
                                // RFC 7230, 3. Message Format
                                // Empty line indicates the end of the header section (RFC 7230, 2.1. Client/Server Messaging)
                                const auto endIterator = std::search(responseData.cbegin(), responseData.cend(),
                                                                     headerEnd.cbegin(), headerEnd.cend());
                                if (endIterator == responseData.cend()) break; // two consecutive CRLFs not found

                                const auto headerBeginIterator = responseData.cbegin();
                                const auto headerEndIterator = endIterator + 2;

                                auto statusLineResult = parseStatusLine(headerBeginIterator, headerEndIterator);
                                auto i = statusLineResult.first;

                                response.status = std::move(statusLineResult.second);

                                for (;;)
                                {
                                    auto headerFieldResult = parseHeaderField(i, headerEndIterator);
                                    i = headerFieldResult.first;

                                    auto fieldName = std::move(headerFieldResult.second.first);
                                    const auto toLower = [](const char c) noexcept {
                                        return (c >= 'A' && c <= 'Z') ? c - ('A' - 'a') : c;
                                    };
                                    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), toLower);

                                    auto fieldValue = std::move(headerFieldResult.second.second);

                                    if (fieldName == "transfer-encoding")
                                    {
                                        // RFC 7230, 3.3.1. Transfer-Encoding
                                        if (fieldValue == "chunked")
                                            chunkedResponse = true;
                                        else
                                            throw ResponseError{"Unsupported transfer encoding: " + fieldValue};
                                    }
                                    else if (fieldName == "content-length")
                                    {
                                        // RFC 7230, 3.3.2. Content-Length
                                        contentLength = stringToUint<std::size_t>(fieldValue.cbegin(), fieldValue.cend());
                                        contentLengthReceived = true;
                                        response.body.reserve(contentLength);
                                    }

                                    response.headerFields.push_back({std::move(fieldName), std::move(fieldValue)});

                                    if (i == headerEndIterator)
                                        break;
                                }

                                responseData.erase(responseData.cbegin(), headerEndIterator + 2);
                                parsingBody = true;
                            }

                            if (parsingBody)
                            {
                                // Content-Length must be ignored if Transfer-Encoding is received (RFC 7230, 3.2. Content-Length)
                                if (chunkedResponse)
                                {
                                    // RFC 7230, 4.1. Chunked Transfer Coding
                                    for (;;)
                                    {
                                        if (expectedChunkSize > 0)
                                        {
                                            const auto toWrite = (std::min)(expectedChunkSize, responseData.size());
                                            response.body.insert(response.body.end(), responseData.begin(),
                                                                 responseData.begin() + static_cast<std::ptrdiff_t>(toWrite));
                                            responseData.erase(responseData.begin(),
                                                               responseData.begin() + static_cast<std::ptrdiff_t>(toWrite));
                                            expectedChunkSize -= toWrite;

                                            if (expectedChunkSize == 0) removeCrlfAfterChunk = true;
                                            if (responseData.empty()) break;
                                        }
                                        else
                                        {
                                            if (removeCrlfAfterChunk)
                                            {
                                                if (responseData.size() < 2) break;

                                                if (!std::equal(crlf.begin(), crlf.end(), responseData.begin()))
                                                    throw ResponseError{"Invalid chunk"};

                                                removeCrlfAfterChunk = false;
                                                responseData.erase(responseData.begin(), responseData.begin() + 2);
                                            }

                                            const auto i = std::search(responseData.begin(), responseData.end(),
                                                                       crlf.begin(), crlf.end());

                                            if (i == responseData.end()) break;

                                            expectedChunkSize = detail::hexStringToUint<std::size_t>(responseData.begin(), i);
                                            responseData.erase(responseData.begin(), i + 2);

                                            if (expectedChunkSize == 0) {
                                                return response;
                                            }
                                        }
                                    }
                                } else {
                                    response.body.insert(response.body.end(), responseData.begin(), responseData.end());
                                    responseData.clear();

                                    // got the whole content
                                    if (contentLengthReceived && response.body.size() >= contentLength) {
                                        return response;
                                    }
                                }
                            }
                        }
                    }
                }

                return response;
            }

            bool secure() const {
                return uri.scheme == "https";
            }

            std::string host() const {
                return uri.host;
            }

            int port() const {
                try {
                    if(uri.port.empty()) {
                        if(uri.scheme == "http") {
                            return 80;
                        } else if(uri.scheme == "https") {
                            return 443;
                        }
                    } else {
                        return lins::str_util::atoui(uri.port);
                    }
                } catch (...) {
                }
                return 80;
            }

        private:
            InternetProtocol internetProtocol;
            Uri uri;
    };

}
