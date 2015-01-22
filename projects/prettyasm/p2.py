#!/usr/bin/env python

class Parser:
	ERR_UNTERMINATED_STRING = 'unterminated string literal'

	@staticmethod
	def tokenize(str, t1 = '', t2 = '', quote = None):
		if len(t1) > 0:
			yield t1
		if len(t2) > 0:
			yield t2
		if quote != None and len(str) == 0:
			raise Exception(Parser.ERR_UNTERMINATED_STRING)
		for i, c in enumerate(str):
			if quote != None and c != quote:
				for x in Parser.tokenize(str[i+1:], str[:i], str[i], quote):
					yield x
				break
			if c in '\'"':
				for x in Parser.tokenize(str[i+1:], str[:i], str[i], c if quote == None else None):
					yield x
				break
			if c in ':,;()+-/*':
				for x in Parser.tokenize(str[i+1:], str[:i], str[i]):
					yield x
				break
			elif c in ' \t':
				for x in Parser.tokenize(str[i+1:], str[:i], ''):
					yield x
				break
			elif i == len(str) - 1:
				yield str

	@staticmethod
	def joins(a1, a2):
		return a1[0:-1] + [''.join([a1[-1]] + [a2])]

	@staticmethod
	def args(head, tokens, expect = None):
		return (lambda car, cdr: 
			Parser.args(Parser.joins(head, car), cdr, expect if car != expect else None) if expect != None 
				else Parser.args(head + [car], cdr, car) if isinstance(car, basestring) and car in "'\""
				else [head] if car == [] or car == ';'
				else [head] + Parser.args([], cdr, expect) if car == ','
				else Parser.args(head + [car], cdr, expect)) (next(tokens, []), tokens)

	def instr(self, instr, tokens, label = None):
		return [label, instr, Parser.args([], tokens) if instr != None else []]

	def parse(self, tokens, label = None):
		return (lambda car, cdr:
			self.instr(None, cdr, label) if car == ';' or car == ''
				else self.parse(cdr, label) if car == ':'
				else self.instr(car.upper(), cdr, label) if self.isInstruction(car)
				else self.parse(cdr, label = car)) (next(tokens, ''), tokens)

	def isInstruction(self, str):
		return str in {"mov", "mvi", "sex", "db"}

	def Parse(self, line):
		return self.parse(Parser.tokenize(line))