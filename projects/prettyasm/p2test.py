#!/usr/bin/env python

from p2 import Parser

Tests = [
	["db '", [], [], Parser.ERR_UNTERMINATED_STRING],
	[";bullshit comment", 
		[], 
		[],
		], 
	["label;bullshit comment", 
		[], 
		[],
		], 
	["label:;bullshit comment", 
		[], 
		[],
		], 
	["label:sex;bullshit comment", 
		[], 
		['label', 'SEX', [[]]],
		], 
	["sex", 
		[], 
		[None, 'SEX', [[]]],
		], 
	["bob: mov a, b 		; fuuu", 
		['bob', ':', 'mov', 'a', ',', 'b', ';', 'fuuu'], 
		['bob', 'MOV', [['a'], ['b']]]
		],
	["mike:mvi c, 'Q' 	; bob", 
		['mike', ':', 'mvi', 'c', ',', "'", 'Q', "'", ';', 'bob'], 
		['mike', 'MVI', [['c'], ["'Q'"]]]
		],
	["rob mvi c, 'Q'+8;ommog zoth", 
		['rob', 'mvi', 'c', ',', "'", 'Q', "'", '+', '8', ';', 'ommog', 'zoth'], 
		['rob', 'MVI', [['c'], ["'Q'", '+', '8']]]
		],
	["mov coq + 5, d", 
		['mov', 'coq', '+', '5', ',', 'd'], 
		[None, 'MOV', [['coq', '+', '5'], ['d']]]
		],
	['mov n, ","+\',\'', 
		['mov', 'n', ',', '"', ',', '"', '+', "'", ',', "'"], 
		[None, 'MOV', [['n'], ['","', '+', "','"]]]
		],
	["mvi a, ';puu'", 
		['mvi', 'a', ',', "'", ';', 'p', 'u', 'u', "'"], 
		[None, 'MVI', [['a'], ["';puu'"]]]
		],
	["snurfel", 
		['snurfel'], 
		['snurfel', None, []]
		],
	["sex 1,2,(3+a)", 
		['sex', '1', ',', '2', ',', '(', '3', '+', 'a', ')'], 
		[None, 'SEX', [['1'], ['2'], ['(', '3', '+', 'a', ')']]]
		],
	["db '[       ]', \"*  ;'-'  *\"", 
		['db', "'", '[', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ']', "'", ',', '"', '*', ' ', ' ', ';', "'", '-', "'", ' ', ' ', '*', '"'], 
		[None, 'DB', [["'[       ]'"], ['"*  ;\'-\'  *"']]]
		],
 	["db '  \"'", 
 		['db', "'", ' ', ' ', '"', "'"], 
 		[None, 'DB', [['\'  "\'']]]
 		],
]

testfail1 = "db '"

def TestTokenizer(str, expect = None, exception = None):
	try:
		tokens = [x for x in Parser.tokenize(str)]
		if (tokens != expect):
			print 'TestTokenizer: FAIL %s expect=%s tokens=%s' % (repr(str), repr(expect), repr(tokens))
		else:
			print 'TestTokenizer: PASS %s' % repr(str)
	except Exception as e:
		if e.args[0] == exception:
			print 'TestTokenizer: PASS %s -> %s' % (repr(str), repr(exception))
			pass
		else:
			print 'Unexpected exception: ', e
			raise e			

def TestParser(str, expect = None, exception = None):
	parser = Parser()
	try:
		expr = [x for x in parser.Parse(str)]
		if (expr != expect):
			print 'TestParser: FAIL %s expect=%s expr=%s' % (repr(str), repr(expect), repr(expr))
		else:
			print 'TestParser: PASS %s' % repr(str)
	except Exception as e:
		if e.args[0] == exception:
			print 'TestParser: PASS %s -> %s' % (repr(str), repr(exception))
			pass
		else:
			print 'Unexpected exception: ', e
			raise e			

for test in Tests:
	TestTokenizer(test[0], test[1], test[3] if len(test) > 3 else None)

for test in Tests:
	TestParser(test[0], test[2], test[3] if len(test) > 3 else None)

#TestTokenizer(test1)
#TestTokenizer(test2)
#TestTokenizer(test10)
#TestTokenizer(testfail1, None, Parser.ERR_UNTERMINATED_STRING)

#TestParser(test1)
#TestParser(test2)
#TestParser(test3)
#TestParser(test4)
#TestParser(test5)
#TestParser(test6)
#TestParser(test7)
#TestParser(test8)
#TestParser(test9)
#TestParser(test10)