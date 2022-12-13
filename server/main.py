#!/usr/bin/env python3

import http.server
import socketserver
import socket
from pathlib import Path
import os
import _thread
import sys
from string import Template
import cgi
import urllib.parse
import tempfile
import subprocess
import html
import signal

app_dir = Path(__file__).parents[1]

def escape_html(str):
	return html.escape(str)

def fixed_width(str):
	return '<pre>\n' + escape_html(str) + '</pre>\n'

def make_error_msg(signo):
	return '<div class="app_error">%s</div>'%signal.strsignal(signo);

def compile_and_execute(source_code, output_stream):
	template = Template('''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>PreTTY output</title>
<link rel="stylesheet" href="https://latex.now.sh/style.min.css" />
<style>
body{max-width:100%; line-height:1.5}
*{font-family:"Andika";}
code,pre{font-family:"Cascadia code"}
</style>
</head>
<body>
<h1>PreTTY output</h1>
<section>
<h2>Compiler output</h2>
$compiler_output
</section>
<section>
<h2>Application output</h2>
$application_output
$error_msg
</section>
</body>
''')
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

		res = template.substitute(compiler_output = compiler_output,
			application_output = application_output,
			error_msg = error_msg)

		output_stream.write(res.encode('utf-8'))

do_exit = False

class HttpReqHandler(http.server.SimpleHTTPRequestHandler):
	def do_GET(self):
		try:
			if self.path == '/':
				self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' % self.request_version).encode('utf-8'))
				with open(app_dir / 'client' / 'index.html', 'rb') as content_file:
					self.wfile.write(content_file.read())
			else:
				filename = Path(urllib.parse.unquote(self.path)).name
				self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' % self.request_version).encode('utf-8'))
				with open(app_dir / 'client' / filename, 'rb') as content_file:
					self.wfile.write(content_file.read())
		except:
			pass

	def do_POST(self):
		try:
			if self.path == '/shutdown':
				global do_exit
				do_exit = True
				with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
					client.connect(('127.0.0.1', self.port))

			elif self.path == '/formaction':
				ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
				pdict['boundary'] = bytes(pdict['boundary'], 'utf-8')
				pdict['CONTENT-LENGTH'] = int(self.headers.get('Content-length'))
				parsed_data = cgi.parse_multipart(self.rfile, pdict)
				self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' %
							self.request_version).encode('utf-8'))
				compile_and_execute(parsed_data['source'][0], self.wfile)
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
	with server as httpd:
		browser = subprocess.run(['xdg-open', 'http://localhost:%d'%port])
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