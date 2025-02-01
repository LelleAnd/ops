/** -*-java-*- Copyright (C) 2014 Saab Dynamics AB
 ***************************************************************************
 *  @file   opsc/CompilerSupport.java
 *  @author Mattias Helsing <mattias.helsing@saabgroup.com>
 *          Updated/Modified by Lennart Andersson
 *
 * Baseclass for my specialized compilers in this package
 *
 ***************************************************************************
 */

package opsc;

import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Vector;
import java.util.HashSet;
import java.util.Comparator;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.TopicInfo;

/**
 *
 * @author helm
 */
public abstract class CompilerSupport extends AbstractTemplateBasedIDLCompiler
{
    /** Where to find templates. Default is in directory /templates in jar file IDLTemplates.jar */
    private String _templateDir = "templates";
    protected String _outputDir = ".";
    protected String _projectName = "";
    Vector<IDLClass> _idlClasses = new Vector<IDLClass>();
    protected Vector<String> _generatedFiles;

    protected boolean _onlyGenTypeSupport = false;
    protected boolean _genMemoryPool = false;

    HashSet<String> _packageNamesInCompilation = new HashSet<String>();

    /** A verbosity flag. Currently supports 0 or not 0 */
    protected int _verbose = 0;

    public CompilerSupport(String projectName) {
        // set projectname
        _projectName = projectName;
        // tab is 4 spaces
        setTabString("    ");
        // endl is platform specific
        setEndlString(System.getProperty("line.separator"));
    }

    /**
     * @Override from IDLCompiler interface
     * */
    // This is currently not used for our compilers
    // Declare it here so specific compilers don't need to
    public void compileTopicConfig(Vector<TopicInfo> topics, String name, String packageString, String projectDirectory)
    {
    }

    public void setPackageNamesInCompilation(HashSet<String> value)
    {
        _packageNamesInCompilation = new HashSet<String>(value);
    }

    public boolean packageNameInCompilation(String value)
    {
        return _packageNamesInCompilation.contains(value);
    }

    public void setVerbose(int value) {
        _verbose = value;
    }

    public void setGenOnlyTypeSupport(boolean value)
    {
        _onlyGenTypeSupport = value;
    }

    public void setGenMemoryPool(boolean value)
    {
        _genMemoryPool = value;
    }

    public void setTemplateDir(String templatedir) {
        _templateDir = templatedir;
    }

    public void setOutputDir(String dir) {
        /** @todo Check and remove trailing slash */
        _outputDir = dir;
    }

    public void setProjectName(String name) {
        _projectName = name;
    }

    protected void setTemplateTextFromResource(java.io.InputStream stream) throws IOException
    {
        byte[] templateBytes = new byte[stream.available()];
        stream.read(templateBytes);
        setTemplateText(new String(templateBytes));
    }

    protected java.io.InputStream findTemplateFile(String templateName) throws IOException {
        // search for a file called templateName
        File thefile = new File( templateName );

        //System.out.println("Looking for template: " + templateName);

        // if the path is absolute - use as is - no fallbacks
        if (thefile.isAbsolute()) {
            if (thefile.isFile()) {
                //System.out.println("found... " + thefile.getCanonicalPath());
                return new java.io.FileInputStream(thefile.getCanonicalPath());
            }
            throw new IOException("No such file " + templateName);
        }

        // try using _templateDir member
        thefile = new File( _templateDir + File.separator + templateName );
        if (thefile.isFile()) {
            //System.out.println("found... " + thefile.getCanonicalPath());
            return new java.io.FileInputStream(thefile.getCanonicalPath());
        }

        String resource = "/usr/share/ops/templates";
        thefile = new File( resource + File.separator + templateName );
        if (thefile.isFile()) {
            //System.out.println("found... " + thefile.getCanonicalPath());
            return new java.io.FileInputStream(thefile.getCanonicalPath());
        }

        resource = "../../share/ops/templates";
        thefile = new File( resource + File.separator + templateName );
        if (thefile.isFile()) {
            //System.out.println("found... " + thefile.getCanonicalPath());
            return new java.io.FileInputStream(thefile.getCanonicalPath());
        }

        resource = "/templates/" + templateName;
        if (this.getClass().getResource( resource ) != null) {
          //System.out.println("found... " + resource);
          return this.getClass().getResourceAsStream(resource);
        }

        resource = "/NBOPSIDLSupport/src/ops/netbeansmodules/idlsupport/templates/" + templateName;
        if (this.getClass().getResource( resource ) != null) {
          //System.out.println("found... " + resource);
          return this.getClass().getResourceAsStream(resource);
        }

        throw new IOException("No such template " + templateName);
    }

    /**
     * @override
     * */
    protected void saveOutputText(String templateText)
    {
        FileOutputStream fos = null;
        try
        {
            //System.out.println(">>>>: " + outputFileName);
            File outFile = new File(outputFileName);
            //System.out.println(">>>>: " + outFile.getAbsolutePath());
            File outFilePath = new File(outputFileName.substring(0, outputFileName.lastIndexOf(File.separator)));
            //System.out.println(">>>>: " + outFilePath.getAbsolutePath());

            outFilePath.mkdirs();
            outFile.createNewFile();

            fos = new FileOutputStream(outFile);
            fos.write(templateText.getBytes());
        } catch (IOException ex) {
            //System.out.println(">>>>: IOException");
            Logger.getLogger(CompilerSupport.class.getName()).log(Level.SEVERE, null, ex);
        } finally {
            try {
                fos.close();
            } catch (IOException ex) {
                //Logger.getLogger(CppFactoryIDLCompiler.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Directive helpers
    // -----------------

    // Returns true if class is a 'toplevel' class
    public static boolean isTopLevel(IDLClass idlClass)
    {
        String s = idlClass.getDirective();
        if (s == null) return false;
        if (isNoFactory(idlClass)) return false;
        int idx = s.indexOf("toplevel");
        if (idx != -1) {
            int idx2 = s.indexOf(",", idx);
            String sub;
            if (idx2 == -1) {
                sub = s.substring(idx);
            } else {
                sub = s.substring(idx, idx2);
            }
            //System.out.println("Info: isTopLevel(), '" + sub + "',  " + idlClass.getClassName());

            if (sub.toLowerCase().indexOf("false") != -1) return false;
        }
        return true;  //default
    }

    // Returns true if class is an 'onlydefinition' class
    public static boolean isOnlyDefinition(IDLClass idlClass)
    {
        String s = idlClass.getDirective();
        if (s == null) return false;
        //System.out.println("Info: isOnlyDefinition(), class: '" + idlClass.getClassName() + "', getDirective() = " + s);
        int idx = s.indexOf("onlydefinition");
        if (idx != -1) {
            //System.out.println("Info: isOnlyDefinition(), " + idlClass.getClassName());
            return true;
        }
        return false;  //default
    }

    // Returns true if class is a 'nofactory' class
    public static boolean isNoFactory(IDLClass idlClass)
    {
        String s = idlClass.getDirective();
        if (s == null) return false;
        //System.out.println("Info: isNoFactory(), class: '" + idlClass.getClassName() + "', getDirective() = " + s);
        int idx = s.indexOf("nofactory");
        if (idx != -1) {
            //System.out.println("Info: isNoFactory(), " + idlClass.getClassName());
            return true;
        }
        return false;  //default
    }

    //
    public static int validateVersion(String msg, String verStr)
    {
        int v = Integer.parseInt(verStr);
        if ((v < 0) || (v > 255)) {
            System.out.println("Error: " + msg + ". //@version spec invalid (must be within 0..255): '" + verStr + "'");
            System.exit(99);
        }
        return v;
    }

    public static class VersionEntry
    {
        public int start = -1;
        public int stop = -1;
    }

    public static class VersionEntryComparator implements Comparator<VersionEntry>
    {
        public int compare(VersionEntry o1, VersionEntry o2)
        {
            if (o1.start == o2.start) {
                if (o1.stop < 0) return -1;
                if (o2.stop < 0) return 1;
                return o1.stop - o2.stop;
            }
            return o1.start - o2.start;
        }
    }

    public static Vector<VersionEntry> getVersions(String msg, String directiveStr)
    {
        Vector<VersionEntry> vec = null;
        while (directiveStr != null) {
            int idx = directiveStr.indexOf("version = ");
            if (idx == -1) break;
            int idx2 = directiveStr.indexOf(",", idx);
            idx += 10;  // Skip 'version = '
            String sub, sub2;
            if (idx2 == -1) {
                sub = directiveStr.substring(idx);
                directiveStr = null;
            } else {
                sub = directiveStr.substring(idx, idx2);
                directiveStr = directiveStr.substring(idx2+1);
            }
            // Make sure value type is 'int: '
            if (sub.indexOf("int: ") != 0) {
                System.out.println("Error: " + msg + ". //@version range invalid (not integer): '" + sub + "'");
                System.exit(99);
            }
            // Skip 'int: '
            sub = sub.substring(5);
            //System.out.println("Info: version, '" + sub + "', for field: " + msg);
            if (vec == null) { vec = new Vector<VersionEntry>(); }
            VersionEntry ent = new VersionEntry();
            int idx3 = sub.indexOf("..");
            if (idx3 != -1) {
                sub2 = sub.substring(idx3+2);
                sub = sub.substring(0, idx3);
                ent.stop = validateVersion(msg, sub2);
            }
            ent.start = validateVersion(msg, sub);
            if ((ent.stop != -1) && (ent.stop < ent.start)) {
                System.out.println("Error: " + msg + ". //@version range invalid : '" + ent.start + ".." + ent.stop + "'");
                System.exit(99);
            }
            vec.add(ent);
        }
        return vec;
    }

    public static Vector<VersionEntry> getReducedVersions(String msg, String directiveStr)
    {
        Vector<VersionEntry> vec = getVersions(msg, directiveStr);
        if (vec != null) {
            // Sort entries
            vec.sort(new VersionEntryComparator());
            // Remove not needed constraints
            // Find the lowest single value, we can remove all above since a single means 'this and up'
            int lastIdx = -1;
            for (int i = 0; i < vec.size(); i++) {
                if (vec.elementAt(i).stop == -1) {
                    lastIdx = i;
                    break;
                }
            }
            if (lastIdx >= 0) {
                while (vec.size() > (lastIdx+1)) { vec.remove(vec.size()-1); }
            }
            ///TODO Check for overlaping ranges, these can be combined

            //for (VersionEntry ent : vec) {
            //    System.out.println("Info: version, '" + ent.start + " .. " + ent.stop + "', for field: " + msg);
            //}
        }
        return vec;
    }

    public static String getVersionDescription(String directiveStr)
    {
        String res = "";
        Vector<VersionEntry> vec = getReducedVersions("", directiveStr);
        if (vec != null) {
            res += "Field valid for version(s): ";
            for (VersionEntry ent : vec) {
                int stop = ent.stop;
                if (stop < 0) stop = 255;
                res += "(" + ent.start + ".." + stop + "), ";
            }
        }
        return res;
    }

    public static int highestVersion(String msg, String directiveStr)
    {
        int version = -1;
        Vector<VersionEntry> vec = getVersions(msg, directiveStr);
        if (vec != null) {
            for (VersionEntry ent : vec) {
                if (version < ent.start) version = ent.start;
                if (version < ent.stop) version = ent.stop;
            }
        }
        return version;
    }

    public static void rangeDirective(String msg, IDLField field)
    {
        String directiveStr = field.getDirective();
        int idx = directiveStr.indexOf("range = ");
        if (idx == -1) return;
        int idx2 = directiveStr.indexOf(",", idx);
        idx += 8;  // Skip 'range = '
        String sub;
        if (idx2 == -1) {
            sub = directiveStr.substring(idx);
        } else {
            sub = directiveStr.substring(idx, idx2);
        }
        // Make sure value type is 'int: ' or 'real: '
        String sublo = "";
        if (sub.indexOf("int: ") == 0) {
            sublo = sub.substring(5);
        } else if (sub.indexOf("float: ") == 0) {
            sublo = sub.substring(7);
        } else {
            System.out.println("Error: " + msg + ". Field " + field.getName() + ". //@range specification invalid (not integer/floating point): '" + sub + "'");
            System.exit(99);
        }
        String subhi;
        int idx3 = sublo.indexOf("..");
        if (idx3 != -1) {
            subhi = sublo.substring(idx3+2);
            sublo = sublo.substring(0, idx3);
            field.setRange(sublo, subhi);
        } else {
            System.out.println("Error: " + msg + ". Field " + field.getName() + ". //@range specification invalid (missing ..): '" + sub + "'");
            System.exit(99);
        }
    }

}
