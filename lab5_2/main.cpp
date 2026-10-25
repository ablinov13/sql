// Лаба 5.0.2. Аналог http-hello-html.js на C++: сервер возвращает HTML-страницу с "3 + 5 = 8".

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    SetConsoleOutputCP(CP_UTF8);

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
    std::cout << "Server running at http://localhost:3000/\n";

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        char request[4096];
        recv(client, request, sizeof(request), 0);

        // Тело ответа собираем в том же порядке, что и в http-hello-html.js
        std::string body =
            "<!DOCTYPE html>\n"
            "<html>\n"
            "  <head>\n"
            "    <meta charset='utf-8'>\n"
            "  </head>\n"
            "  <body>\n";
        int a = 3;
        int b = 5;
        body += std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(a + b) + "\n";
        body +=
            "  </body>\n"
            "</html>\n";

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
