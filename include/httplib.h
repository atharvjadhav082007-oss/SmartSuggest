#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cctype>

#ifdef _WIN32
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET(s) closesocket(s)
    #define IS_VALID_SOCKET(s) ((s) != INVALID_SOCKET)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/time.h>
    #include <signal.h>
    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define IS_VALID_SOCKET(s) ((s) >= 0)
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif

namespace httplib {

struct Request {
    std::string method;
    std::string path;
    std::string target; // full target with query
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> headers;
    std::string body;

    bool has_param(const std::string& key) const {
        return params.find(key) != params.end();
    }

    std::string get_param_value(const std::string& key, size_t id = 0) const {
        (void)id;
        auto it = params.find(key);
        if (it != params.end()) {
            return it->second;
        }
        return "";
    }

    bool has_header(const std::string& key) const {
        for (const auto& h : headers) {
            if (equal_case_insensitive(h.first, key)) return true;
        }
        return false;
    }

    std::string get_header_value(const std::string& key) const {
        for (const auto& h : headers) {
            if (equal_case_insensitive(h.first, key)) return h.second;
        }
        return "";
    }

private:
    static bool equal_case_insensitive(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) {
                return false;
            }
        }
        return true;
    }
};

struct Response {
    int status = 200;
    std::map<std::string, std::string> headers;
    std::string body;

    void set_content(const std::string& s, const std::string& content_type) {
        body = s;
        headers["Content-Type"] = content_type;
    }

    void set_header(const std::string& key, const std::string& val) {
        headers[key] = val;
    }

    void set_redirect(const std::string& url, int stat = 302) {
        status = stat;
        headers["Location"] = url;
    }
};

class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;

    static std::string url_decode(const std::string& in) {
        std::string out;
        out.reserve(in.size());
        for (size_t i = 0; i < in.size(); ++i) {
            if (in[i] == '%') {
                if (i + 2 < in.size()) {
                    auto hexDigit = [](char c) -> int {
                        if (c >= '0' && c <= '9') return c - '0';
                        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                        return -1;
                    };
                    int h1 = hexDigit(in[i + 1]);
                    int h2 = hexDigit(in[i + 2]);
                    if (h1 != -1 && h2 != -1) {
                        out.push_back(static_cast<char>((h1 << 4) | h2));
                        i += 2;
                    } else {
                        out.push_back(in[i]);
                    }
                } else {
                    out.push_back(in[i]);
                }
            } else if (in[i] == '+') {
                out.push_back(' ');
            } else {
                out.push_back(in[i]);
            }
        }
        return out;
    }

    static void parse_form_urlencoded(const std::string& body, std::map<std::string, std::string>& params) {
        std::istringstream stream(body);
        std::string pair;
        while (std::getline(stream, pair, '&')) {
            if (pair.empty()) continue;
            size_t eq_pos = pair.find('=');
            if (eq_pos != std::string::npos) {
                std::string k = url_decode(pair.substr(0, eq_pos));
                std::string v = url_decode(pair.substr(eq_pos + 1));
                params[k] = v;
            } else {
                params[url_decode(pair)] = "";
            }
        }
    }

private:
    struct MountPoint {
        std::string mount_dir;
        std::string base_dir;
    };

    std::map<std::string, Handler> get_routes;
    std::map<std::string, Handler> post_routes;
    std::vector<MountPoint> mount_points;
    bool is_running = false;
    socket_t listen_fd = INVALID_SOCKET;

    static std::string get_mime_type(const std::string& path) {
        if (path.rfind(".html") != std::string::npos || path.rfind(".htm") != std::string::npos) return "text/html; charset=utf-8";
        if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
        if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
        if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
        if (path.rfind(".xml") != std::string::npos) return "text/xml; charset=utf-8";
        if (path.rfind(".png") != std::string::npos) return "image/png";
        if (path.rfind(".jpg") != std::string::npos || path.rfind(".jpeg") != std::string::npos) return "image/jpeg";
        if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
        if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
        return "text/plain; charset=utf-8";
    }

    bool handle_static_file(const Request& req, Response& res) {
        for (const auto& mp : mount_points) {
            if (req.path.rfind(mp.mount_dir, 0) == 0) {
                std::string rel_path = req.path.substr(mp.mount_dir.size());
                if (rel_path.empty() || rel_path == "/") {
                    rel_path = "index.html";
                }
                if (!rel_path.empty() && rel_path.front() == '/') {
                    rel_path = rel_path.substr(1);
                }
                // Block path traversal (e.g. /../words.txt)
                if (rel_path.find("..") != std::string::npos) {
                    continue;
                }
                std::string base = mp.base_dir;
                if (!base.empty() && base.back() != '/' && base.back() != '\\') {
                    base += '/';
                }
                std::string full_path = base + rel_path;
                std::ifstream file(full_path, std::ios::binary);
                if (file.is_open()) {
                    std::ostringstream ss;
                    ss << file.rdbuf();
                    res.status = 200;
                    res.set_content(ss.str(), get_mime_type(full_path));
                    return true;
                }
            }
        }
        return false;
    }

    void parse_request_headers(const std::string& header_text, Request& req) {
        std::istringstream stream(header_text);
        std::string request_line;
        if (!std::getline(stream, request_line)) return;

        std::istringstream line_stream(request_line);
        line_stream >> req.method >> req.target;

        size_t q_pos = req.target.find('?');
        if (q_pos != std::string::npos) {
            req.path = req.target.substr(0, q_pos);
            std::string query_str = req.target.substr(q_pos + 1);
            std::istringstream q_stream(query_str);
            std::string param_pair;
            while (std::getline(q_stream, param_pair, '&')) {
                if (param_pair.empty()) continue;
                size_t eq_pos = param_pair.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = url_decode(param_pair.substr(0, eq_pos));
                    std::string val = url_decode(param_pair.substr(eq_pos + 1));
                    req.params[key] = val;
                } else {
                    req.params[url_decode(param_pair)] = "";
                }
            }
        } else {
            req.path = req.target;
        }

        std::string header_line;
        while (std::getline(stream, header_line) && header_line != "\r" && !header_line.empty()) {
            size_t colon = header_line.find(':');
            if (colon != std::string::npos) {
                std::string h_key = header_line.substr(0, colon);
                std::string h_val = header_line.substr(colon + 1);
                // trim
                size_t first = h_val.find_first_not_of(" \t\r\n");
                size_t last = h_val.find_last_not_of(" \t\r\n");
                if (first != std::string::npos && last != std::string::npos) {
                    req.headers[h_key] = h_val.substr(first, last - first + 1);
                } else {
                    req.headers[h_key] = "";
                }
            }
        }
    }

    void process_client(socket_t client_fd) {
        const size_t MAX_TOTAL_BYTES = 65536; // 64 KB limit
        std::string raw;
        char buffer[4096];
        size_t header_end_pos = std::string::npos;

        // Drop idle/slow clients so one stalled connection can't block the server
#ifdef _WIN32
        DWORD recv_timeout_ms = 10000;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&recv_timeout_ms, sizeof(recv_timeout_ms));
#else
        struct timeval recv_timeout{};
        recv_timeout.tv_sec = 10;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
#endif

        // 1. Read until headers complete (\r\n\r\n)
        while (raw.size() < MAX_TOTAL_BYTES) {
            header_end_pos = raw.find("\r\n\r\n");
            if (header_end_pos != std::string::npos) break;

            int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes <= 0) break;
            raw.append(buffer, static_cast<size_t>(bytes));
        }

        if (raw.empty()) {
            CLOSE_SOCKET(client_fd);
            return;
        }

        if (header_end_pos == std::string::npos) {
            send_simple_response(client_fd, 400, "Bad Request (Header too large or invalid)");
            CLOSE_SOCKET(client_fd);
            return;
        }

        std::string header_text = raw.substr(0, header_end_pos);
        std::string body_prefix = raw.substr(header_end_pos + 4);

        Request req;
        Response res;
        parse_request_headers(header_text, req);

        // Check Content-Length for body parsing
        size_t content_length = 0;
        if (req.has_header("Content-Length")) {
            try {
                content_length = std::stoul(req.get_header_value("Content-Length"));
            } catch (...) {
                send_simple_response(client_fd, 400, "Bad Request: Invalid Content-Length");
                CLOSE_SOCKET(client_fd);
                return;
            }
        }

        if (content_length > MAX_TOTAL_BYTES) {
            send_simple_response(client_fd, 413, "Payload Too Large (Limit 64 KB)");
            CLOSE_SOCKET(client_fd);
            return;
        }

        // Read remaining body if Content-Length specified
        req.body = body_prefix;
        while (req.body.size() < content_length) {
            if (req.body.size() >= MAX_TOTAL_BYTES) {
                send_simple_response(client_fd, 413, "Payload Too Large (Limit 64 KB)");
                CLOSE_SOCKET(client_fd);
                return;
            }
            int to_read = static_cast<int>(std::min(sizeof(buffer), content_length - req.body.size()));
            int bytes = recv(client_fd, buffer, to_read, 0);
            if (bytes <= 0) break;
            req.body.append(buffer, static_cast<size_t>(bytes));
        }

        if (req.body.size() > content_length && content_length > 0) {
            req.body.resize(content_length);
        }

        // Parse form urlencoded body if present
        std::string ctype = req.get_header_value("Content-Type");
        if (ctype.find("application/x-www-form-urlencoded") != std::string::npos) {
            parse_form_urlencoded(req.body, req.params);
        }

        // CORS headers
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, HEAD, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, X-Twilio-Signature, Origin, Accept, X-Requested-With, *");

        if (req.method == "OPTIONS") {
            res.status = 204;
        } else if (req.method == "GET" || req.method == "HEAD") {
            auto route_it = get_routes.find(req.path);
            if (route_it != get_routes.end()) {
                route_it->second(req, res);
            } else if (!handle_static_file(req, res)) {
                res.status = 404;
                res.set_content("<html><body><h1>404 Not Found</h1></body></html>", "text/html");
            }
        } else if (req.method == "POST") {
            auto route_it = post_routes.find(req.path);
            if (route_it != post_routes.end()) {
                route_it->second(req, res);
            } else {
                res.status = 404;
                res.set_content("<html><body><h1>404 Route Not Found</h1></body></html>", "text/html");
            }
        } else {
            res.status = 405;
            res.set_content("Method Not Allowed", "text/plain");
        }

        send_response(client_fd, res);
        CLOSE_SOCKET(client_fd);
    }

    void send_simple_response(socket_t client_fd, int status, const std::string& msg) {
        Response res;
        res.status = status;
        res.set_content(msg, "text/plain");
        send_response(client_fd, res);
    }

    void send_response(socket_t client_fd, const Response& res) {
        std::ostringstream response_stream;
        response_stream << "HTTP/1.1 " << res.status << " ";
        switch (res.status) {
            case 200: response_stream << "OK"; break;
            case 204: response_stream << "No Content"; break;
            case 302: response_stream << "Found"; break;
            case 400: response_stream << "Bad Request"; break;
            case 403: response_stream << "Forbidden"; break;
            case 404: response_stream << "Not Found"; break;
            case 405: response_stream << "Method Not Allowed"; break;
            case 413: response_stream << "Payload Too Large"; break;
            case 429: response_stream << "Too Many Requests"; break;
            default:  response_stream << "Status " << res.status; break;
        }
        response_stream << "\r\n";

        Response mutable_res = res;
        mutable_res.set_header("Content-Length", std::to_string(mutable_res.body.size()));
        mutable_res.set_header("Connection", "close");

        for (const auto& h : mutable_res.headers) {
            response_stream << h.first << ": " << h.second << "\r\n";
        }
        response_stream << "\r\n";
        response_stream << mutable_res.body;

        std::string response_str = response_stream.str();
        size_t sent = 0;
        while (sent < response_str.size()) {
            int n = send(client_fd, response_str.c_str() + sent, static_cast<int>(response_str.size() - sent), 0);
            if (n <= 0) break;
            sent += static_cast<size_t>(n);
        }
    }

public:
    Server() {
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#else
        // A client closing early must not kill the whole process
        signal(SIGPIPE, SIG_IGN);
#endif
    }

    ~Server() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    Server& Get(const std::string& pattern, Handler handler) {
        get_routes[pattern] = handler;
        return *this;
    }

    Server& Post(const std::string& pattern, Handler handler) {
        post_routes[pattern] = handler;
        return *this;
    }

    bool set_mount_point(const std::string& mount_point, const std::string& base_dir) {
        mount_points.push_back({mount_point, base_dir});
        return true;
    }

    bool listen(const std::string& host, int port) {
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (!IS_VALID_SOCKET(listen_fd)) {
            std::cerr << "[Server] Failed to create socket\n";
            return false;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(static_cast<uint16_t>(port));

        if (host == "0.0.0.0" || host.empty()) {
            server_addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            server_addr.sin_addr.s_addr = inet_addr(host.c_str());
        }

        if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "[Server] Bind failed on " << host << ":" << port << "\n";
            CLOSE_SOCKET(listen_fd);
            listen_fd = INVALID_SOCKET;
            return false;
        }

        if (::listen(listen_fd, 10) == SOCKET_ERROR) {
            std::cerr << "[Server] Listen failed\n";
            CLOSE_SOCKET(listen_fd);
            listen_fd = INVALID_SOCKET;
            return false;
        }

        is_running = true;
        std::cout << "[Server] HTTP server started at http://" << host << ":" << port << "/\n";

        while (is_running) {
            sockaddr_in client_addr{};
#ifdef _WIN32
            int addr_len = sizeof(client_addr);
            socket_t client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
#else
            socklen_t addr_len = sizeof(client_addr);
            socket_t client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
#endif
            if (!IS_VALID_SOCKET(client_fd)) {
                if (!is_running) break;
                continue;
            }
            try {
                process_client(client_fd);
            } catch (const std::exception& e) {
                std::cerr << "[Server] Exception handling client: " << e.what() << "\n";
                CLOSE_SOCKET(client_fd);
            } catch (...) {
                std::cerr << "[Server] Unknown exception handling client\n";
                CLOSE_SOCKET(client_fd);
            }
        }

        return true;
    }

    void stop() {
        is_running = false;
        if (IS_VALID_SOCKET(listen_fd)) {
            CLOSE_SOCKET(listen_fd);
            listen_fd = INVALID_SOCKET;
        }
    }
};

} // namespace httplib

#endif // HTTPLIB_H
