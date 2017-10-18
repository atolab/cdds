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

public abstract class MetaNode {
    String doc;
    String version;
    String dimension;
    
    public MetaNode(String doc, String version, String dimension) {
        this.doc = doc;
        this.version = version;
        this.dimension = dimension;
    }

    public String getDoc() {
        return doc;
    }

    public void setDoc(String doc) {
        this.doc = doc;
    }
    
    public String getVersion() {
        return this.version;
    }

    public String getDimension() {
        return this.dimension;
    }
}
