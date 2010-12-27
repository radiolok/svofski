/*
 * Main.java
 *
 * Created on 9 Октябрь 2007 г., 22:41
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.svo.vector2utf;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/**
 *
 * @author svo
 */
public class Главный {
    public Главный() {
    }
    
   public static void main(String[] args) {
        if (args.length < 2) {
            нах();
        }
        
        String inputFile = args[0];
        String outputFile = args[1];
        
        String магия = Таблицы.ВОВА_ВЕКТОР;
        if (args.length == 3) {
            магия = args[2];
        }

        Перекодировщег пепе = new Перекодировщег(inputFile, outputFile, магия);
        try {
            пепе.открытьФайлы();
        } catch (IOException бля) {
            нах("Экскепшн при открытии файлов" + бля.getLocalizedMessage());
        }
        try {
            пепе.работать();
            пепе.всё();
        } catch (IOException фпесту) {
            нах("Работать не получилось, потому что " + фпесту);
        }
        
        System.exit(0);        
    }
    
    private static void нах() {
        System.err.println("Usage: vvp <input file> <output file> <magic>");
        System.err.println("\tmagic:\t(none):\trecode as per Vladimir Vector (only funny block drawings)");
        System.err.println("\t\tkoi8:\trecode from koi8");
        System.exit(1);
    }
    
    private static void нах(String doh) {
        System.err.println(doh);
        System.exit(1);
    }
    
    
}
