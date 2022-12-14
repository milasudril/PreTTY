#!/usr/bin/env python3

from string import Template

def string_from_template_file(filename, substitutes):
	with open(filename, 'rb') as f:
		template = Template(f.read().decode('utf-8'))
		return template.substitute(substitutes)