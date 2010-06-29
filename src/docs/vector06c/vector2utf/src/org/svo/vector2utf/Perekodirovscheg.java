/*
 * Recoder.java
 *
 * Created on 9 ќкт€брь 2007 г., 22:45
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.svo.vector2utf;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.HashMap;

/**
 *
 * @author svo
 */
public class ѕерекодировщег {
    /** Creates a new instance of Recoder */
    public ѕерекодировщег(String inn, String outn, String волшебна€) {
        откеда = inn;
        куда = outn;
        
        инициализироватьс€(“аблицы.этсамое().вз€ть“аблицуѕоћановениюѕалочки(волшебна€));
        входна€ одировка = “аблицы.этсамое().вз€ть одировкуѕоћановениюѕалочки(волшебна€);
    }
    

    private void инициализироватьс€(char[][] табло) {
        for(int i = 0; i < табло.length; i++) {
            мапа.put(табло[i][0], табло[i][1]);
        }
    }
    
    public void открыть‘айлы() throws IOException {
        InputStreamReader иср = new InputStreamReader(new FileInputStream(откеда), входна€ одировка);
        читало = new BufferedReader(иср);
        
        писало = new PrintWriter(куда, "utf-8");
    }
    
    public void работать() throws IOException {
        String строка;
        
        while ((строка = читало.readLine()) != null) {
            писало.println(переху€чить—троку(строка));
        }
        System.err.println("Ѕсе нах");
    }
    
    public void всЄ() throws IOException {
        try {
            читало.close();
            писало.close();
        } catch (Exception ex) {
            throw new IOException("Ќе смогла € закрыть файл..");
        }
    }
    
    private String переху€чить—троку(String вход) {
        StringBuffer буфар = new StringBuffer();
        for (int i = 0; i < вход.length(); i++) {
            char c = вход.charAt(i);
            char q = мапа.containsKey(c) ? мапа.get(c) : c;
            буфар.append(q);
        }
        
        return буфар.toString();
    }
    
    private HashMap<Character,Character> мапа = new HashMap<Character,Character>();
    private String откеда, куда;
    private BufferedReader читало = null;
    private PrintWriter писало = null;
    private String входна€ одировка = "cp866";
}

