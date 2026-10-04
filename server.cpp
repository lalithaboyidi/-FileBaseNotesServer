#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

// ============================================================
// NOTE CLASS
// ============================================================

class Note
{
public:
    int id;
    string title;
    string content;
    string createdAt;
    string updatedAt;
    bool favorite;
    bool deleted;

    Note()
    {
        id = 0;
        favorite = false;
        deleted = false;
    }
};


// ============================================================
// NOTE MANAGER
// ============================================================

class NoteManager
{
private:
    vector<Note> notes;
    string fileName = "Notes.json";
    int nextId = 1;

public:

    NoteManager()
    {
        load();
    }

    string getTime()
    {
        time_t now = time(nullptr);

        tm localTime{};

        localtime_s(&localTime, &now);

        char buffer[30];

        strftime(
            buffer,
            sizeof(buffer),
            "%Y-%m-%d %H:%M",
            &localTime
        );

        return string(buffer);
    }


    string escapeJson(const string& text)
    {
        string result;

        for (char c : text)
        {
            if (c == '\\')
                result += "\\\\";
            else if (c == '"')
                result += "\\\"";
            else if (c == '\n')
                result += "\\n";
            else if (c == '\r')
                result += "\\r";
            else
                result += c;
        }

        return result;
    }


    string extractString(
        const string& object,
        const string& key
    )
    {
        string search = "\"" + key + "\"";

        size_t keyPos = object.find(search);

        if (keyPos == string::npos)
            return "";

        size_t colon = object.find(
            ':',
            keyPos
        );

        if (colon == string::npos)
            return "";

        size_t firstQuote =
            object.find('"', colon + 1);

        if (firstQuote == string::npos)
            return "";

        string result;

        bool escaped = false;

        for (
            size_t i = firstQuote + 1;
            i < object.length();
            i++
        )
        {
            char c = object[i];

            if (escaped)
            {
                if (c == 'n')
                    result += '\n';
                else if (c == 'r')
                    result += '\r';
                else
                    result += c;

                escaped = false;
            }
            else if (c == '\\')
            {
                escaped = true;
            }
            else if (c == '"')
            {
                break;
            }
            else
            {
                result += c;
            }
        }

        return result;
    }


    int extractInt(
        const string& object,
        const string& key
    )
    {
        string search = "\"" + key + "\"";

        size_t keyPos = object.find(search);

        if (keyPos == string::npos)
            return 0;

        size_t colon = object.find(
            ':',
            keyPos
        );

        if (colon == string::npos)
            return 0;

        size_t start =
            object.find_first_of(
                "0123456789",
                colon + 1
            );

        if (start == string::npos)
            return 0;

        size_t end =
            object.find_first_not_of(
                "0123456789",
                start
            );

        return stoi(
            object.substr(
                start,
                end - start
            )
        );
    }


    bool extractBool(
        const string& object,
        const string& key
    )
    {
        string search = "\"" + key + "\"";

        size_t keyPos = object.find(search);

        if (keyPos == string::npos)
            return false;

        size_t colon = object.find(
            ':',
            keyPos
        );

        if (colon == string::npos)
            return false;

        size_t truePos =
            object.find("true", colon);

        size_t falsePos =
            object.find("false", colon);

        if (
            truePos != string::npos &&
            (
                falsePos == string::npos ||
                truePos < falsePos
            )
        )
        {
            return true;
        }

        return false;
    }


    vector<string> getObjects(
        const string& json
    )
    {
        vector<string> objects;

        int depth = 0;

        bool insideString = false;

        bool escaped = false;

        size_t start = 0;

        for (
            size_t i = 0;
            i < json.length();
            i++
        )
        {
            char c = json[i];

            if (insideString)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (c == '\\')
                {
                    escaped = true;
                }
                else if (c == '"')
                {
                    insideString = false;
                }

                continue;
            }

            if (c == '"')
            {
                insideString = true;
            }
            else if (c == '{')
            {
                if (depth == 0)
                    start = i;

                depth++;
            }
            else if (c == '}')
            {
                depth--;

                if (depth == 0)
                {
                    objects.push_back(
                        json.substr(
                            start,
                            i - start + 1
                        )
                    );
                }
            }
        }

        return objects;
    }


    void load()
    {
        ifstream file(fileName);

        if (!file)
        {
            save();

            return;
        }

        string json(
            (
                istreambuf_iterator<char>(
                    file
                )
            ),
            istreambuf_iterator<char>()
        );

        notes.clear();

        vector<string> objects =
            getObjects(json);

        for (const string& object : objects)
        {
            Note note;

            note.id =
                extractInt(object, "id");

            note.title =
                extractString(object, "title");

            note.content =
                extractString(object, "content");

            note.createdAt =
                extractString(object, "createdAt");

            note.updatedAt =
                extractString(object, "updatedAt");

            note.favorite =
                extractBool(object, "favorite");

            note.deleted =
                extractBool(object, "deleted");

            if (note.id > 0)
            {
                notes.push_back(note);

                nextId =
                    max(
                        nextId,
                        note.id + 1
                    );
            }
        }
    }


    void save()
    {
        ofstream file(
            fileName,
            ios::trunc
        );

        file << "[\n";

        for (
            size_t i = 0;
            i < notes.size();
            i++
        )
        {
            const Note& note = notes[i];

            file << "  {\n";

            file << "    \"id\": "
                 << note.id
                 << ",\n";

            file << "    \"title\": \""
                 << escapeJson(note.title)
                 << "\",\n";

            file << "    \"content\": \""
                 << escapeJson(note.content)
                 << "\",\n";

            file << "    \"createdAt\": \""
                 << escapeJson(note.createdAt)
                 << "\",\n";

            file << "    \"updatedAt\": \""
                 << escapeJson(note.updatedAt)
                 << "\",\n";

            file << "    \"favorite\": "
                 << (
                        note.favorite
                            ? "true"
                            : "false"
                    )
                 << ",\n";

            file << "    \"deleted\": "
                 << (
                        note.deleted
                            ? "true"
                            : "false"
                    )
                 << "\n";

            file << "  }";

            if (i + 1 < notes.size())
                file << ",";

            file << "\n";
        }

        file << "]\n";
    }


    void create(
        const string& title,
        const string& content
    )
    {
        Note note;

        note.id = nextId++;

        note.title = title;

        note.content = content;

        note.createdAt = getTime();

        note.updatedAt = note.createdAt;

        note.favorite = false;

        note.deleted = false;

        notes.push_back(note);

        save();
    }


    bool update(
        int id,
        const string& title,
        const string& content
    )
    {
        for (Note& note : notes)
        {
            if (note.id == id)
            {
                note.title = title;

                note.content = content;

                note.updatedAt = getTime();

                save();

                return true;
            }
        }

        return false;
    }


    bool moveToTrash(int id)
    {
        for (Note& note : notes)
        {
            if (note.id == id)
            {
                note.deleted = true;

                note.updatedAt = getTime();

                save();

                return true;
            }
        }

        return false;
    }


    bool restore(int id)
    {
        for (Note& note : notes)
        {
            if (note.id == id)
            {
                note.deleted = false;

                note.updatedAt = getTime();

                save();

                return true;
            }
        }

        return false;
    }


    bool permanentDelete(int id)
    {
        auto it =
            remove_if(
                notes.begin(),
                notes.end(),
                [id](const Note& note)
                {
                    return note.id == id;
                }
            );

        if (it == notes.end())
            return false;

        notes.erase(
            it,
            notes.end()
        );

        save();

        return true;
    }


    bool toggleFavorite(int id)
    {
        for (Note& note : notes)
        {
            if (note.id == id)
            {
                note.favorite =
                    !note.favorite;

                note.updatedAt = getTime();

                save();

                return true;
            }
        }

        return false;
    }


    string toJson()
    {
        ostringstream out;

        out << "[";

        for (
            size_t i = 0;
            i < notes.size();
            i++
        )
        {
            const Note& note = notes[i];

            if (i > 0)
                out << ",";

            out << "{";

            out << "\"id\":"
                << note.id
                << ",";

            out << "\"title\":\""
                << escapeJson(note.title)
                << "\",";

            out << "\"content\":\""
                << escapeJson(note.content)
                << "\",";

            out << "\"createdAt\":\""
                << escapeJson(note.createdAt)
                << "\",";

            out << "\"updatedAt\":\""
                << escapeJson(note.updatedAt)
                << "\",";

            out << "\"favorite\":"
                << (
                    note.favorite
                        ? "true"
                        : "false"
                )
                << ",";

            out << "\"deleted\":"
                << (
                    note.deleted
                        ? "true"
                        : "false"
                );

            out << "}";
        }

        out << "]";

        return out.str();
    }
};


// ============================================================
// FILE READER
// ============================================================

string readFile(
    const string& path
)
{
    ifstream file(
        path,
        ios::binary
    );

    if (!file)
        return "";

    return string(
        (
            istreambuf_iterator<char>(
                file
            )
        ),
        istreambuf_iterator<char>()
    );
}


// ============================================================
// HTTP RESPONSE
// ============================================================

string makeResponse(
    int code,
    const string& contentType,
    const string& body
)
{
    string status =
        "200 OK";

    if (code == 400)
        status =
            "400 Bad Request";

    else if (code == 404)
        status =
            "404 Not Found";

    ostringstream response;

    response
        << "HTTP/1.1 "
        << status
        << "\r\n";

    response
        << "Content-Type: "
        << contentType
        << "\r\n";

    response
        << "Access-Control-Allow-Origin: *\r\n";

    response
        << "Content-Length: "
        << body.size()
        << "\r\n";

    response
        << "Connection: close\r\n";

    response
        << "\r\n";

    response
        << body;

    return response.str();
}


// ============================================================
// JSON BODY FIELD
// ============================================================

string getBodyField(
    const string& body,
    const string& key
)
{
    string search =
        "\"" + key + "\"";

    size_t pos =
        body.find(search);

    if (pos == string::npos)
        return "";

    size_t colon =
        body.find(':', pos);

    if (colon == string::npos)
        return "";

    size_t firstQuote =
        body.find('"', colon + 1);

    if (firstQuote == string::npos)
        return "";

    string result;

    bool escaped = false;

    for (
        size_t i = firstQuote + 1;
        i < body.length();
        i++
    )
    {
        char c = body[i];

        if (escaped)
        {
            if (c == 'n')
                result += '\n';
            else if (c == 'r')
                result += '\r';
            else
                result += c;

            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else if (c == '"')
        {
            break;
        }
        else
        {
            result += c;
        }
    }

    return result;
}


// ============================================================
// CLIENT HANDLER
// ============================================================

void handleClient(
    SOCKET client,
    NoteManager& manager
)
{
    char buffer[8192];

    string request;


    int received =
        recv(
            client,
            buffer,
            sizeof(buffer),
            0
        );


    if (received <= 0)
    {
        closesocket(client);

        return;
    }


    request.append(
        buffer,
        received
    );


    size_t headerEnd =
        request.find(
            "\r\n\r\n"
        );


    if (headerEnd ==
        string::npos)
    {
        closesocket(client);

        return;
    }


    string headers =
        request.substr(
            0,
            headerEnd
        );


    string body =
        request.substr(
            headerEnd + 4
        );


    size_t contentLength = 0;


    size_t cl =
        headers.find(
            "Content-Length:"
        );


    if (cl != string::npos)
    {
        size_t start =
            cl + 15;

        size_t end =
            headers.find(
                "\r\n",
                start
            );

        contentLength =
            stoi(
                headers.substr(
                    start,
                    end - start
                )
            );
    }


    while (
        body.size() <
        contentLength
    )
    {
        received =
            recv(
                client,
                buffer,
                sizeof(buffer),
                0
            );

        if (received <= 0)
            break;

        body.append(
            buffer,
            received
        );
    }


    istringstream requestLine(
        headers
    );

    string method;
    string path;
    string version;


    requestLine
        >> method
        >> path
        >> version;


    size_t query =
        path.find('?');


    if (query != string::npos)
    {
        path =
            path.substr(
                0,
                query
            );
    }


    // ========================================================
    // WEB FILES
    // ========================================================

    if (
        method == "GET" &&
        (path == "/" ||
         path == "/index.html")
    )
    {
        string html =
            readFile(
                "Web/index.html"
            );

        string response =
            makeResponse(
                200,
                "text/html; charset=UTF-8",
                html
            );

        send(
            client,
            response.c_str(),
            static_cast<int>(
                response.size()
            ),
            0
        );

        closesocket(client);

        return;
    }


    if (
        method == "GET" &&
        path == "/style.css"
    )
    {
        string css =
            readFile(
                "Web/style.css"
            );

        string response =
            makeResponse(
                200,
                "text/css",
                css
            );

        send(
            client,
            response.c_str(),
            static_cast<int>(
                response.size()
            ),
            0
        );

        closesocket(client);

        return;
    }


    if (
        method == "GET" &&
        path == "/script.js"
    )
    {
        string js =
            readFile(
                "Web/script.js"
            );

        string response =
            makeResponse(
                200,
                "application/javascript",
                js
            );

        send(
            client,
            response.c_str(),
            static_cast<int>(
                response.size()
            ),
            0
        );

        closesocket(client);

        return;
    }


    // ========================================================
    // GET ALL NOTES
    // ========================================================

    if (
        method == "GET" &&
        path == "/api/notes"
    )
    {
        string json =
            manager.toJson();

        string response =
            makeResponse(
                200,
                "application/json",
                json
            );

        send(
            client,
            response.c_str(),
            static_cast<int>(
                response.size()
            ),
            0
        );

        closesocket(client);

        return;
    }


    // ========================================================
    // CREATE
    // ========================================================

    if (
        method == "POST" &&
        path == "/api/notes"
    )
    {
        string title =
            getBodyField(
                body,
                "title"
            );

        string content =
            getBodyField(
                body,
                "content"
            );


        if (
            title.empty() ||
            content.empty()
        )
        {
            string response =
                makeResponse(
                    400,
                    "application/json",
                    "{\"success\":false}"
                );

            send(
                client,
                response.c_str(),
                static_cast<int>(
                    response.size()
                ),
                0
            );

            closesocket(client);

            return;
        }


        manager.create(
            title,
            content
        );


        string response =
            makeResponse(
                200,
                "application/json",
                "{\"success\":true}"
            );


        send(
            client,
            response.c_str(),
            static_cast<int>(
                response.size()
            ),
            0
        );


        closesocket(client);

        return;
    }


    // ========================================================
    // NOTE ID OPERATIONS
    // ========================================================

    string prefix =
        "/api/notes/";


    if (
        path.rfind(prefix, 0) == 0
    )
    {
        string rest =
            path.substr(
                prefix.length()
            );


        size_t slash =
            rest.find('/');


        string idText =
            (
                slash == string::npos
                    ? rest
                    : rest.substr(
                        0,
                        slash
                    )
            );


        if (!idText.empty())
        {
            int id =
                stoi(idText);


            string action =
                (
                    slash == string::npos
                        ? ""
                        : rest.substr(
                            slash + 1
                        )
                );


            // UPDATE
            if (
                method == "PUT" &&
                action.empty()
            )
            {
                string title =
                    getBodyField(
                        body,
                        "title"
                    );

                string content =
                    getBodyField(
                        body,
                        "content"
                    );


                bool ok =
                    manager.update(
                        id,
                        title,
                        content
                    );


                string response =
                    makeResponse(
                        ok ? 200 : 404,
                        "application/json",
                        ok
                            ? "{\"success\":true}"
                            : "{\"success\":false}"
                    );


                send(
                    client,
                    response.c_str(),
                    static_cast<int>(
                        response.size()
                    ),
                    0
                );


                closesocket(client);

                return;
            }


            // MOVE TO TRASH
            if (
                method == "DELETE" &&
                action.empty()
            )
            {
                bool ok =
                    manager.moveToTrash(
                        id
                    );


                string response =
                    makeResponse(
                        ok ? 200 : 404,
                        "application/json",
                        ok
                            ? "{\"success\":true}"
                            : "{\"success\":false}"
                    );


                send(
                    client,
                    response.c_str(),
                    static_cast<int>(
                        response.size()
                    ),
                    0
                );


                closesocket(client);

                return;
            }


            // PERMANENT DELETE
            if (
                method == "DELETE" &&
                action == "permanent"
            )
            {
                bool ok =
                    manager.permanentDelete(
                        id
                    );


                string response =
                    makeResponse(
                        ok ? 200 : 404,
                        "application/json",
                        ok
                            ? "{\"success\":true}"
                            : "{\"success\":false}"
                    );


                send(
                    client,
                    response.c_str(),
                    static_cast<int>(
                        response.size()
                    ),
                    0
                );


                closesocket(client);

                return;
            }


            // FAVORITE
            if (
                method == "PATCH" &&
                action == "favorite"
            )
            {
                bool ok =
                    manager.toggleFavorite(
                        id
                    );


                string response =
                    makeResponse(
                        ok ? 200 : 404,
                        "application/json",
                        ok
                            ? "{\"success\":true}"
                            : "{\"success\":false}"
                    );


                send(
                    client,
                    response.c_str(),
                    static_cast<int>(
                        response.size()
                    ),
                    0
                );


                closesocket(client);

                return;
            }


            // RESTORE
            if (
                method == "PATCH" &&
                action == "restore"
            )
            {
                bool ok =
                    manager.restore(
                        id
                    );


                string response =
                    makeResponse(
                        ok ? 200 : 404,
                        "application/json",
                        ok
                            ? "{\"success\":true}"
                            : "{\"success\":false}"
                    );


                send(
                    client,
                    response.c_str(),
                    static_cast<int>(
                        response.size()
                    ),
                    0
                );


                closesocket(client);

                return;
            }
        }
    }


    // ========================================================
    // NOT FOUND
    // ========================================================

    string response =
        makeResponse(
            404,
            "application/json",
            "{\"success\":false,\"message\":\"Not found\"}"
        );


    send(
        client,
        response.c_str(),
        static_cast<int>(
            response.size()
        ),
        0
    );


    closesocket(client);
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    WSADATA wsaData;


    if (
        WSAStartup(
            MAKEWORD(2, 2),
            &wsaData
        ) != 0
    )
    {
        cout
            << "WSAStartup failed."
            << endl;

        return 1;
    }


    NoteManager manager;


    SOCKET serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );


    if (
        serverSocket ==
        INVALID_SOCKET
    )
    {
        cout
            << "Socket creation failed."
            << endl;

        WSACleanup();

        return 1;
    }


    sockaddr_in serverAddress{};


    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_port =
        htons(8080);


    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );


    int option = 1;


    setsockopt(
        serverSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<char*>(
            &option
        ),
        sizeof(option)
    );


    if (
        bind(
            serverSocket,
            reinterpret_cast<sockaddr*>(
                &serverAddress
            ),
            sizeof(serverAddress)
        ) == SOCKET_ERROR
    )
    {
        cout
            << "Bind failed."
            << endl;

        closesocket(
            serverSocket
        );

        WSACleanup();

        return 1;
    }


    if (
        listen(
            serverSocket,
            SOMAXCONN
        ) == SOCKET_ERROR
    )
    {
        cout
            << "Listen failed."
            << endl;

        closesocket(
            serverSocket
        );

        WSACleanup();

        return 1;
    }


    cout << endl;

    cout
        << "===================================="
        << endl;

    cout
        << "     FILE-BASED NOTES SERVER"
        << endl;

    cout
        << "===================================="
        << endl;

    cout
        << "Server running at:"
        << endl;

    cout
        << "http://127.0.0.1:8080"
        << endl;

    cout << endl;

    cout
        << "Data file: Notes.json"
        << endl;

    cout
        << "Press Ctrl+C to stop."
        << endl;

    cout
        << "===================================="
        << endl;


    while (true)
    {
        SOCKET client =
            accept(
                serverSocket,
                nullptr,
                nullptr
            );


        if (
            client ==
            INVALID_SOCKET
        )
        {
            continue;
        }


        handleClient(
            client,
            manager
        );
    }


    closesocket(
        serverSocket
    );

    WSACleanup();

    return 0;
}