// Лаба 5.0.3. Статическая страница — таблица книг из предыдущих лаб (data/books.txt),
// отдаётся в браузер по схеме http-hello-html: начало HTML → данные → конец HTML.

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

// Заменяет символы, особые для HTML, чтобы текст из файла не сломал разметку
std::string escapeHtml(const std::string& s)
{
    std::string out;
    for (char ch : s) {
        switch (ch) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        default: out += ch;
        }
    }
    return out;
}

// Строит строки таблицы <tr><td>...</td></tr> из файла books.txt
// (название,жанр,цена,год,автор). Возвращает false, если файл не найден
bool buildRows(const std::string& path, std::string& rows, int& count)
{
    std::ifstream file(path);
    if (!file) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, ',')) f.push_back(part);
        if (f.size() != 5) continue;

        count++;
        rows += "      <tr><td>" + std::to_string(count) + "</td>";
        rows += "<td>" + escapeHtml(f[0]) + "</td>";
        rows += "<td>" + escapeHtml(f[4]) + "</td>";
        rows += "<td>" + escapeHtml(f[1]) + "</td>";
        rows += "<td class='num'>" + escapeHtml(f[2]) + "</td>";
        rows += "<td class='num'>" + escapeHtml(f[3]) + "</td></tr>\n";
    }
    return true;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    // Данные читаем один раз при запуске: страница статическая
    std::string rows;
    int count = 0;
    if (!buildRows("data/books.txt", rows, count)) {
        std::cerr << "Не найден файл data/books.txt (запускайте из папки решения)\n";
        return 1;
    }

    // Начало страницы (как первый response.write в http-hello-html.js)
    std::string head =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "  <head>\n"
        "    <meta charset='utf-8'>\n"
        "    <title>Книжный магазин</title>\n"
        "    <style>\n"
        "      body { font-family: Arial, sans-serif; margin: 30px; }\n"
        "      table { border-collapse: collapse; }\n"
        "      th, td { border: 1px solid #999; padding: 6px 12px; }\n"
        "      th { background: #dde6f0; }\n"
        "      tr:nth-child(even) td { background: #f5f5f5; }\n"
        "      .num { text-align: right; }\n"
        "    </style>\n"
        "  </head>\n"
        "  <body>\n"
        "    <h1>Книжный магазин</h1>\n"
        "    <table>\n"
        "      <tr><th>№</th><th>Название</th><th>Автор</th><th>Жанр</th><th>Цена, руб.</th><th>Год</th></tr>\n";

    // Конец страницы (как response.end в http-hello-html.js)
    std::string tail =
        "    </table>\n"
        "    <p>Всего книг: " + std::to_string(count) + "</p>\n"
        "  </body>\n"
        "</html>\n";

    std::string body = head + rows + tail;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Ошибка WSAStartup\n";
        return 1;
    }

    // IPv6-сокет, который принимает и IPv4 (127.0.0.1), и IPv6 (::1) — как сервер Node.js
    SOCKET server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    DWORD v6only = 0;
    setsockopt(server, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&v6only), sizeof(v6only));
    // запретить второму серверу (например, node) занять тот же порт одновременно с нами
    BOOL exclusive = TRUE;
    setsockopt(server, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive), sizeof(exclusive));
    sockaddr_in6 addr{};
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;  // все адреса компьютера
    addr.sin6_port = htons(3000);
    if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR ||
        listen(server, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Не удалось занять порт 3000 (ошибка " << WSAGetLastError()
                  << "). Возможно, уже запущен другой сервер.\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }
    std::cout << "Загружено книг: " << count << "\n";
    std::cout << "Server running at http://localhost:3000/\n";

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        char request[4096];
        recv(client, request, sizeof(request), 0);

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;
        send(client, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(client);
    }
}
