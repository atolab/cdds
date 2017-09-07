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
package org.opensplice.common.view.entity;


/**
 * Implementation of EntityInfoFormatter that formats output in HTML
 * format.
 * 
 * @date Jun 7, 2004
 */
public class EntityInfoFormatterHTML extends EntityInfoFormatter{
    
    /**
     * Constructs new formatter that formats data in a HTML representation.
     */
    public EntityInfoFormatterHTML(){
        super();
        tab = "&nbsp;&nbsp;&nbsp;&nbsp;";
        separator = "";
        newLine = "<br>";
    }
    
    /**
     * Creates HTML representation of the supplied object. Object types that 
     * are supported are:
     * - String
     * - MetaType
     * 
     * If the type is not supported, no error will occur.
     */
    @Override
    public synchronized String getValue(Object object){
        curObject = object;
        if(object == null){
            curValue = "<html><body>No selection</body></html>";
        }
//        else if(object instanceof MetaType){
//            this.setUserDataType((MetaType)object);
//        }
        else if(object instanceof String){
            String entityInfo = ((String)object).replaceAll("\\t", "&nbsp;");
            entityInfo = entityInfo.replaceAll("<", "&lt;");
            entityInfo = entityInfo.replaceAll(">", "&gt;");
            entityInfo = entityInfo.replaceAll("\\n", "<br>");
            curValue = "<html><body>" + entityInfo +  "</body></html>";
        }
        else{
            curValue = "<html><body>Cannot show info of " + object.getClass().toString() + "</body></html>";
        }
        return curValue;
    }
}
