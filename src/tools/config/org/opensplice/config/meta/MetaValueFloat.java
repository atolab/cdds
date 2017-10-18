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
package org.opensplice.config.meta;

public class MetaValueFloat extends MetaValueNatural {
    public MetaValueFloat(String doc, Float defaultValue, Float maxValue,
            Float minValue, String dimension) {
        super(doc, defaultValue, maxValue, minValue, dimension);
    }

    @Override
    public boolean setMaxValue(Object maxValue) {
        boolean result;
        
        if(maxValue instanceof Float){
            this.maxValue = maxValue;
            result = true;
        } else {
            result = false;
        }
        return result;
    }

    @Override
    public boolean setMinValue(Object minValue) {
        boolean result;
        
        if(minValue instanceof Float){
            this.minValue = minValue;
            result = true;
        } else {
            result = false;
        }
        return result;
    }

    @Override
    public boolean setDefaultValue(Object defaultValue) {
        boolean result;
        
        if(defaultValue instanceof Float){
            this.defaultValue = defaultValue;
            result = true;
        } else {
            result = false;
        }
        return result;
    }
}
