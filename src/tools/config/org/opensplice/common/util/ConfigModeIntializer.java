/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2015 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.common.util;

public class ConfigModeIntializer {
    
    public static final String COMMUNITY = "COMMUNITY";
    public static final String COMMERCIAL = "COMMERCIAL";
    public static final int  COMMUNITY_MODE = 1;
    public static final int  COMMERCIAL_MODE = 2;
    public static final int  COMMUNITY_MODE_FILE_OPEN = 3;
    public static final int LITE_MODE = 4;
    public static int  CONFIGURATOR_MODE = COMMERCIAL_MODE;

    public static void setMode(int mode) {
        CONFIGURATOR_MODE = mode;
    }

    public int getMode() {
        return CONFIGURATOR_MODE;
    }

}
