/*
 * Таблицы.java
 *
 * Created on 10 Октябрь 2007 г., 13:00
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.svo.vector2utf;

/**
 *
 * @author svo
 */
public final class Таблицы {
    public final static String ВОВА_ВЕКТОР = "vv";
    public final static String КОИ8_Р = "koi8";
    
    
    private Таблицы() {
    }
    
    public static Таблицы этсамое() {
        if (нашиТаблицы == null) {
            нашиТаблицы = new Таблицы();
        }
        return нашиТаблицы;
    }
    
    public char[][] взятьТаблицуПоМановениюПалочки(String палочка) {
        char[][] результатт;
        
        результатт = КОИ8_Р.equalsIgnoreCase(палочка) ? таблица_как_в_кои8 :
                     ВОВА_ВЕКТОР.equalsIgnoreCase(палочка) ? таблица_как_во_ВладимирВекторе :
                         таблица_как_во_ВладимирВекторе;
        
        return результатт;
    }
    
    public String взятьКодировкуПоМановениюПалочки(String палочка) {
        return КОИ8_Р.equalsIgnoreCase(палочка) ? "koi8-r" :
                            ВОВА_ВЕКТОР.equalsIgnoreCase(палочка) ? "cp866" :
                                "cp866";
    }

    private final static char[][] таблица_как_в_кои8 = {
        {'\u253c','\u2551'},
        {'\u00b2', '\u2550'},
        // double corners
        {'\u2265', '\u2554'},
        {'\u2580', '\u2557'},
        {'\u2264','\u255a'},
        {'\u2584','\u255d'},
        
        {'\u25a0', '\u2500'},
        {'\u2510', '\u2502'},
        {'\u2553', '\u2558'},
        {'\u258c', '\u255b'},
        {'\u2551', '\u2564'},
        {'\u2558', '\u2518'},
        {'\u251c', '\u2562'},
        {'\u2248', '\u255f'},
        
        {'\u2554', '\u2552'},
        {'\u252c', '\u2555'},
        {'\u2514', '\u2524'},
        {'\u2559', '\u250c'},
        {'\u2591', '\u2514'},
        {'\u2593', '\u252c'},
        
        // жыр
        {'\u255a', '\u2588'},
        {'\u255d', '\u2590'},
        {'\u255c', '\u258c'},
        
        // особые и странные буквы
        {'\u256a', '№'},
        {'\u2561', '/'},
        {'Ё'     , '\\'},
        {'\u2563', '\\'},
        {'\u2562', '/'},
        {'\u256b', '\u00a9'},
        {'\u256c', '\u263b'},
        
        // белки и стрелки
        {'\u2567', '\u2193',},
        {'\u2566', '\u2191',},
        {'\u2564', '\u2192',},
        {'\u2565', '\u2190'},
                 
        
        // буква Ё
        {'\u2560', 'ё'},
        
    };
    
    
    private final static char[][] таблица_как_во_ВладимирВекторе = 
    {
        {'Є', '\u2554'},       // double UL
        {'\u2580','\u2557'},   // double UR
        {'є', '\u255a'},       // double BL
        {'\u2584', '\u255d'},  // double BR
        {'\u253c', '\u2551'},  // double ||
        {'¤', '\u2550'},       // double =  
        {'\u2500', '\u2588'},   // 25% grey

        {'\u2559', '\u250c'},       // single UL
        {'\u2590','\u2510'},   // single UR
        {'\u2591', '\u2514'},       // single BL
        {'\u2558', '\u2518'},  // single BR
        
        
        {'\u25a0', '\u2500'},   // single --
        {'\u2510', '\u2502'},   // single |
        {'\u2593', '\u252c'},   // single T
        {'\u2592', '\u2534'},   // inverted single T
        {'\u2219', '\u253c'},   // single +
        
        {'Ї',      '\u251c'},   // single T-90 deg
        {'\u2514', '\u2524'},   // single T+90 deg
        
        {'\u2551', '\u2564'},   // double horizontal single vertical T
        {'\u251c', '\u2562'},   // single horizontal double vertical T-90
        {'ў',      '\u255f'},   // single horizontal double vertical T+90
        {'Ў',      '\u2567'},   // double horizontal single vertical T inverted
        
        {'\u00b0', '\u2560'},
        {'\u2557', '\u256a'},
        {'\u2534', '\u2563'},
        
        {'\u255a',  '\u2593'},
        {'\u255b',  '\u2591'},
        {'\u255e',  '\u2592'},
    };
    
    private static Таблицы нашиТаблицы = null;
}
