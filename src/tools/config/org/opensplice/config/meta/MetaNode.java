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
