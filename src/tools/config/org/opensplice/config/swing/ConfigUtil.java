/*
 * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
package org.opensplice.config.swing;

import org.opensplice.config.data.DataAttribute;
import org.opensplice.config.data.DataElement;
import org.opensplice.config.data.DataNode;
import org.opensplice.config.meta.MetaAttribute;
import org.opensplice.config.meta.MetaElement;
import org.opensplice.config.meta.MetaNode;

public class ConfigUtil {
    public static String getExtendedDataElementString(DataElement element){
        String firstAttrChild;
        DataNode[] children;
        String name;
        String result = null;
        
        if(element != null){
            children = element.getChildren();
            firstAttrChild = null;
            
            for(int j=0; (j<children.length) && (firstAttrChild == null); j++){
                if(children[j] instanceof DataAttribute){
                    name = ((MetaAttribute)children[j].getMetadata()).getName();
                    
                    if("name".equalsIgnoreCase(name)){
                        firstAttrChild = "[" + ((MetaAttribute)children[j].getMetadata()).getName() + "=";
                        firstAttrChild += ((DataAttribute)children[j]).getValue() + "]";
                    }
                }
            }
            
            for(int j=0; (j<children.length) && (firstAttrChild == null); j++){
                if(children[j] instanceof DataAttribute){
                    firstAttrChild = "[" + ((MetaAttribute)children[j].getMetadata()).getName() + "=";
                    firstAttrChild += ((DataAttribute)children[j]).getValue() + "]";
                }
            }
            if(firstAttrChild == null){
                firstAttrChild = "";
            }
            result = ((MetaElement)element.getMetadata()).getName() + firstAttrChild; 
        }
        return result;
    }
    
    public static String getDataElementString(DataElement element){
        String result = null;
        
        if(element != null){
            result = ((MetaElement)element.getMetadata()).getName();
        }
        return result;
    }
    
    public static String getExtendedMetaElementString(MetaElement element){
        String firstAttrChild;
        MetaNode[] children;
        String result = null;
        
        if(element != null){
            children = element.getChildren();
            firstAttrChild = null;
            
            for(int j=0; (j<children.length) && (firstAttrChild == null); j++){
                if(children[j] instanceof MetaAttribute){
                    firstAttrChild = "[" + ((MetaAttribute)children[j]).getName() + "]";
                }
            }
            if(firstAttrChild == null){
                firstAttrChild = "";
            }
            result = element.getName() + firstAttrChild; 
        }
        return result;
    }
    
    public static String getMetaElementString(MetaElement element){
        String result = null;
        
        if(element != null){
            result = element.getName();
        }
        return result;
    }
}
