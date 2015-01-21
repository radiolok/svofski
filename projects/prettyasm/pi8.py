#!/usr/bin/env python

from p2 import Parser

class Instr8:
	ops0 = {
	    "nop": "00",
	    "hlt":  "76",
	    "ei":   "fb",
	    "di":   "f3",
	    "sphl": "f9",
	    "xchg": "eb",
	    "xthl": "e3",
	    "daa":  "27",
	    "cma":  "2f",
	    "stc":  "37",
	    "cmc":  "3f",
	    "rlc":  "07",
	    "rrc":  "0f",
	    "ral":  "17",
	    "rar":  "1f",
	    "pchl": "e9",
	    "ret":  "c9",
	    "rnz":  "c0",
	    "rz":   "c8",
	    "rnc":  "d0",
	    "rc":   "d8",
	    "rpo":  "e0",
	    "rpe":  "e8",
	    "rp":   "f0",
	    "rm":   "f8"}

	opsIm16 = {
	    "lda":  "3a",
	    "sta":  "32",
	    "lhld": "2a",
	    "shld": "22",
	    "jmp":  "c3",
	    "jnz":  "c2",
	    "jz":   "ca",
	    "jnc":  "d2",
	    "jc":   "da",
	    "jpo":  "e2",
	    "jpe":  "ea",
	    "jp":   "f2",
	    "jm":   "fa",
	    "call": "cd",
	    "cnz":  "c4",
	    "cz":   "cc",
	    "cnc":  "d4",
	    "cc":   "dc",
	    "cpo":  "e4",
	    "cpe":  "ec",
	    "cp":   "f4",
	    "cm":   "fc"}

	# lxi rp, im16
	opsRpIm16 = {
	    "lxi":  "01"    # 00rp0001, bc=00, de=01,hl=10, sp=11
	    }


	# adi 33, out 10
	opsIm8 = {
	    "adi":  "c6",
	    "aci":  "ce",
	    "sui":  "d6",
	    "sbi":  "de",
	    "ani":  "e6",
	    "xri":  "ee",
	    "ori":  "f6",
	    "cpi":  "fe",
	    "in":   "0db",
	    "out":  "d3"}


	opsRegIm8 = {
	    "mvi":  "06"}

	opsRegReg = {
	    "mov":  "40"}

	opsReg = {
	    "add": "80", # regsrc
	    "adc": "88",
	    "sub": "90",
	    "sbb": "98",
	    "ana": "a0",
	    "xra": "a8",
	    "ora": "b0",
	    "cmp": "b8",

	    "inr": "04", # regdst (<<3)
	    "dcr": "05"}

	opsRp = {
	    "ldax": "0A", # rp << 4 (only B, D)
	    "stax": "02", # rp << 4 (only B, D)
	    "dad":  "09", # rp << 4
	    "inx":  "03", # rp << 4
	    "dcx":  "0b", # rp << 4
	    "push": "c5", # rp << 4
	    "pop":  "c1"  # rp << 4
	    }

	@staticmethod
	def AllMnemonics():
		return reduce(lambda x, y: x + y, 
				[x.keys() for x in 
					[Instr8.ops0, Instr8.opsIm16, Instr8.opsRpIm16, Instr8.opsIm8, Instr8.opsRegIm8, Instr8.opsRegReg, Instr8.opsReg, Instr8.opsRp]])

class Pi8(Parser):	
	AllMnemonics = []

	def __init__(self):
		self.AllMnemonics = Instr8.AllMnemonics()

	def isInstruction(self, str):
		return str in self.AllMnemonics


#parser = Pi8()

#print parser.Parse("mvi a, '$'")