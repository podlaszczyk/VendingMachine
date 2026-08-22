from http.server import BaseHTTPRequestHandler, HTTPServer
import json

received = {}

class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path != "/transactions":
            self.send_error(404)
            return

        length = int(self.headers["Content-Length"])
        body = json.loads(self.rfile.read(length))
        key = self.headers.get("Idempotency-Key")

        duplicate = key in received
        received[key] = body

        self.send_response(200 if duplicate else 201)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps({
            "accepted": True,
            "duplicate": duplicate
        }).encode())

        print("key:", key, "status:", body["status"], "duplicate:", duplicate)

    def log_message(self, format, *args):
        print(format % args)


HTTPServer(("127.0.0.1", 8080), Handler).serve_forever()