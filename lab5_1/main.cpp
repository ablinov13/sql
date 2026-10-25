// Лаба 5.0.1. Аналог http-hello.js на C++: HTTP-сервер на порту 3000 отвечает "Hello World".

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")  // библиотека сокетов Windows

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    // Инициализация WinSock (в JS это делает сам Node.js)
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Ошибка WSAStartup\n";
        return 1;
    }

    // Сокет, который будет принимать подключения: аналог http.createServer(...).listen(3000)
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

    // Бесконечный цикл: на каждый запрос браузера — один ответ (как функция (request, response) в JS)
    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        char request[4096];
        recv(client, request, sizeof(request), 0);  // запрос читаем, но не разбираем

        std::string body = "Hello World\n";
        // response.writeHead(200, {"Content-Type": "text/plain"})
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;  // response.end("Hello World\n")
        send(client, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(client);
    }
}
