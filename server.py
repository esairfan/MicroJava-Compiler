import os
import subprocess
import json
from http.server import SimpleHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    daemon_threads = True


PORT = 8000

class CompilerHandler(SimpleHTTPRequestHandler):
    def do_POST(self):
        if self.path == '/compile':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            
            # Parse POST payload
            try:
                payload = json.loads(post_data)
                code = payload.get('code', '')
            except Exception:
                code = post_data
                
            # Write to a temp file
            temp_file_path = os.path.join('test', 'temp.mj')
            os.makedirs('test', exist_ok=True)
            with open(temp_file_path, 'w', encoding='utf-8') as f:
                f.write(code)
                
            # Compile C++ if executable doesn't exist
            exe_path = './minicompiler'
            if os.name == 'nt':
                exe_path = 'minicompiler.exe'
                
            if not os.path.exists(exe_path):
                # Run g++ compilation
                compile_cmd = ["g++", "-std=c++17", "-Wall", "-Iinclude", "src/main.cpp", "src/lexer.cpp", "src/token.cpp", "src/symbol_table.cpp", "src/parser_rd.cpp", "src/tables.cpp", "src/parser_ll.cpp", "src/parser_lr.cpp", "-o", "minicompiler"]
                subprocess.run(compile_cmd, capture_output=True)
                
            # Execute compiler on the temp file
            run_cmd = [exe_path, temp_file_path]
            process = subprocess.run(run_cmd, capture_output=True, text=True, encoding='utf-8')
            
            stdout_data = process.stdout
            stderr_data = process.stderr
            exit_code = process.returncode
            
            response = {
                'status': 'success' if (exit_code == 0 and "Lexical error" not in stdout_data and "Syntax error" not in stdout_data and "Error at line" not in stdout_data and "Error at line" not in stderr_data) else 'error',
                'stdout': stdout_data,
                'stderr': stderr_data,
                'exit_code': exit_code
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode('utf-8'))
        else:
            self.send_error(404, "Not Found")
            
    def do_GET(self):
        # Serve GUI files from 'gui' directory
        if self.path == '/' or self.path == '/index.html':
            self.path = '/gui/index.html'
        elif self.path in ['/style.css', '/app.js']:
            self.path = '/gui' + self.path
            
        # Determine content type
        content_type = 'text/html'
        if self.path.endswith('.css'):
            content_type = 'text/css'
        elif self.path.endswith('.js'):
            content_type = 'application/javascript'
            
        # Remove leading slash to check local path
        local_path = self.path[1:] if self.path.startswith('/') else self.path
        
        if os.path.exists(local_path) and os.path.isfile(local_path):
            self.send_response(200)
            self.send_header('Content-Type', content_type)
            self.end_headers()
            with open(local_path, 'rb') as f:
                self.wfile.write(f.read())
        else:
            # Fall back to default handler
            super().do_GET()

    def do_OPTIONS(self):
        # Enable CORS
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

def run(server_class=ThreadedHTTPServer, handler_class=CompilerHandler):
    server_address = ('', PORT)
    httpd = server_class(server_address, handler_class)
    print(f"Starting server on http://localhost:{PORT}")
    httpd.serve_forever()

if __name__ == '__main__':
    run()
