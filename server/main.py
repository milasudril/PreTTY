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

PORT = 8020
server_dir = Path(__file__).parents[1] / 'client'

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
		self.wfile.write(('%s 200\r\nContent-Type: text/plain\r\n\r\n' % self.request_version).encode('utf-8'))
		ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
		pdict['boundary'] = bytes(pdict['boundary'], 'utf-8')
		pdict['CONTENT-LENGTH'] = int(self.headers.get('Content-length'))
		parsed_data = cgi.parse_multipart(self.rfile, pdict)
		self.wfile.write(parsed_data['source'][0].encode('utf-8'))



handler = HttpReqHandler

with socketserver.TCPServer(("127.0.0.1", PORT), handler) as httpd:
	print("serving at port", PORT)
	while True:
		sock = httpd.get_request()
		if httpd.verify_request(sock[0], sock[1]) == False:
			print("Invalid req")
			continue

		_thread.start_new_thread(httpd.process_request, (sock[0], sock[1]))
