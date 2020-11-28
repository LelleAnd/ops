/*
 * NewJavaCompiler.java
 *
 * Created on den 12 november 2007, 15:39
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package opsc;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.lang.ClassLoader;
import java.util.List;
import java.util.Vector;
import java.util.Arrays;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.IDLEnumType;

/**
 *
 * @author angr
 */
public class JavaCompiler extends opsc.Compiler
{
    final static String CLASS_COMMENT_REGEX = "__classComment";
    final static String CONSTRUCTOR_BODY_REGEX = "__constructorBody";
    final static String DECLARATIONS_REGEX = "__declarations";
    final static String SERIALIZE_REGEX = "__serialize";
    final static String DESERIALIZE_REGEX = "__deserialize";
    final static String CLONE_BODY_REGEX = "__cloneBody";
    final static String SIZE_REGEX = "__size";
    final static String JAVA_DIR = "Java";
    //Vector<IDLClass> idlClasses;
    //private String projectDirectory;
    private static String BASE_CLASS_NAME_REGEX = "__baseClassName";
    private static String CREATE_BODY_REGEX = "__createBody";
    String createdFiles = "";
    private Vector<JarDependency> jarDependencies;
    //private String _projectName;

    public JavaCompiler(String projname) {
        super(projname);
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        createdFiles = "";
        _idlClasses = idlClasses;
        //this.projectDirectory = projectDirectory;
        try
        {
            if (!_onlyGenTypeSupport) {
              for (IDLClass iDLClass : _idlClasses)
              {
                if (iDLClass.getType() == IDLClass.ENUM_TYPE)
                {
                  //System.out.println("Compile enum");
                  compileEnum(iDLClass);
                }
                else
                {
                  compileDataClass(iDLClass);
                  if (!isOnlyDefinition(iDLClass)) {
                    if (!isTopLevel(iDLClass)) {
                      System.out.println("Info: Java, skipping generation of publisher/subscriber for " + iDLClass.getClassName());
                    } else {
                      compileSubscriber(iDLClass);
                      compilePublisher(iDLClass);
                    }
                  }
                }
              }
            }
            compileTypeSupport(idlClasses, _projectName);
        }
        catch (IOException iOException)
        {
            System.out.println("Generating Java failed with the following exception: " + iOException.getMessage());
        }

    }

    protected void compileEnum(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();

        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        //setOutputFileName(projectDirectory + JAVA_DIR + "/" + packageFilePart + "/" + className + ".java");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".java");

        java.io.InputStream stream = findTemplateFile("javaenumtemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(DECLARATIONS_REGEX, getEnumDeclarations(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    private String getClassComment(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getComment() != null) {
            if (!idlClass.getComment().equals("")) {
                String comment = idlClass.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                    ret += tab(0) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(0) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
            }
        }
        return ret;
    }

    public void compileDataClass(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();
        String baseClassName = "OPSObject";
        if (idlClass.getBaseClassName() != null)
        {
            baseClassName = idlClass.getBaseClassName();
        }
        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".java");

        java.io.InputStream stream;
        if (isOnlyDefinition(idlClass)) {
            stream = findTemplateFile("javatemplatebare.tpl");
        } else {
            stream = findTemplateFile("javatemplate.tpl");
        }
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(CLASS_COMMENT_REGEX, getClassComment(idlClass));
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, baseClassName);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(CONSTRUCTOR_BODY_REGEX, getConstructorBody(idlClass));
        templateText = templateText.replace(DECLARATIONS_REGEX, getDeclarations(idlClass));
        templateText = templateText.replace(SERIALIZE_REGEX, getSerialize(idlClass));
        templateText = templateText.replace(CLONE_BODY_REGEX, getCloneBody(idlClass));

        //Save the modified text to the output file.
        saveOutputText(templateText);
        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    public String getName()
    {
        return "JavaCompiler";
    }

    protected String getFieldName(IDLField field)
    {
        String name = field.getName();
        if (isReservedName(name)) return name + "_";
        return name;
    }

    protected void compilePublisher(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();
        String packageName = idlClass.getPackageName();

        String packageFilePart = packageName.replace(".", "/");

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + "Publisher.java");

        java.io.InputStream stream = findTemplateFile("javapublishertemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);

        //Save the modified text to the output file.
        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileSubscriber(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();
        String packageName = idlClass.getPackageName();

        String packageFilePart = packageName.replace(".", "/");

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + "Subscriber.java");

        java.io.InputStream stream = findTemplateFile("javasubscribertemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);

        //Save the modified text to the output file.
        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
    }

    protected void compileTypeSupport(Vector<IDLClass> idlClasses, String projectName)
    {
      try {
        String className = projectName + "TypeFactory";
        String packageName = projectName;

        String packageFilePart = packageName.replace(".", "/");

        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".java");

        java.io.InputStream stream = findTemplateFile("javatypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);

        //Save the modified text to the output file.
        String createBodyText = "";

        for (IDLClass iDLClass : idlClasses)
        {
            if (isOnlyDefinition(iDLClass) || isNoFactory(iDLClass)) continue;

            createBodyText += tab(2) + "if(type.equals(\"" + iDLClass.getPackageName() + "." + iDLClass.getClassName() + "\"))" + endl();
            createBodyText += tab(2) + "{" + endl();
            createBodyText += tab(3) + "return new " + iDLClass.getPackageName() + "." + iDLClass.getClassName() + "();" + endl();
            createBodyText += tab(2) + "}" + endl();
        }
        createBodyText += tab(2) + "return null;" + endl();

        templateText = templateText.replace(CREATE_BODY_REGEX, createBodyText);

        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
      }
      catch (IOException iOException)
      {
          System.out.println("Generating Java Factory failed with the following exception: " + iOException.getMessage());
      }
    }

    protected String getConstructorBody(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getVersion() > 0) {
            ret += tab(2) + "idlVersionMask |= 1;" + endl();
        }
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String iVal = "";
            String eType = elementType(field.getType());
            if (field.isEnumType()) {
                //Set first enum value as init value
                if (field.getValue().length() > 0) {
                    iVal = eType + "." + field.getValue();
                }
            }
            String fieldName = getFieldName(field);
            if (field.isArray() && (field.getArraySize() > 0)) {
                // for (int i = 0; i < 5; i++) _stringFixArr.Add("");
                if (!field.isEnumType()) iVal = initValue(eType);
                ret += tab(2) + "for (int i = 0; i < " + field.getArraySize() + "; i++) " + fieldName + ".add(" + iVal + ");" + endl();
            } else if (field.isEnumType() && !field.isArray()) {
                ret += tab(2) + fieldName + " = " + iVal + ";" + endl();
            }
        }
        return ret;
    }

    protected String initValue(String s)
    {
        s = elementType(s);
        if (s.equals("string")) return "\"\"";
        if (s.equals("boolean")) return "false";
        if (s.equals("short")) return "(short)0";
        if (s.equals("int")) return "0";
        if (s.equals("long")) return "(long)0";
        if (s.equals("double")) return "0.0";
        if (s.equals("float")) return "0.0f";
        if (s.equals("byte")) return "(byte)0";
        return "new " + s + "()";
    }

    protected String getCloneBody(IDLClass idlClass)
    {
        String ret = "";
        ret += tab(2) + "cloneResult." + idlClass.getClassName() + "_version = this." + idlClass.getClassName() + "_version;" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (field.isIdlType()) {
                if (!field.isArray()) {
                    ret += tab(2) + "cloneResult." + fieldName + " = (" + field.getType() + ")this." + fieldName + ".clone();" + endl();
                } else {
                    //data.forEach((n) -> System.out.println(n));
                    ret += tab(2) + "cloneResult." + fieldName + " = new java.util.Vector<" + elementType(field.getType()) + ">();" + endl();
                    ret += tab(2) + "this." + fieldName + ".forEach((item) -> cloneResult." + fieldName + ".add((" +
                      elementType(field.getType()) + ")item.clone()));" + endl();
                }
            }
            //"Arrays.copyOf(original, newLength)";
            else if (field.isArray()) {
                ret += tab(2) + "cloneResult." + fieldName + " = (java.util.Vector)this." + fieldName + ".clone();" + endl();
            } else {
                ret += tab(2) + "cloneResult." + fieldName + " = this." + fieldName + ";" + endl();
            }
        }
        return ret;
    }

    private String getEnumDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (int i = 0; i < idlClass.getEnumNames().size(); i++) {
            ret += idlClass.getEnumNames().get(i);
            ret += ",";
        }
        return ret;
    }

    protected String getEnumTypeDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (IDLEnumType et : idlClass.getEnumTypes()) {
            if (!et.getComment().equals("")) {
                String comment = et.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                    ret += tab(1) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(1) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
            }
            ret += tab(1) + "public enum " + et.getName() + " {" + endl();
            String values = "";
            for (String eName : et.getEnumNames()) {
                if (values != "") values += ", ";
                values += eName;
            }
            ret += tab(2) + values + endl();
            ret += tab(1) + "};" + endl();
        }
        if (ret != "") ret += endl();
        return ret;
    }

    protected String getDeclarations(IDLClass idlClass)
    {
        String ret = getEnumTypeDeclarations(idlClass);

        if (!isOnlyDefinition(idlClass)) {
            int version = idlClass.getVersion();
            if (version < 0) { version = 0; }
            ret += tab(1) + "public static final byte " + idlClass.getClassName() + "_idlVersion = " + version + ";" + endl();
        }

        for (IDLField field : idlClass.getFields()) {
            String fieldName = getFieldName(field);
            if (!field.getComment().equals("")) {
                String comment = field.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                  ret += tab(1) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                  comment = comment.substring(idx+1);
                }
                ret += tab(1) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
            }
            if (field.isArray()) {
                ret += tab(1) + "public " + getDeclareVector(field);
            } else if (field.getType().equals("string")) {
                if (field.isStatic()) {
                    ret += tab(1) + "public static final " + languageType(field.getType()) + " " + fieldName + " = " + field.getValue() + ";" + endl() + endl();
                } else {
                    ret += tab(1) + "public " + languageType(field.getType()) + " " + fieldName + " = \"\";" + endl();
                }
            } else if (field.isIdlType()) {
                ret += tab(1) + "public " + languageType(field.getType()) + " " + fieldName + " = new " + languageType(field.getType()) + "();" + endl();
            } else if (field.isEnumType()) {
                ret += tab(1) + "public " + field.getType() + " " + fieldName + ";" + endl();
            } else //Simple primitive type
            {
                if (field.isStatic()) {
                    String suffix = "";
                    if (languageType(field.getType()).equals("float")) suffix = "f";
                    ret += tab(1) + "public static final " + languageType(field.getType()) + " " + fieldName + " = " + field.getValue() + suffix + ";" + endl() + endl();
                } else {
                    ret += tab(1) + "public " + languageType(field.getType()) + " " + fieldName + ";" + endl();
                }
            }
        }
        return ret;
    }

    protected String getDeclareVector(IDLField field)
    {
        return languageType(field.getType()) + " " + getFieldName(field) + " = new " + languageType(field.getType()) + "();" + endl();
    }

    protected String languageType(String s)
    {
        if (s.equals("string")) return "String";
        if (s.equals("boolean")) return "boolean";
        if (s.equals("short")) return "short";
        if (s.equals("int")) return "int";
        if (s.equals("long")) return "long";
        if (s.equals("double")) return "double";
        if (s.equals("float")) return "float";
        if (s.equals("byte")) return "byte";
        if (s.equals("string[]")) return "java.util.Vector<String>";
        if (s.equals("short[]")) return "java.util.Vector<Short>";
        if (s.equals("int[]")) return "java.util.Vector<Integer>";
        if (s.equals("long[]")) return "java.util.Vector<Long>";
        if (s.equals("double[]")) return "java.util.Vector<Double>";
        if (s.equals("float[]")) return "java.util.Vector<Float>";
        if (s.equals("byte[]")) return "java.util.Vector<Byte>";
        if (s.equals("boolean[]")) return "java.util.Vector<Boolean>";
        if (s.endsWith("[]")) return "java.util.Vector<" + elementType(s) + ">";
        if (s.equals("static string")) return "final static String";
        return s;
    }

    protected String elementType(String s)
    {
        if (s.endsWith("[]")) return s.substring(0, s.indexOf('['));
        return s;
    }

    private String getFieldGuard(String versionName, IDLField field)
    {
        String ret = "";
        Vector<VersionEntry> vec = getReducedVersions(field.getName(), field.getDirective());
        if (vec != null) {
            for (VersionEntry ent : vec) {
                String cond = "(" + versionName + " >= " + ent.start + ")";
                if (ent.stop != -1) {
                    cond = "(" + cond + " && (" + versionName + " <= " + ent.stop + "))";
                }
                if (ret.length() > 0) {
                    ret += " || ";
                }
                ret += cond;
            }
        }
        return ret;
    }

    protected String getSerialize(IDLClass idlClass)
    {
        String ret = "";
        String versionName = idlClass.getClassName() + "_version";
        String versionIdlName = idlClass.getClassName() + "_idlVersion";
        // Need an implicit version field that may be [de]serialized
        ret += tab(2) + "if (idlVersionMask != 0) {" + endl();
        ret += tab(3) + "byte tmp = archive.inout(\"" + versionName + "\", " + versionName + ");" + endl();
        ret += tab(3) + "if (tmp > " + versionIdlName + ") {" + endl();
        ret += tab(4) + "throw new IOException(\"" + idlClass.getClassName() + ": received version '\" + tmp + \"' > known version '\" + " + versionIdlName + " + \"'\");" + endl();
        ret += tab(3) + "}" + endl();
        ret += tab(3) + versionName + " = tmp;" + endl();
        ret += tab(2) + "} else {" + endl();
        ret += tab(3) + versionName + " = 0;" + endl();
        ret += tab(2) + "}" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            String fieldGuard = getFieldGuard(versionName, field);
            int tabCnt = 2;
            if (fieldGuard.length() > 0) {
                ret += tab(tabCnt) + "if (" + fieldGuard + ") {" + endl();
                tabCnt += 1;
            }
            if (field.isIdlType()) {
                if (!field.isArray()) {
                  if (field.isAbstract()) {
                    ret += tab(tabCnt) + fieldName + " = (" + field.getType() + ") archive.inout(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                  } else {
                    ret += tab(tabCnt) + fieldName + " = (" + field.getType() + ") archive.inout(\"" + field.getName() + "\", " + fieldName + ", " + field.getType() + ".class);" + endl();
                  }
                } else {
                  if (field.isAbstract()) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(field.getType()) + ") archive.inoutSerializableList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                  } else {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(field.getType()) + ") archive.inoutSerializableList(\"" + field.getName() + "\", " + fieldName + ", " +
                            elementType(field.getType()) + ".class);" + endl();
                  }
                }
            } else if (field.isEnumType()) {
                if (!field.isArray()) {
                    ret += tab(tabCnt) + fieldName + " = archive.inoutEnum(\"" + field.getName() + "\", " + fieldName + ", " + field.getType() + ".values());" + endl();
                } else {
                  ret += tab(tabCnt) + fieldName + " = (" + languageType(field.getType()) + ") archive.inoutEnumList(\"" +
                      field.getName() + "\", " + fieldName + ", " + elementType(field.getType()) + ".values());" + endl();
                }
            } else if (field.isArray()) {
                String fieldType = field.getType();
                if (fieldType.equals("int[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutIntegerList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("byte[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutByteList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("short[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutShortList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("long[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutLongList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("boolean[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutBooleanList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("float[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutFloatList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("double[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutDoubleList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else if (fieldType.equals("string[]")) {
                    ret += tab(tabCnt) + fieldName + " = (" + languageType(fieldType) + ") archive.inoutStringList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                }
            } else {
                ret += tab(tabCnt) + fieldName + " = archive.inout(\"" + field.getName() + "\", " + fieldName + ");" + endl();
            }
            if (fieldGuard.length() > 0) {
                tabCnt -= 1;
                ret += tab(tabCnt) + "}" + endl();
            }
        }
        return ret;
    }

    public void setJarDependencies(Vector<JarDependency> jarDeps)
    {
        this.jarDependencies = jarDeps;
    }

    public void buildAndJar(String projectDir) throws IOException, InterruptedException
    {
        // don't want to build bat scripts on linux...
        final boolean isLinux = System.getProperty("os.name").equals("Linux");

        String jarDepString = "";
        String manifestJarDepString = "Class-Path: ";
        Vector<String> jarCopyList = new Vector<String>();

        //System.out.println(">>>>: projectDir = " +  projectDir);
        //System.out.println(">>>>: _outputDir = " +  _outputDir);

        if (jarDependencies != null) {
            for (JarDependency jarDep : jarDependencies) {
                File jarToBeCopied = new File(jarDep.path + "");
                if (!jarToBeCopied.isAbsolute()) {
                    jarToBeCopied = new File(projectDir + File.separator + jarDep.path + "");
                }
                jarDepString += "\"" + jarToBeCopied.getName() + "\"" + File.pathSeparator;

                // Backward compatibility with HMI-based IDL compiler.
                // We now have moved the compiled jars to the Java subdirectory
                // So if files don't exist try in the 'Java' directory
                if (!jarToBeCopied.exists()) {
                  jarToBeCopied = new File(jarToBeCopied.getParent() + File.separator + JAVA_DIR + File.separator + jarToBeCopied.getName());
                }

                jarCopyList.add(jarToBeCopied.getAbsolutePath());

                manifestJarDepString += jarToBeCopied.getName() + " ";
            }
            manifestJarDepString += endl();
        } else {
            System.out.println("WARN: no jar dependencies");
        }

        String dinfoFile = "debugger_buildinfo.ops_tmp";
        String dinfoPath = _outputDir + File.separator + dinfoFile;
        // Remove the '_outputDir + File.separator' part from the filelist since we compile from the _outputDir
        String tmp = createdFiles.replace('\\', '/');
        tmp = tmp.replaceAll("\"" + _outputDir.replace('\\', '/') + "/", "\"");
        createAndWriteFile(dinfoPath, tmp);

        String manFile = "manifest_adds.ops_tmp";
        String manFilePath = _outputDir + File.separator + manFile;
        createAndWriteFile(manFilePath, manifestJarDepString);

        /// Try to find out the full path for the included OPS Jar files
        String ExePath = "";
        String SubPath = "";
        try {
          ExePath = JavaCompiler.class.getProtectionDomain().getCodeSource().getLocation().getPath();
          //System.out.println("Debug: ExePath: " + ExePath);
          ExePath = java.net.URLDecoder.decode(ExePath, "UTF-8");
          ExePath = ExePath.replaceFirst("file:/", "");
          // On windows the ExePath can be like "/D:/OPS/OPS4/Tools/opsc/dist/opsc.jar"
          // So we need to remove the first '/' in this case
          int idx = ExePath.indexOf(":");
          if (idx > 0) {
            ExePath = ExePath.substring(1);
          }
        }
        catch (Exception e) {
            SubPath = "";
            ExePath = "";
        }
        if (_verbose > 0) System.out.println("Debug: ExePath: " + ExePath);

        // resolve path to OPSJLib. Not required for this app but for generated java code
        String OPSJLibpath = ops.OPSObject.class.getProtectionDomain().getCodeSource().getLocation().getPath();
        try {
            String decodedPath = java.net.URLDecoder.decode(OPSJLibpath, "UTF-8");
            // On windows the ExePath can be like "/D:/OPS/OPS4/Tools/opsc/dist/OPSJLib.jar"
            // So we need to remove the first '/' in this case
            int idx = decodedPath.indexOf(":");
            if (idx > 0) {
              decodedPath = decodedPath.substring(1);
            }
            OPSJLibpath = decodedPath;
        } catch(java.io.UnsupportedEncodingException uee) {
            System.out.println("Error: Failed to decode path to " + OPSJLibpath);
        }
        if (_verbose > 0) System.out.println("Debug: Path to OPSJLib: " + OPSJLibpath);

        // resolve path to ConfigurationLib
        String ConfigurationLibpath = configlib.Serializable.class.getProtectionDomain().getCodeSource().getLocation().getPath();
        try {
            String decodedPath = java.net.URLDecoder.decode(ConfigurationLibpath, "UTF-8");
            // On windows the ExePath can be like "/D:/OPS/OPS4/Tools/opsc/dist/ConfigurationLib.jar"
            // So we need to remove the first '/' in this case
            int idx = decodedPath.indexOf(":");
            if (idx > 0) {
              decodedPath = decodedPath.substring(1);
            }
            ConfigurationLibpath = decodedPath;
        } catch(java.io.UnsupportedEncodingException uee) {
            System.out.println("Error: Failed to decode path to " + ConfigurationLibpath);
        }
        if (_verbose > 0) System.out.println("Debug: Path to ConfigurationLib: " + ConfigurationLibpath);

        // Add OPS libs to jarDepString
        jarDepString += "\"" + OPSJLibpath + "\"" + File.pathSeparator;
        jarDepString += "\"" + ConfigurationLibpath + "\"" + File.pathSeparator;

        // -----------------------------------------
        // Create batch / bash file for compiling of the generated java files
        // The batch / bash file will be running  in the _output dir
        String classOutputDir = ".obj";

        String silent = "";
        if (!isLinux) {
          silent = "@";
        }

        String execString =
                "javac -cp " + jarDepString +
                " -d " + classOutputDir +
                " @" + "\"" + dinfoFile + "\"";

        String batFileText = "";
        if (isLinux) {
            batFileText += "#!/bin/sh" + endl();
            batFileText += "set -e # exit on err" + endl() + endl();

            batFileText += "# find out where this script is" + endl();
            batFileText += "SCRIPT_PATH=`readlink -f \"$0\"`; SCRIPT_PATH=`dirname \"$SCRIPT_PATH\"`; SCRIPT_PATH=`eval \"cd \\\"$SCRIPT_PATH\\\" && pwd\"`" + endl();
            batFileText += "echo \"invoking script at: $SCRIPT_PATH\"" + endl();
            batFileText += "cd $SCRIPT_PATH" + endl();

        } else {
            //batFileText += "@echo off" + endl();
            batFileText += silent + "pushd %~dp0" + endl();   // cd to bat-file directory
        }
        batFileText += silent + "echo Compiling Java..." + endl();
        batFileText += silent + "javac -version" + endl();
        if (isLinux) {
            batFileText += "mkdir -p " + classOutputDir + endl();
        } else {
            // on windows, mkdir works like posix mkdir -p IF commandline extensions are enabled
            batFileText += silent + "if not exist \"" + classOutputDir + "\" (" + endl();
            batFileText += silent + "mkdir " + classOutputDir + endl();
            batFileText += ")" + endl();
        }

        // Add commands for copying all dependency jars
        String copyCmd = "copy";
        if (isLinux) {
          copyCmd = "cp";
        }
        for (String jarSrc : jarCopyList) {
          batFileText += copyCmd + " \"" + jarSrc + "\" ." + endl();
        }

        batFileText += execString + endl();

        String jarPackString = "jar cfm \"" + _projectName + ".jar\" \"" +
            manFile + "\" -C " + classOutputDir + " . ";
        batFileText += jarPackString + endl();
        batFileText += silent + "echo Compiling Java done." + endl();
        if (!isLinux) {
            batFileText += silent + "popd" + endl();
        }

        String script = "";

        if (isLinux) {
            script = _outputDir + File.separator + "java_build_script.sh";
            if (_verbose > 0) System.out.println("Debug: Path to script: " + script);
            java.io.File file = new java.io.File(script);
            java.io.Writer output = new java.io.BufferedWriter(new java.io.FileWriter(file));
//            java.io.Writer output = new java.io.BufferedWriter(new java.io.FileWriter(script));
            output.write(batFileText);
            output.close();

            //file.setReadable(true, false);
            //file.setWriteable(true, false);
            file.setExecutable(true, false);

//            Runtime rTime = Runtime.getRuntime();
//            // make script executable
//            Process process = rTime.exec("chmod u+x " + script);
//
//            process.waitFor();
        } else {
            script = _outputDir + File.separator + "java_build_script.bat";
            if (_verbose > 0) System.out.println("Debug: Path to script: " + script);
            createAndWriteFile(script, batFileText);
        }

        // --------------------------------------------------------------------
        // Run the batch / shell script and redirct output to standard out
        try {
          ProcessBuilder pb;
          System.out.println("Info: running \"" + script + "\"");
          if (isLinux) {
            pb = new ProcessBuilder("sh", script);
          } else {
            pb = new ProcessBuilder(script, "");
          }
          pb.redirectErrorStream(true);
          Process p = pb.start();
          InputStream inp = p.getInputStream();

          int c;
          while ((c = inp.read()) != -1) {
            System.out.write(c);
          }
        }
        catch (IOException e) {
          System.out.println("Error: " + e.getMessage());
        }
    }

    public void appendFileToBuild(List<String> file)
    {
        for (String string : file)
        {
          createdFiles += "\"" + string + "\"\n";
        }
    }

    public boolean isReservedName(String name)
    {
        return Arrays.binarySearch(reservedNames, name) >= 0;
    }

    // Array of all reserved keywords in ascending order (for binarySearch() to work)
    private static final String[] reservedNames = {
      "abstract", "assert",
      "boolean", "break", "byte",
      "case", "catch", "char", "class", "const", "continue",
      "default", "do", "double",
      "else", "enum", "extends",
      "final", "finally", "float", "for",
      "goto",
      "if", "implements", "import", "instanceof", "int", "interface",
      "long",
      "native", "new",
      "package", "private", "protected", "public",
      "return",
      "short", "static", "strictfp", "super", "switch", "synchronized",
      "this", "throw", "throws", "transient", "try",
      "void", "volatile",
      "while"
    };

}
