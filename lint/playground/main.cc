
#include "httplib.h"
#include "peglib.h"
#include <cstdio>
#include <functional>

using namespace httplib;
using namespace std;

void open_url(const char* url)
{
    string cmd;
#ifdef _MSC_VER
    cmd += "start ";
#else
    cmd += "open ";
#endif
    cmd += url;
    system(cmd.c_str());
}

function<void (size_t, size_t, const string&)> makeJSONFormatter(string& json)
{
    auto init = make_shared<bool>(true);

    return [&json, init](size_t ln, size_t col, const string& msg) mutable {
        if (!init) {
            json += ",";
        }
        json += "{";
        json += R"("ln":)" + to_string(ln) + ",";
        json += R"("col":)" + to_string(col) + ",";
        json += R"("msg":")" + msg + R"(")";
        json += "}";
        *init = false;
    };
}

bool parse_grammar(const string& text, peglib::peg& peg, string& json)
{
    peg.log = makeJSONFormatter(json);
    json += "[";
    auto ret = peg.load_grammar(text.data(), text.size());
    json += "]";
    return ret;
}

bool parse_code(const string& text, peglib::peg& peg, string& json)
{
    peg.log = makeJSONFormatter(json);
    json += "[";
    auto ret = peg.parse_n(text.data(), text.size());
    json += "]";
    return ret;
}

int main(void)
{
    Server svr;

    if (!svr.set_base_dir("./www")) {
        cerr << "invalid 'www' directory." << endl;
        return -1;
    }

    svr.post("/parse", [](const Request& req, Response& res) {
        const auto& grammarText = req.params.at("grammar");

        string grammarResult;
        string codeResult;

        peglib::peg peg;
        auto ret = parse_grammar(grammarText, peg, grammarResult);

        if (ret && peg) {
            const auto& codeText = req.params.at("code");
            parse_code(codeText, peg, codeResult);
        }

        string json;
        json += "{";
        json += "\"grammar\":" + grammarResult;
        if (!codeResult.empty()) {
            json += +",";
            json += "\"code\":" + codeResult;
        }
        json += "}";

        res.set_content(json, "application/json");
    });

    svr.get("/stop", [&](const Request& req, Response& res) {
        svr.stop();
    });

    svr.set_error_handler([](const Request& req, Response& res) {
        const char* fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, res.status);
        res.set_content(buf, "text/html");
    });

    open_url("http://localhost:1234");

    svr.listen("localhost", 1234);

    return 0;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix