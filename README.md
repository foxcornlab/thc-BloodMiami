# `thc-BloodMiami`

`AI` chat-box pentesting tool, written in `C`. Fires `language` (`SQL`/`NoSQL`/`XSS`/`SSTI`) and `prompt` injection payloads at a chat interface and flags the ones that bite — `DB` error strings, reflected script markers, model jailbreak signals. Fast, threaded, `libcurl` based.

## `What it does`

- `Language injection` (`-i L`) — `SQLi`, `NoSQL`, `XSS`, `JS` injection probes; matches response against error/signature lists (`mysql_fetch_array`, `ORA-01756`, `<script>`, `onerror=`, ...)
- `Prompt injection` (`-i P`) — jailbreak/prompt-override probes; grades hits as `Low` (model confusion), `Medium` (possible bypass), `Critical` (jailbreak signal)
- `Threaded` sender — tunable concurrency (`-c`, default `10`)
- `Custom headers`, `user-agent`, `timeouts`, `verbose` mode, results to file (`-o`)
- Ships two starter dictionaries: `wordlist/language.txt`, `wordlist/prompt.txt`

## `Build`

```sh
git clone https://github.com/foxcornlab/thc-BloodMiami.git
cd thc-BloodMiami

# deps: gcc, make, libcurl dev headers
sudo apt install libcurl4-openssl-dev   # debian/ubuntu

make
```

## `Usage`

```sh
# language injection scan
./build/bloodmiami -t https://example.com/chat -i L -d wordlist/language.txt

# prompt injection scan, verbose, results to file
./build/bloodmiami -t https://example.com/chat -i P -d wordlist/prompt.txt -v -o results.txt

# 20 threads, 10s timeout, custom header
./build/bloodmiami -t https://example.com/chat -i L -d wordlist/language.txt -c 20 -T 10 \
  -H "Authorization: Bearer <token>"
```

```
Usage: bloodmiami [OPTIONS]

Required:
  -t, --target URL      Target chat interface URL
  -i, --injection TYPE  Injection type: L (language) or P (prompt)
  -d, --dictionary FILE Dictionary file with payloads

Optional:
  -v, --verbose         Enable verbose output
  -T, --timeout SEC     Request timeout in seconds (default: 5)
  -U, --user-agent STR  Custom User-Agent string
  -H, --header STR      Add custom HTTP header
  -c, --concurrency NUM Concurrent requests (default: 10)
  -o, --output FILE     Save results to file
  -h, --help            Show help
  -V, --version         Show version
```

## `How it works`

Each dictionary line becomes one `POST` (`Content-Type: application/json`) with the payload embedded in a chat-style JSON body. The response body is scanned for:

- `SQL` errors — `mysql_fetch_array`, `ORA-01756`, `Unclosed quotation mark`, `PostgreSQL query failed`, ...
- `NoSQL` errors — `MongoError`, `BSONObj`, `WriteResult`, ...
- `XSS` markers — `<script>`, `javascript:`, `onerror=`, `document.cookie`, ...
- `JS` errors — `SyntaxError`, `ReferenceError`, `eval(`, ...
- `HTTP 5xx` — flagged as a possible injection-triggered server error

Payloads are JSON-escaped before embedding so quotes/backslashes in the dictionary can't corrupt the request body. TLS verification is intentionally disabled (`CURLOPT_SSL_VERIFYPEER=0`) — this is a testing tool, keep it on lab/authorized targets.

## `Payload dictionaries`

- `wordlist/language.txt` — `SQLi` (`UNION`, error-based, time-based), `NoSQL` operators, `XSS` vectors, `SSTI` probes
- `wordlist/prompt.txt` — jailbreak attempts, role-play overrides, system-prompt extraction probes, indirect-injection markers

Add your own: one payload per line, `#` for comments. Payloads hit the target as-is, so point this only at systems you are allowed to test.

## `Disclaimer`

For authorized security testing and research only. You are responsible for getting permission before pointing this at anything you don't own.

## `Thanks to`

- `The open-source security community` — payload ideas and signature lists
- `libcurl` — doing all the heavy HTTP lifting

> Research by `Vineeth Kumar` (`foxcornlab`). Found a bug or want a feature, open an `issue`.
