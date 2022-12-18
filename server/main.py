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
import os
import template_file
import mimetypes

app_dir = Path(__file__).parents[1]

def escape_html(str):
	return html.escape(str)

def write_text(str, output_stream):
	output_stream.write(str.encode('utf-8'))

def print_delimiter(output_stream):
	write_text('<hr>', output_stream)

def make_error_msg(return_code):
	msg = ''
	if return_code < 0:
		msg = 'Process terminated by %s'%signal.strsignal(-return_code)
	else:
		msg = 'Process terminated with exit status %d'%return_code
	return '<p class="error">%s</p>'%msg

def pump_data_esc(src_fd, dest, buffer_size):
	while (buffer := os.read(src_fd, buffer_size)):
		write_text(html.escape(buffer.decode('utf-8')), dest)
		dest.flush()

def pump_data(src_fd, dest, buffer_size):
	while (buffer := os.read(src_fd, buffer_size)):
		dest.write(buffer)
		dest.flush()

def get_mime_from_path(src):
	return mimetypes.guess_type(src)[0]

def compile_single_src_cxx_file(src_file, exec_name, log_stream):
	cxx_inc_dir = app_dir / 'lib' / 'cxx'
	with subprocess.Popen(['g++',
		('-I%s'%cxx_inc_dir),
		'-std=c++20',
		'-O3',
		'-ffast-math',
		'-Wall',
		'-Wextra',
		'-Wconversion',
		'-Werror',
		src_file,
		'-o',
		exec_name],
		bufsize = 0,
		stdout = subprocess.PIPE,
		stdin = subprocess.DEVNULL,
		stderr = subprocess.STDOUT) as compiler:
		write_text('''<h2>Compiler output</h2>''', log_stream)
		log_stream.flush()
		write_text('<pre>', log_stream)
		pump_data_esc(compiler.stdout.fileno(), log_stream, 65536)
		write_text('</pre>', log_stream)
		log_stream.flush()
		compiler.wait()
		print_delimiter(log_stream)
		log_stream.flush()
		if compiler.returncode != 0:
			write_text(make_error_msg(compiler.returncode), log_stream)
			return False
		else:
			write_text('<p>Program compiled successfully</p>', log_stream)
			return True

def run_executable(exec_name, log_stream):
	with subprocess.Popen([exec_name], bufsize=0,
		stdout=subprocess.PIPE,
		stdin=subprocess.DEVNULL,
		stderr=subprocess.STDOUT) as application:
		write_text('''<h2>Application output</h2>\n''', log_stream)
		log_stream.flush()
		pump_data(application.stdout.fileno(), log_stream, 65536)
		application.wait()
		print_delimiter(log_stream)
		log_stream.flush()
		if application.returncode != 0:
			write_text(make_error_msg(application.returncode), log_stream)
			return False
		else:
			write_text('<p>Program exited normally</p>\n', log_stream)
			return True


def build_and_run(source_code, output_stream, api_key):
	write_text('''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="referrer" content="no-referrer">
''', output_stream)

	write_text(template_file.string_from_template_file(app_dir / 'client/output_header.html',
		{'api_key': api_key}), output_stream)

	write_text('''</head>
<body onkeydown="window.parent.document_on_key_down(event)">\n''', output_stream)
	write_text('''<h1>PreTTY output</h1>\n''', output_stream)
	output_stream.flush()
	with tempfile.TemporaryDirectory() as temp_dir:
		src_file_name = temp_dir + '/src.cpp'
		with open(src_file_name, 'wb') as src_file:
			write_text(source_code, src_file)

		exec_name = temp_dir + '/src.out'
		if compile_single_src_cxx_file(src_file_name, exec_name, output_stream):
			output_stream.flush()
			print_delimiter(output_stream)
			output_stream.flush()
			run_executable(exec_name, output_stream)
			output_stream.flush()

	write_text('''</body>
</html>''', output_stream)

def load(filename, old_source, output_stream, api_key):
	try:
		with open(filename, 'rb') as f:
			file_content = f.read().decode('utf-8')
	except:
		file_content = old_source

	str = template_file.string_from_template_file(app_dir / 'client' / 'main_page.html',
		{'api_key': api_key, 'current_file': filename, 'current_source': file_content})

	write_text(str, output_stream)

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
				write_text('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version,
					self.wfile)
				return

			if params['api_key'] != self.api_key:
				write_text('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version,
					self.wfile)
				return

			if path == '/!shutdown':
				shutdown(self.port)
				return

			if path == '/':
				path = '/main_page.html'

			src_file = app_dir / ('client' + path)

			write_text('%s 200\r\nContent-Type: %s\r\n\r\n' % (self.request_version,
				get_mime_from_path(src_file)), self.wfile)

			write_text(template_file.string_from_template_file(src_file,
				{'port':self.port, 'api_key': self.api_key, 'current_file': '', 'current_source': ''}), self.wfile)

		except Exception as exc:
			print(exc)

	def do_POST(self):
		try:
			ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
			pdict['boundary'] = bytes(pdict['boundary'], 'utf-8')
			pdict['CONTENT-LENGTH'] = int(self.headers.get('Content-length'))
			parsed_data = cgi.parse_multipart(self.rfile, pdict)

			if not 'api_key' in parsed_data:
				print('api check 1')
				write_text('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version,
					self.wfile)
				return

			if parsed_data['api_key'][0] != self.api_key:
				print('api check 2')
				write_text('%s 403 Forbidden\r\nInvalid api key\r\n' % self.request_version,
					self.wfile)
				return

			if self.path == '/shutdown':
				shutdown(self.port)
				return

			if self.path == '/load':
				write_text('%s 200\r\nContent-Type: text/html\r\n\r\n' %
					self.request_version, self.wfile)
				load(parsed_data['filename'][0], parsed_data['source'][0], self.wfile, self.api_key)
				return

			if self.path == '/save':
				write_text('%s 200\r\nContent-Type: text/html\r\n\r\n' %
					self.request_version, self.wfile)
				return

			if self.path == '/build_and_run':
				write_text('%s 200\r\nContent-Type: text/html\r\n\r\n' %
					self.request_version, self.wfile)
				build_and_run(parsed_data['source'][0], self.wfile, self.api_key)
				return

			self.wfile.write(('%s 400 Bad request %s\r\n' % (self.request_version, self.path)).encode('utf-8'))

		except Exception as exc:
			print(exc)

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
			write_text(template_file.string_from_template_file(app_dir / 'client/login.html',
				{'port':handler.port, 'api_key': handler.api_key}), login_page_file)

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