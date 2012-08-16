//
// Pretty 8080 Assembler
// 
// Send comments to svofski at gmail dit com 
// 
// Copyright (c) 2009 Viacheslav Slavinsky
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//
// Translation help:
// Leonid Kirillov, Alexander Timoshenko, Upi Tamminen,
// Cristopher Green, Nard Awater, Ali Asadzadeh,
// Guillermo S. Romero, Anna Merkulova, Stephan Henningsen
// 
// Revison Log
// Rev.A: Initial release
// Rev.B: A lot of fixes to compile TINIDISK.ASM by Dr. Li-Chen Wang
// Rev.C: Performance optimizations and cleanup, labels->hash
// Rev.D: More syntax fixes; opera navigation and Back Button Toolbar
// Rev.E: Navigation to label references (backref menu)
//        Nice labels table
//        Some Opera-related fixes
// Rev.F: fixed '.' and semi-colon in db
//        tab scroll fixed
// Rev.G: $ can now work as hex prefix
// Rev.H: Fixed spaces in reg-reg, .binfile, .hexfile
// Rev.I: Fixed bug in evaluation of hex literals ending with d
//
// TODO: evaluation should ignore precedence, it's all left-to-right
//

// -- global DOM elements

var debug = false;

var debugOut = '';

var inTheOpera = false;

var binFileName = 'test.bin';
var hexFileName = 'test.hex';
var downloadFormat = 'bin';
var objCopy = 'gobjcopy';
var postbuild = '';
var doHexDump = true;

// -- utility stuffs --
function fromBinary(val) {
    x = 0;
    n = 1;
    for (i = val.length - 1; i >= 0; i--) {
        if (val[i] == '1')
            x += n;
        else if (val[i] != '0') 
            return Number.NaN;
        n *= 2;
    }

    return new Number(x);
}

function char8(val) {
    if (val > 32 && val < 127) return String.fromCharCode(val);
    return '.';
}

function hex8(val) {
    if (val < 0 || val > 255)  return "??";

    var hexstr = "0123456789ABCDEF";
    return hexstr[(val & 0xf0) >> 4] + hexstr[val & 0x0f];
}

function hex16(val) {
	return hex8((val & 0xff00) >> 8) + hex8(val & 0x00ff);
}

function isValidIm16(s) {
	return s != null && s.length > 0;
}

function isValidIm8(s) {
	return s != null && s.length > 0;
}

function isWhitespace(c) {
    return c=='\t' || c == ' ';// this is too slow c.match(/\s/);
}

Array.prototype.indexOf = function (element) {
    for (var i = 0; i < this.length; i++) {
          if (this[i] == element) {
              return i;
          }
    }
    return -1;
};

String.prototype.trim = function() { return this.replace(/^\s+|\s+$/g, ''); };
String.prototype.endsWith = function(c) { return this[this.length-1] == c; };

// -- Assembler --

var ops0 = {
"nop": "00",
"hlt":	"76",
"ei":	"fb",
"di":	"f3",
"sphl":	"f9",
"xchg":	"eb",
"xthl":	"e3",
"daa":	"27",
"cma":	"2f",
"stc":	"37",
"cmc":	"3f",
"rlc":	"07",
"rrc":	"0f",
"ral":	"17",
"rar":	"1f",
"pchl":	"e9",
"ret":	"c9",
"rnz":	"c0",
"rz":	"c8",
"rnc":	"d0",
"rc":	"d8",
"rpo":	"e0",
"rpe":	"e8",
"rp":	"f0",
"rm":	"f8"
};

var opsIm16 = {
"lda":	"3a",
"sta":	"32",
"lhld":	"2a",
"shld":	"22",
"jmp":	"c3",
"jnz":	"c2",
"jz":	"ca",
"jnc":	"d2",
"jc":	"da",
"jpo":	"e2",
"jpe":	"ea",
"jp":	"f2",
"jm":	"fa",
"call":	"cd",
"cnz":	"c4",
"cz":	"cc",
"cnc":	"d4",
"cc":	"dc",
"cpo":	"e4",
"cpe":	"ec",
"cp":	"f4",
"cm":	"fc"
};

// lxi rp, im16
var opsRpIm16 = {
"lxi":	"01"	// 00rp0001, bc=00, de=01,hl=10, sp=11
};

// adi 33, out 10
var opsIm8 = {
"adi": 	"c6",
"aci": 	"ce",
"sui":	"d6",
"sbi":	"de",
"ani":	"e6",
"xri":	"ee",
"ori":	"f6",
"cpi":	"fe",
"in":	"0db",
"out": 	"d3"
};

var opsRegIm8 = {
"mvi": 	"06"
};

var opsRegReg = {
"mov": 	"40"
};

var opsReg = {
"add": "80", // regsrc
"adc": "88",
"sub": "90",
"sbb": "98",
"ana": "a0",
"xra": "a8",
"ora": "b0",
"cmp": "b8",

"inr": "04", // regdst (<<3)
"dcr": "05"
};

// these are the direct register ops, regdst
var opsRegDst = new Array("inr", "dcr");

var opsRp = {
"ldax": "0A", // rp << 4 (only B, D)
"stax": "02", // rp << 4 (only B, D)
"dad":  "09", // rp << 4
"inx":  "03", // rp << 4
"dcx":  "0b", // rp << 4
"push": "c5", // rp << 4
"pop":  "c1" // rp << 4
};


var LabelsCount = 0;
var labels = new Object();

var resolveTable = Array(); // label negative id, resolved address
var mem = Array();
var textlabels = Array();
var references = Array();
var errors = Array();

function clearLabels() {
    LabelsCount = 0;
    labels = new Object();
}

function resolveNumber(identifier) {
    if (identifier == undefined || identifier.length == 0) return;
    
    if ((identifier[0] == "'" || identifier[0] == "'")
        && identifier.length == 3) {
        return (0xff & identifier.charCodeAt(1));
    }

    if (identifier[0] == '$') {
        identifier = "0x" + identifier.substr(1, identifier.length-1);
    }

	if ("0123456789".indexOf(identifier[0]) != -1) {
        var test;
		test = new Number(identifier);
		if (!isNaN(test)) {
			return test;
		}

        var suffix = identifier[identifier.length-1].toLowerCase();
        switch (suffix) {
        case 'd':
            test = new Number(identifier.substr(0, identifier.length-1));
            if (!isNaN(test)) {
                return test;
            }
            break;
        case 'h':
			test = new Number("0x" + identifier.substr(0, identifier.length-1));
			if (!isNaN(test)) {
				return test;
			}
            break;
        case 'b':
			test = fromBinary(identifier.substr(0, identifier.length-1));
			if (!isNaN(test)) {
				return test;
			}
            break;
        case 'q':
            try {
                var oct = identifier.substr(0, identifier.length-1);
                for (var i = oct.length; --i >= 0;) {
                    if (oct[i] == '8' || oct[i] == '9') return -1;
                }
                return new Number(
                    eval('0' + identifier.substr(0, identifier.length-1)));
            } catch(err) {}
            break;
        }
	}
	return -1;
}

function referencesLabel(identifier, linenumber) {
    identifier = identifier.toLowerCase();
    if (references[linenumber] == undefined) {
        references[linenumber] = identifier;
    }
}

function markLabel(identifier, address, linenumber, override) {
    identifier = identifier.replace(/\$([0-9a-fA-F]+)/, '0x$1');
    identifier = identifier.replace(/(^|[^'])(\$|\.)/, ' '+address+' ');
	var number = resolveNumber(identifier.trim());
	if (number != -1) return number;
	
	if (linenumber == undefined) {
        LabelsCount++;
		address = -1 - LabelsCount;
	}

    identifier = identifier.toLowerCase();
	
	var found = labels[identifier];
    if (found != undefined) {
        if (address >= 0) {
            resolveTable[-found] = address;
        } else {
            address = found;
        }
    }

	if (!found || override) {
        labels[identifier] = address;
	}

    if (linenumber != undefined) {
        textlabels[linenumber] = identifier;
    }
	
	return address;
}

function setmem16(addr, immediate) {
	if (immediate >= 0) {
		mem[addr] = immediate & 0xff;
		mem[addr+1] = immediate >> 8;
	} else {
		mem[addr] = immediate;
		mem[addr+1] = immediate;
	}
}

function setmem8(addr, immediate) {
	mem[addr] = immediate < 0 ? immediate : immediate & 0xff;
}

function parseRegisterPair(s) {
    if (s != undefined) {
        s = s.split(';')[0].toLowerCase();
    	if (s == 'b' || s == 'bc') return 0;
    	if (s == 'd' || s == 'de') return 1;
     	if (s == 'h' || s == 'hl') return 2;
          	if (s == 'sp'|| s == 'psw' || s == 'a') return 3;
    }
	return -1;
}

// b=000, c=001, d=010, e=011, h=100, l=101, m=110, a=111
function parseRegister(s) {
    if (s == undefined) return -1;
    s = s.toLowerCase();
	return "bcdehlma".indexOf(s[0]);
}

function tokenDBDW(s, addr, len, linenumber) {
    var size = -1;

    if (s.length == 0) return 0;

    n = markLabel(s, addr);
    referencesLabel(s, linenumber);

    if (len == undefined) len = 1;

    if (len == 1 && n < 256) {
        setmem8(addr, n);
        size = 1;
    } else if (len == 2 && n < 65536) {
        setmem16(addr, n); 
        size = 2;
    }

    return size;
}

function tokenString(s, addr, linenumber) {
    for (var i = 0; i < s.length; i++) {
        setmem8(addr+i, s.charCodeAt(i));
    }
    return s.length;
}

function parseDeclDB(args, addr, linenumber, dw) {
    var text = args.slice(1).join(' ');
    var arg = "";
    var mode = 0;
    var cork = false;
    var nbytes = 0;

    for (var i = 0; i < text.length; i++) {
        switch (mode) {
        case 0:
            if (text[i] == '"' || text[i] == "'") {
                mode = 1; cork = text[i];
                break;
            } else if (text[i] == ',') {
                var len = tokenDBDW(arg, addr+nbytes, dw, linenumber);
                if (len < 0) {
                    return -1;
                }
                nbytes += len;
                arg = "";
            } else if (text[i] == ';') {
                i = text.length;
                break;
            } else {
                arg += text[i];
            }
            break;
        case 1:
            if (text[i] != cork) {
                arg += text[i]; 
            } else {
                cork = false;
                mode = 0;
                len = tokenString(arg, addr+nbytes, linenumber);
                if (len < 0) {
                    return -1;
                }
                nbytes += len;
                arg = "";
            }
            break; 
        }
    }
    if (mode == 1) return -1;    // unterminated string
    var len = tokenDBDW(arg, addr+nbytes, dw, linenumber);
    if (len < 0) return -1;
    nbytes += len;

    return nbytes;
}

function getExpr(arr) {
    var ex = arr.join(' ').trim();
    if (ex[0] == '"' || ex[0] == "'") {
        return ex;
    }
    return ex.split(';')[0];
}

function useExpr(s, addr, linenumber) {
    var expr = getExpr(s);
    if (expr == undefined || expr.trim().length == 0) return false;

    var immediate = markLabel(expr, addr);
    referencesLabel(expr, linenumber);
    return immediate;
}

function parseInstruction(s, addr, linenumber) {
    var parts = s.split(/\s+/);
		
	for (var i = 0; i < parts.length; i++) {
		if (parts[i][0] == ';') {
			parts.length = i;
			break;
		}
	}
    
    var labelTag;
    var immediate;

	for (;parts.length > 0;) {
		var opcs;
	    var mnemonic = parts[0].toLowerCase();

        if (mnemonic.length == 0) {
            parts = parts.slice(1);
            continue;
        }

		// no operands
		if ((opcs = ops0[mnemonic]) != undefined) {
			mem[addr] = new Number("0x" + opcs);
			return 1;
		}
		
		// immediate word
		if ((opcs = opsIm16[mnemonic]) != undefined) {
			mem[addr] = new Number("0x" + opcs);

            immediate = useExpr(parts.slice(1), addr, linenumber);
            //if (!immediate) return -3;

			setmem16(addr+1, immediate);
			return 3;
		}
		
		// register pair <- immediate
		if ((opcs = opsRpIm16[mnemonic]) != undefined) {
			subparts = parts.slice(1).join(" ").split(",");
			if (subparts.length < 2) return -3;
			rp = parseRegisterPair(subparts[0]);
			if (rp == -1) return -3;

			mem[addr] = (new Number("0x" + opcs)) | (rp << 4);

            immediate = useExpr(subparts.slice(1), addr, linenumber);

			setmem16(addr+1, immediate);
			return 3;
		}

		// immediate byte		
		if ((opcs = opsIm8[mnemonic]) != undefined) {
			mem[addr] = new Number("0x" + opcs);
            immediate = useExpr(parts.slice(1), addr, linenumber);
            //if (!immediate) return -2;
            setmem8(addr+1, immediate);
            return 2;
		}

		// single register, im8
		if ((opcs = opsRegIm8[mnemonic]) != undefined) {
			subparts = parts.slice(1).join(" ").split(",");
			if (subparts.length < 2) return -2;
			reg = parseRegister(subparts[0]);
			if (reg == -1) return -2;

			mem[addr] = new Number("0x" + opcs) | reg << 3;

            immediate = useExpr(subparts.slice(1), addr, linenumber);

            setmem8(addr+1, immediate);
			return 2;			
		}
				
		// dual register (mov)
		if ((opcs = opsRegReg[mnemonic]) != undefined) {
			subparts = parts.slice(1).join(" ").split(",");
			if (subparts.length < 2) return -1;
			reg1 = parseRegister(subparts[0].trim());
			reg2 = parseRegister(subparts[1].trim());
			if (reg1 == -1 || reg2 == -1) return -1;
			mem[addr] = new Number("0x" + opcs) | reg1 << 3 | reg2;
			return 1;
		}

		// single register
		if ((opcs = opsReg[mnemonic]) != undefined) {
			reg = parseRegister(parts[1]);
			if (reg == -1) return -1;
			
			if (opsRegDst.indexOf(mnemonic) != -1) {
				reg <<= 3;
			}
			
			mem[addr] = new Number("0x" + opcs) | reg;
			return 1;
		}
		
		// single register pair
		if ((opcs = opsRp[mnemonic]) != undefined) {
			rp = parseRegisterPair(parts[1]);
			if (rp == -1) return -1;
			mem[addr] = new Number("0x" + opcs) | rp << 4;
			return 1;
		}		
		
		// rst
		if (mnemonic == "rst") {
			n = resolveNumber(parts[1]);
			if (n >= 0 && n < 8) {
				mem[addr] = 0xC7 | n << 3;
				return 1;
			}
			return -1;
		}
		
		if (mnemonic == ".org" || mnemonic == "org") {
			n = resolveNumber(parts[1]);
			if (n >= 0) {
				return -100000-n;
			}
			return -1;
		}

        if (mnemonic == ".binfile") {
            if (parts[1] != undefined && parts[1].trim().length > 0) {
                binFileName = parts[1];
            }
            return -100000;
        }

        if (mnemonic == ".hexfile") {
            if (parts[1] != undefined && parts[1].trim().length > 0) {
                hexFileName = parts[1];
            }
            return -100000;
        }

        if (mnemonic == ".download") {
            if (parts[1] != undefined && parts[1].trim().length > 0) {
                downloadFormat = parts[1].trim();
            }
            return -100000;
        }

        if (mnemonic == ".objcopy") {
           objCopy = parts.slice(1).join(' '); 
           return -100000;
        }

        if (mnemonic == ".postbuild") {
            postbuild = parts.slice(1).join(' ');
            return -100000;
        }

        if (mnemonic == ".nodump") {
            doHexDump = false;
            return -100000;
        }

        // assign immediate value to label
        if (mnemonic == ".equ" || mnemonic == "equ") {
            if (labelTag == undefined) return -1;
            var value = evaluateExpression(parts.slice(1).join(' '), addr);
            markLabel(labelTag, value, linenumber, true);
            return 0;
        }

        if (mnemonic == 'cpu' ||
            mnemonic == 'aseg' ||
            mnemonic == '.aseg') return 0;

        if (mnemonic == 'db' || mnemonic == '.db' || mnemonic == 'str') {
            return parseDeclDB(parts, addr, linenumber, 1);
        }
        if (mnemonic == 'dw' || mnemonic == '.dw') {
            return parseDeclDB(parts, addr, linenumber, 2);
        }
        if (mnemonic == 'ds' || mnemonic == '.ds') {
            var size = evaluateExpression(parts.slice(1).join(' '), addr);
            if (size >= 0) {
                for (var i = 0; i < size; i++) {
                    setmem8(addr+i, 0);
                }
                return size;
            }
            return -1;
        }
		
		if (parts[0][0] == ";") {
			return 0;
		}

        // nothing else works, it must be a label
        if (labelTag == undefined) {
            var splat = mnemonic.split(':');
            labelTag = splat[0];
            markLabel(labelTag, addr, linenumber);

            parts.splice(0, 1, splat.slice(1).join(':'));
            continue;
        }
		
        mem[addr] = -2;
		return -1; // error
	}
	
	return 0; // empty
}


// -- output --

function labelList() {
    labelList.s = "            ";
    labelList.f = function(label, addr) {
        var result = label.substring(0, labelList.s.length);
        if (result.length < labelList.s.length) {
            result += labelList.s.substring(result.length);
        }
        result += addr < 0 ? "????" : hex16(addr);
        return result;
    }

    var sorted = [];
    for (var i in labels) {
        sorted[sorted.length] = i;
    }
    sorted.sort();

    var result = "<pre>Labels:</pre>";
    result += '<div class="hordiv"></div>';
    result += '<pre>';
    var col = 1;
    for (var j = 0; j < sorted.length; j++) {
        var i = sorted[j];
        var label = labels[i];

        // hmm? 
        if (label == undefined) continue;
        if (i.length == 0) continue; // resolved expressions
        result += "<span class='" +
            (col%4 == 0 ? 't2' : 't1') +  
            "' onclick=\"return gotoLabel('"+i+"');\"";
        if (label < 0) result += ' style="background-color:pink;" ';
        result += ">";
        result += labelList.f(i,label);
        result += "</span>";
        if (col % 4 == 0) result += "<br/>";
        col++;
    }
    result += "</pre>";
    
    return result;
}

function dumpspan(org, mode) {
    var result = "";
    var nonempty = false;
    conv = mode ? char8 : hex8;
    for (var i = org; i < org+16; i++) {
        if (mem[i] != undefined) nonempty = true;
        if (mode == 1) {
            result += conv(mem[i]);
        } else {
            result += (i > org && i%8 == 0) ? "-" : " ";
            if (mem[i] == undefined) {
                result += '  ';
            }
            else if (mem[i] < 0) {
                result += '<span class="errorline">' + conv(mem[i]) + '</span>';
            } else {
                result += conv(mem[i]);
            }
        }
    }

    return nonempty ? result : false;
}

function dump() {
	var org;
	for (org = 0; org < mem.length && mem[org] == undefined; org++);
	
	if (org % 16 != 0) org = org - org % 16;
	
    var result = "<pre>Memory dump:</pre>";
    result += '<div class="hordiv"></div>';
    var lastempty;

    var printline = 0;

    for (i = org; i < mem.length; i += 16) {
        span = dumpspan(i, 0);
        if (span || !lastempty) {
		    result += '<pre ' + 'class="d' + (printline++%2) + '"';
            result += ">";
        }
        if (span) {
            result += hex16(i) + ": ";
            result += span;
            result += '  ';
            result += dumpspan(i, 1);
            result += "</pre><br/>";
            lastempty = false;
        } 
        if (!span && !lastempty) {
            result += " </pre><br/>";
            lastempty = true;
        }
    }

    return result;
}

function intelHex() {
    var i, j;
    var line = "";
    var r = "";
    var pureHex = "";
    r += "<pre>Intel HEX:</pre>";
    r += '<div class="hordiv"></div>';

    r += "<pre>";
    r += 'cat &gt;' + hexFileName + ' &lt;&lt;X<br/>';
    //r += 'ed<br>i<br>';
    for (i = 0; i < mem.length;) {
        for (j = i; j < mem.length && mem[j] == undefined; j++);
        i = j;
        if (i >= mem.length) break; 

        line = ":";

        cs = 0;

        rec = "";
        for (j = 0; j < 32 && mem[i+j] != undefined; j++) {
           if (mem[i+j] < 0) mem[i+j] = 0;
           rec += hex8(mem[i+j]); 
           cs += mem[i+j];
        }

        cs += j; line += hex8(j);   // byte count
        cs += (i>>8)&255; cs+=i&255; line += hex16(i);  // record address
        cs += 0; line += "00";      // record type 0, data
        line += rec;

        cs = 0xff&(-(cs&255));
        line += hex8(cs);
        pureHex += line + '\n';
        r += line + '<br/>';

        i += j;
    }
    r += ':00000001FF<br/>';
    pureHex += ':00000001FF\n';
    //r += '.<br>w ' + hexFileName +'<br>q<br>';
    r += 'X<br/>';
    r += objCopy + ' -I ihex ' + hexFileName + ' -O binary ' + 
        binFileName + '<br/>';
    if (postbuild.length > 0) {
        r += postbuild + '<br/>';
    }
    r += '</pre>';

//    var formData = document.getElementById('hex');
//    formData.value = pureHex;
//    var formBinName = document.getElementById('formbinname');
//    formBinName.value = binFileName;
//    var formHexName = document.getElementById('formhexname');
//    formHexName.value = hexFileName;
//   var formDownloadFormat = document.getElementById('downloadformat');
//    formDownloadFormat.value = downloadFormat;

    return pureHex;
}

function getListHeight() {
    var listElement = document.getElementById('list');
    return inTheOpera ? 
        listElement.style.pixelHeight : listElement.offsetHeight;

}

function gotoLabel(label) {
    var sought = textlabels.indexOf(label.toLowerCase());
    var element = document.getElementById("label" + sought);
    if (element != undefined) {
        startHighlighting(sought, element);
        element = element.parentNode;
        var destination = element.offsetTop - getListHeight()/2;
        scrollTo(destination, true);
    }
    return false;
}

function getReferencedLabel(lineno) {
    var refto = references[lineno];
    if (refto != undefined) {
        var sought = textlabels.indexOf(refto.toLowerCase());
        return document.getElementById("label" + sought);
    }
    return undefined;
}

function getReferencingLines(lineno) {
    var refs = new Array();
    var fullrefs = new Array();
    var label = textlabels[lineno];
    if (label != undefined) {
        for(var i = 0; i < references.length; i++) {
            if (references[i] == label) {
                var element = document.getElementById("code" + i);
                refs[refs.length] = element;
                element = document.getElementById("l" + i);
                fullrefs[fullrefs.length] = element;
            }
        }
    }
    referencingLinesFull = fullrefs;
    return refs;
}

function getLabel(l) {
    return labels[l.toLowerCase()];
}

function listing(text,lengths,addresses) {
    var result = "";
    var addr = 0;
    for(var i = 0; i < text.length; i++) {
        var labeltext = "";
        var remainder = text[i];
        var parts = text[i].split(/[\:\s]/);
        if (parts.length > 1) {
            if (getLabel(parts[0]) != -1) {
                labeltext = parts[0];
                remainder = text[i].substring(labeltext.length);
            }
        }

        var id = "l" + i;
        var labelid = "label" + i;
        var remid = "code" + i;

        var hexes = "";
        var unresolved = false;
        var width = 0;

        var len = lengths[i] > 4 ? 4 : lengths[i];
        for (var b = 0; b < len; b++) {
            hexes += hex8(mem[addresses[i]+b]) + ' ';
            width += 3;
            if (mem[addresses[i]+b] < 0) unresolved = true;
        }
        for (b = 0; b < 16 - width; b++) { hexes += ' '; }

        result += '<pre id="' + id + '"' 
        //    +
        //    ' onmouseover="return mouseover('+i+');"' + 
        //    ' onmouseout="return mouseout('+i+');"';

        if (unresolved || errors[i] != undefined) {
            result += ' class="errorline" ';
        }

        result += 
            '>' + (lengths[i] > 0 ? hex16(addresses[i]) : "");
        result += '\t';

        result += hexes;

        if (labeltext.length > 0) {
            var t = '<span class="l" id="' + labelid + '"' +
            ' onmouseover="return mouseovel('+i+');"' + 
            ' onmouseout="return mouseout('+i+');"' +
            '>' + labeltext + '</span>';
            result += t;
        }
        for (b = 0; b < remainder.length && isWhitespace(remainder[b]); b++) {
            result += ' ';
        }
        remainder = remainder.substring(b);
        if (remainder.length > 0) {
            result += '<span id="' + remid + '"' +
            ' onmouseover="return mouseover('+i+');"' + 
            ' onmouseout="return mouseout('+i+');"' +
            '>' + remainder + '</span>';
        }

        // hacked this into displaying only first and last lines
        // of db thingies
        if (len < lengths[i]) {
            result += '<br/>\t.&nbsp;.&nbsp;.&nbsp;<br/>';
            for (var subline = 1; subline*4 < lengths[i]; subline++) {
                var subresult = '';
                subresult += hex16(addresses[i]+subline*4) + '\t';
                for (var sofs = 0; sofs < 4; sofs++) {
                    var adr = subline*4+sofs;
                    if (adr < lengths[i]) {
                        subresult += hex8(mem[addresses[i]+adr]) + ' ';
                    }
                }
                //result += "<br/>";
            }
            result += subresult + "<br/>";
        }
        result += '</pre>';

        addr += lengths[i];
    }

    result += labelList();

    result += "<div>&nbsp;</div>";
    
    if (doHexDump) {
        result += dump();
    }

    result += "<div>&nbsp;</div>";

    result += intelHex();

    result += "<div>&nbsp;</div>";

    return result;
}

function error(line, text) {
    errors[line] = text;
}


function readInput(filename) {
    var src = read(filename);
    var inputlines = src.split('\n');
    var result = "";
    for (var line = 0; line < inputlines.length; line++) {
        var text = inputlines[line];
        var parts = text.split(/\s+/);
            
        for (var i = 0; i < parts.length; i++) {
            if (parts[i][0] == ';') {
                parts.length = i;
                break;
            }
        }

        for (;parts.length > 0;) {
            var directive = parts[0].toLowerCase();
            if (directive.length == 0) {
                parts = parts.slice(1);
                continue;
            }

            if (directive == ".include") {
                if (parts[1] != undefined && parts[1].trim().length > 0) {
                    includeFileName = parts[1];
                    text = "\n;;; include " + includeFileName + " begin\n"
                    text += readInput(includeFileName);
                    text += "\n;;; include " + includeFileName + " end\n"
                }
            }
            break;
        }
        result += text + "\n";
   }

   return result;
}

// assembler main entry point
function assemble(filename) {
    var src = readInput(filename);

    var list = '';
    var lengths = Array();
    var addresses = Array();

    var inputlines = src.split('\n');
    
    var addr = 0;
    clearLabels();
    resolveTable.length = 0;
    mem.length = 0;
    backrefWindow = false;
    references.length = 0;
    textlabels.length = 0;
    errors.length = 0;
    doHexDump = true;
    postbuild = '';
    objCopy = 'gobjcopy';
    
    for (var line = 0; line < inputlines.length; line++) {
		var size = parseInstruction(inputlines[line].trim(), addr, line);
		if (size <= -100000) {
			addr = -size-100000;
			size = 0;
		} else if (size < 0) {
			error(line, "syntax error");
			size = -size;
		}
        lengths[line] = size;
        addresses[line] = addr;
		addr += size;
    }
    
    resolveLabelsTable();
    evaluateLabels();
    resolveLabelsInMem();
    
    list += listing(inputlines, lengths, addresses);

    if (makeListing) {
        print(list);
    } else {
        print(intelHex())
    }
}

function evaluateExpression(input, addr) {
    var q;
    var originput = input;
    //console.log("input=" + input + " addr=" + addr);
    try {
        input = input.replace(/\$([0-9a-fA-F]+)/, '0x$1');
        input = input.replace(/(^|[^'])\$|\./gi, ' '+addr+' ');
        input = input.replace(/([\d\w]+)\s(shr|shl|and|or|xor)\s([\d\w]+)/gi,'($1 $2 $3)');
        input = input.replace(/\b(shl|shr|xor|or|and|[+\-*\/()])\b/gi,
            function(m) {
                switch (m) {
                case 'and':
                    return '&';
                case 'or':
                    return '|';
                case 'xor':
                    return '^';
                case 'shl':
                    return '<<';
                case 'shr':
                    return '>>';
                default:
                    return m;
                }
            });
        q = input.split(/<<|>>|[+\-*\/()\^\&]/);
    } catch (e) {
        return -1;
    }
    input = input;

    var expr = '';
    for (var ident = 0; ident < q.length; ident++) {
        var qident = q[ident].trim();
        if (-1 != resolveNumber(qident)) continue;
        var addr = labels[qident];//.indexOf(qident);
        if (addr != undefined) {
            //addr = labels[idx+1];
            if (addr >= 0) {
                expr += 'var _' + qident + '=' + addr +';\n';
                var rx = new RegExp('\\b'+qident+'\\b', 'gm');
                input = input.replace(rx, '_' + qident);
            } else {
                expr = false;
                break;
            }
        }
    }
    //console.log('0 input=',  input);
    //console.log('1 expr=', expr);
    expr += input.replace(/0x[0-9a-fA-F]+|[0-9][0-9a-fA-F]*[hbqdHBQD]|'.'/g,
        function(m) {
            return resolveNumber(m);
        });
    //console.log('expr=', expr);
    try {
        return eval(expr.toLowerCase());
    } catch (err) {
        //console.log('expr was:',expr.toLowerCase(), originput);
        //console.log(err);
    }

    return -1;
}

function evaluateLabels() {
    for (var i in labels) {
        var label = labels[i];
        if (label < 0 && resolveTable[-label] == undefined) {
            var result = evaluateExpression(i,-1);
            if (result >= 0) {
                resolveTable[-label] = result;
                labels[i] = undefined;
            }
        } 
    }
}

function resolveLabelsInMem() {
    for (var i = 0; i < mem.length;) {
        var negativeId;
        if ((negativeId = mem[i]) < 0) {
            newvalue = resolveTable[-negativeId];

            if (newvalue != undefined) mem[i] = newvalue & 0xff;
            i++;
            if (mem[i] == negativeId) {
                if (newvalue != undefined) mem[i] = 0xff & (newvalue >> 8);
                i++;
            }
        } else {
            i++;
        }
    }
}
 
function resolveLabelsTable(nid) {   
   for (var i in labels) {
        var label = labels[i];
        if (label < 0) {
            var addr = resolveTable[-label];
            if (addr != undefined) {
                labels[i] = addr;
            }
        }
    }
}


assemble(inputFile)
