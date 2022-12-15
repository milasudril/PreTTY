#!/usr/bin/env python3

import http.server
import socketserver
import socket
from pathlib import Path
import _thread
import cgi
import urllib.parse
import tempfile
import subprocess
import html
import signal
import secrets
import template_file

app_dir = Path(__file__).parents[1]

def escape_html(str):
	return html.escape(str)

def fixed_width(str):
	return '<pre>\n' + escape_html(str) + '</pre>\n'

def make_error_msg(signo):
	return '<div class="app_error">%s</div>'%signal.strsignal(signo);

def build_and_run(source_code, output_stream):
	with tempfile.TemporaryDirectory() as temp_dir:
		src_file_name = temp_dir + '/src.cpp'
		with open(src_file_name, 'wb') as src_file:
			src_file.write(source_code.encode('utf-8'))

		cxx_inc_dir = app_dir / 'lib' / 'cxx'
		exec_name = temp_dir + '/src.out'
		compiler_result = subprocess.run(['g++',
			('-I%s'%cxx_inc_dir),
			'-std=c++20',
			'-O3',
			'-ffast-math',
			'-Wall',
			'-Wextra',
			'-Wconversion',
			'-Werror',
			src_file_name,
			'-o',
			exec_name],
			capture_output=True)
		application_output = 'Compilation failed'
		compiler_output = 'Compilation succeeded'
		error_msg = ''

		if compiler_result.returncode == 0:
			exec_result = subprocess.run([exec_name], capture_output=True)
			application_output = exec_result.stdout.decode('utf-8')
			if exec_result.returncode < 0:
				error_msg = make_error_msg(-exec_result.returncode)
		else:
			compiler_output = fixed_width(compiler_result.stderr.decode('utf-8'))

		res = template_file.string_from_template_file(app_dir / 'client/output.html',
			{'compiler_output': compiler_output,
			'application_output': application_output,
			'error_msg': error_msg})

		output_stream.write(res.encode('utf-8'))

do_exit = False

def shutdown(port):
	print('Shutting down server')
	global do_exit
	do_exit = True
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
		client.connect(('127.0.0.1', port))

class HttpReqHandler(http.server.SimpleHTTPRequestHandler):
	def do_GET(self):
		try:
			url = urllib.parse.urlparse(self.path)
			path = url.path
			params = dict(urllib.parse.parse_qsl(url.query))
			if not 'api_key' in params:
				self.wfile.write(('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version).encode('utf-8'))
				return

			if params['api_key'] != self.api_key:
				self.wfile.write(('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version).encode('utf-8'))
				return

			if path == '/!shutdown':
				shutdown(self.port)
				return

			if path == '/':
				path = '/index.html'

			src_file = app_dir / ('client' + path)

			self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' % self.request_version).encode('utf-8'))

			self.wfile.write(template_file.string_from_template_file(src_file,
				{'port':self.port, 'api_key': self.api_key}).encode('utf-8'))

		except Exception as exc:
			print(exc)

	def do_POST(self):
		try:
			ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
			pdict['boundary'] = bytes(pdict['boundary'], 'utf-8')
			pdict['CONTENT-LENGTH'] = int(self.headers.get('Content-length'))
			parsed_data = cgi.parse_multipart(self.rfile, pdict)

			if not 'api_key' in parsed_data:
				self.wfile.write(('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version).encode('utf-8'))
				return

			if parsed_data['api_key'][0] != self.api_key:
				self.wfile.write(('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version).encode('utf-8'))
				return

			if self.path == '/shutdown':
				shutdown(self.port)
				return

			if self.path == '/build_and_run':
				self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' %
							self.request_version).encode('utf-8'))
				build_and_run(parsed_data['source'][0], self.wfile)
				return

			self.wfile.write(('%s 400 Bad request %s\r\n' % (self.request_version, self.path)).encode('utf-8'))
		except:
			pass

def create_socket(listen_address, handler):
	port = 65535
	while True:
		try:
			return (socketserver.TCPServer((listen_address, port), handler), port)

		except OSError as e:
			if port > 49152:
				port = port - 1
			else:
				port = 65535

def run():
	handler = HttpReqHandler
	server, port = create_socket('127.0.0.1', handler)
	handler.port = port
	handler.api_key = secrets.token_hex()

	with tempfile.TemporaryDirectory() as temp_dir:
		login_page = temp_dir + '/login.html'
		with open(login_page, 'wb') as login_page_file:
			login_page_file.write(template_file.string_from_template_file(app_dir / 'client/login.html',
				{'port':handler.port, 'api_key': handler.api_key}).encode('utf-8'))

		with server as httpd:
			browser = subprocess.run(['xdg-open', login_page])
			global do_exit
			while not do_exit:
				sock = httpd.get_request()
				if do_exit:
					return browser.returncode
				if httpd.verify_request(sock[0], sock[1]) == False:
					print("Invalid req")
					continue

				_thread.start_new_thread(httpd.process_request, (sock[0], sock[1]))
			return browser.returncode

if __name__ == '__main__':
	exit(run())