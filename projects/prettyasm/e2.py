#!/usr/bin/env python

class Eval:
	FUNCTIONS = {'('};

	def __init__(self):
		pass

	@staticmethod
	def isOperator(str):
		return isinstance(str, basestring) and str in {'+','-','*','/','&','&&','|','||','~','!','<<','>>','=='};

	def wrap(operator, operands):
		return [operator] + operands

	@staticmethod
	def precedence(op):
		l = [['', '&&', '||'],  ['+', '-'],  ['*', '/']]
		try:
			return l.index([x for x in l if op in x][-1])
		except:
			return -1

	def isfunc(self, tok):
		return tok in FUNCTIONS

	def bu(self, car, cdr, precedence = 0, right = False):
		if car == []:
			return []
		elif car == '(':
			return self.bu(self.bu(next(cdr, []), cdr), cdr)
		elif car == ')':
			return []
		elif Eval.isOperator(car):
			return (lambda operator, x1, x2:
				x1 + list(car) + x2 if Eval.precedence(x2[-1] if len(x2) > 0 else -1) <= Eval.precedence(operator) 
				else x1 + x2 + list(car)) (
					car,
					self.bu(next(cdr, []), cdr, Eval.precedence(car), True),
					self.bu(next(cdr, []), cdr, 0, False))
		elif right:
			return list(car)
		else:
			return list(car) + self.bu(next(cdr, []), cdr)


from p2 import Parser

P = Parser()
E = Eval()

def e(expr):
	t = P.tokenize(expr)
	#return E.unwind(next(t, []), t, '+')
	return E.bu(next(t, []), t)

print e('a+b*c') # a b c * +
print e('(a+b)*(2-c/7)') 

#print e('(a-b)*(c+d)/5')  	# ['a', 'b', '-', 'c', 'd', '+', '*', '5', '/']
#print e('a+(b+(c+d))')   	# ['a', 'b', 'c', 'd', '+', '+', '+']
#print e('((a+b)+c)+d')  	# ['a', 'b', '+', 'c', '+', 'd', '+']

#print e('c+(a+b)*d')
#print e('(a+b)*(b+c)')

#print e('a*b+c') 	# a b * c + 
#print e('a+b*c')    # a b c * +
#print e('c+(a+b)')  # c a b + + 
#print e('c*(a+b)')	# c a b + *
#print e('c*(a/(b+c))')	# c a b c + / *
#
#print e('c*(a/(b+c))+1')	# c a b c + / * 1 + 



