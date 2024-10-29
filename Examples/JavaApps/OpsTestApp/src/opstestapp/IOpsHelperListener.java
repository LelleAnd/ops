/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package opstestapp;

import ops.OPSObject;
import ops.protocol.OPSMessage;

/**
 *
 * @author Lelle
 */
public interface IOpsHelperListener extends ILogListener
{
    void OnData(String topName, OPSMessage msg, OPSObject arg);
}
