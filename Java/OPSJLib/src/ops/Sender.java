/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package ops;

import java.io.IOException;
import java.net.InetAddress;

/**
 *
 * @author staxgr
 */
public interface Sender
{
    void open() throws IOException;
    void close();
    int getLocalPort();
    boolean sendTo(byte[] bytes, String ip, int port);
    boolean sendTo(byte[] bytes, int offset, int size, String ip, int port);

    boolean sendTo(byte[] bytes, int offset, int size, InetAddress ipAddress, int port);
}
