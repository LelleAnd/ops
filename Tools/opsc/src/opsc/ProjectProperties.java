/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package opsc;

//import com.sun.corba.se.impl.orbutil.GetPropertyAction;
import configlib.ArchiverInOut;
import configlib.Serializable;
import configlib.SerializableFactory;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

/**
 *
 * @author angr
 */
public class ProjectProperties implements Serializable
{
    public boolean generatePython = true;
    public boolean generateCpp = true;
    public boolean generateJava = true;
    public boolean buildJava = false;
    public boolean generateCS = true;
    public boolean buildCS = true;
    public boolean buildDebugProject = true;
    public String defaultOPSTopicConfigFile = "src/ops_config.xml";
    public Vector<JarDependency> javaBuildJarDependencies = new Vector<JarDependency>();
    public Vector<JarDependency> csBuildDllDependencies = new Vector<JarDependency>();
    public boolean generateOPSConfigClass = false;
    public boolean opsConfigClassNamespace = false;
    public boolean generateOPSConfigXMLFileFromSourceComments = false;
    public boolean addChecksumToConfig = false;
    public String debugProjDomainID = "FooDomain";
    public String vsExampleTopicName = "";
    public String vsExampleDataType = "";
    public String vsExampleDomainID = "";
    public boolean vsExampleEnabled = false;
    private Vector<Property> properties = new Vector<Property>();

    public void serialize(ArchiverInOut archiver) throws IOException
    {
        generatePython = archiver.inout("generatePython", generatePython);
        generateCpp = archiver.inout("generateCpp", generateCpp);
        generateJava = archiver.inout("generateJava", generateJava);
        //System.out.println("buildJava is " + buildJava);
        buildJava = archiver.inout("buildJava", buildJava);
        //System.out.println("buildJava is " + buildJava);
        generateCS = archiver.inout("generateCS", generateCS);
        buildCS = archiver.inout("buildCS", buildCS);
        buildDebugProject = archiver.inout("buildDebugProject", buildDebugProject);
        debugProjDomainID = archiver.inout("debugProjDomainID", debugProjDomainID);
        defaultOPSTopicConfigFile = archiver.inout("defaultOPSTopicConfigFile", defaultOPSTopicConfigFile);
        generateOPSConfigClass = archiver.inout("generateOPSConfigClass", generateOPSConfigClass);
        javaBuildJarDependencies = (Vector<JarDependency>) archiver.inoutSerializableList("javaBuildJarDependencies", javaBuildJarDependencies);
        csBuildDllDependencies = (Vector<JarDependency>) archiver.inoutSerializableList("csBuildDllDependencies", csBuildDllDependencies);
        vsExampleTopicName = archiver.inout("vsExampleTopicName", vsExampleTopicName);
        vsExampleDataType = archiver.inout("vsExampleDataType", vsExampleDataType);
        vsExampleDomainID = archiver.inout("vsExampleDomainID", vsExampleDomainID);
        vsExampleEnabled = archiver.inout("vsExampleEnabled", vsExampleEnabled);
        properties = (Vector<Property>) archiver.inoutSerializableList("properties", properties);
    }

    public void setProperty(Property p)
    {
        if (!existProperty(p.key))
        {
            properties.add(p);
        } else
        {
            getProperty(p.key).value = p.value;
        }
    }

    public String getPropertyValue(String key, String defaultValue)
    {
        for (Property property : properties)
        {
            if (property.key.equals(key))
            {
                return property.value;
            }
        }
        return defaultValue;
    }
    public Property getProperty(String key)
    {
        for (Property property : properties)
        {
            if (property.key.equals(key))
            {
                return property;
            }
        }
        return null;
    }

    public boolean existProperty(String key)
    {
        for (Property property : properties)
        {
            if (property.key.equals(key))
            {
                return true;
            }
        }
        return false;
    }

    public void list(java.io.PrintStream out) {
        for (Property property : properties)
        {
            out.println(property.key + ": " + property.value);
        }
    }

    public static SerializableFactory getSerializableFactory()
    {
        return new ProjectPropertiesFactory();
    }

    static class ProjectPropertiesFactory implements SerializableFactory
    {

        public Serializable create(String type)
        {
            //System.out.println(">>>>: Factory() type = " + type);
            if (type.equals("opsc.ProjectProperties"))
            {
                return new ProjectProperties();
            }
            if (type.equals("opsc.JarDependency") ||
                type.equals("ops.netbeansmodules.idlsupport.projectproperties.JarDependency"))  // Backward compatibility
            {
                return new JarDependency();
            }
            if (type.equals("opsc.Property") ||
                type.equals("ops.netbeansmodules.idlsupport.projectproperties.Property"))  // Backward compatibility
            {
                return new Property();
            }
            return null;
        }
    }
}
