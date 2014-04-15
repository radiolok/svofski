#!/usr/bin/env python

test1 = "bob: mov a, b 		; fuuu"
test2 = "mike:mvi c, 'Q' 	; bob"
test3 = "rob mvi c, 'Q'+8;ommog zoth"
test4 = "mov c + 5, d"
test5 = "mov n, ','"
test6 = "mvi a, ';'"
test7 = "shcnob"
test8 = "sex 1,2,(3+a)"

NORMAL, STRING_LITERAL = 0, 1

def tokenize(str, t1 = '', t2 = ''):
	if len(t1) > 0:
		yield t1
	if len(t2) > 0:
		yield t2
	for i, c in enumerate(str):
		if c in ':,\';':
			for x in tokenize(str[i+1:], str[:i], str[i]):
				yield x
			break
		elif c in ' \t':
			for x in tokenize(str[i+1:], str[:i], ''):
				yield x
			break
		elif i == len(str) - 1:
			yield str

def glue(tokens, state = 0):
	car, cdr = next(tokens, ''), tokens
	return (car + ' ' + (glue(cdr, state + 1) if car != '' else ''))
		
def isInstruction(str):
	return str in {"mov","mvi", "sex"}

def args(head, tokens, expect = None):
	return (lambda car, cdr: 
		args([''.join(head + [car])], cdr, expect if car != expect else None) if expect != None 
		else args([''.join(head + [car])], cdr, car) if car == "'" 
		else head if car == [] or car == ';'
		else head + args([], cdr, expect) if car == ','
		else args([''.join(head + [car])], cdr, expect)) (next(tokens, []), tokens)

def instr(instr, tokens, label = None):
	return ['L:'+label if label != None else '', instr.upper() if instr != None else None, args([], tokens)]

def parse(tokens, label = None):
	return (lambda car, cdr:
		instr(None, cdr, label) if car == ';' or car == ''
		else parse(cdr, label) if car == ':'
		else instr(car, cdr, label) if isInstruction(car)
		else parse(cdr, label = car)) (next(tokens, ''), tokens)

#print [x for x in tokenize(test1)]
#print [x for x in tokenize(test2)]
#print [x for x in tokenize(test3)]

print parse(tokenize(test1))
print parse(tokenize(test2))
print parse(tokenize(test3))
print parse(tokenize(test4))
print parse(tokenize(test5))
print parse(tokenize(test6))
print parse(tokenize(test7))
print parse(tokenize(test8))
