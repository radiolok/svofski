import java.text.*;
import java.util.*;
import java.io.*;

public class FC {
	public static void main(String[] argv) throws Exception {
		DataInputStream dis = new DataInputStream(new FileInputStream(argv[0]));
		
		byte[] input = new byte[4096];
		int bytesCount = dis.read(input);
		
		System.err.println("Read " + bytesCount + " bytes");
		
		byte[] vertical = new byte[5];
		
		System.out.println("#include<inttypes.h>\n#include <avr/pgmspace.h>\nconst prog_uint8_t charrom[] = {");
		
		for (int i = 0; i < bytesCount; i+=7) {
			
			System.out.print("    ");
			
			int mask = 0x80;
			for (int v = 0; v < 5; v++) {
				vertical[v] = 0;
				for (int h = 0; h < 7; h++) {
					vertical[v] |= ((input[i+6-h] & mask) != 0 ? 1 : 0) << h;
				}
				mask >>= 1;

				System.out.print("0x" + Integer.toHexString(0xff & vertical[v]) + ", ");
			}
			System.out.println((i/7 < 32 || i/7>127) ? "" : ("\t\t// " + (char)(i/7))  + "-");
		}
		System.out.println("};");
	}
}