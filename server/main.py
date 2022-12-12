#!/usr/bin/env python3

import http.server
import socketserver
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

PORT = 8001
server_dir = Path(__file__).parents[1] / 'client'

def escape_html(str):
	return html.escape(str)

def fixed_width(str):
	return '<pre>\n' + escape_html(str) + '</pre>\n'

def compile_and_execute(source_code, output_stream):
	template = Template('''<!DOCTYPE html>
<html>
<head>
<title>Devbench output</title>
<style>
*{font-family:"Andika"}
code,pre{font-family:"Cascadia code"; font-size:14px}
</style>
</head>
<body>
<h1>Devbench output</h1>
<section>
<h1>Compiler output</h1>
$compiler_output
</section>
<section>
<h1>Application output</h1>
$application_output
</section>
</body>
''')
	with tempfile.TemporaryDirectory() as temp_dir:
		src_file_name = temp_dir + '/src.cpp'
		with open(src_file_name, 'wb') as src_file:
			src_file.write(source_code.encode('utf-8'))

		exec_name = temp_dir + '/src.out'
		compiler_result = subprocess.run(['g++',
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

		if compiler_result.returncode == 0:
			exec_result = subprocess.run([exec_name], capture_output=True)
			application_output = exec_result.stdout.decode('utf-8')
		else:
			compiler_output = fixed_width(compiler_result.stderr.decode('utf-8'))

		res = template.substitute(compiler_output = compiler_output,
			application_output = application_output)

		output_stream.write(res.encode('utf-8'))

class HttpReqHandler(http.server.SimpleHTTPRequestHandler):
	def do_GET(self):
		if self.path == '/':
			self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' % self.request_version).encode('utf-8'))
			with open(server_dir / 'index.html', 'rb') as content_file:
				self.wfile.write(content_file.read())
		else:
			filename = Path(urllib.parse.unquote(self.path)).name
			try:
				print(server_dir / filename)
				self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' % self.request_version).encode('utf-8'))
				with open(server_dir / filename, 'rb') as content_file:
					self.wfile.write(content_file.read())
			except:
				super(HttpReqHandler, self).do_GET()

	def do_POST(self):
		ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
		pdict['boundary'] = bytes(pdict['boundary'], 'utf-8')
		pdict['CONTENT-LENGTH'] = int(self.headers.get('Content-length'))
		parsed_data = cgi.parse_multipart(self.rfile, pdict)
		self.wfile.write(('%s 200\r\nContent-Type: text/html\r\n\r\n' %
					self.request_version).encode('utf-8'))
		compile_and_execute(parsed_data['source'][0], self.wfile)

handler = HttpReqHandler

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


socket, port = create_socket('127.0.0.1', HttpReqHandler)
with socket as httpd:
	print("serving at port", port)
	while True:
		sock = httpd.get_request()
		if httpd.verify_request(sock[0], sock[1]) == False:
			print("Invalid req")
			continue

		_thread.start_new_thread(httpd.process_request, (sock[0], sock[1]))
