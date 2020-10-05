/*
 * CSharpCompiler.java
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
import java.util.List;
import java.util.Vector;
import java.util.Arrays;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.IDLEnumType;

/**
 *
 * @author Lelle
 */
public class CSharpCompiler extends opsc.Compiler
{
    final static String CONSTRUCTOR_BODY_REGEX = "__constructorBody";
    final static String DECLARATIONS_REGEX = "__declarations";
    final static String SERIALIZE_REGEX = "__serialize";
    final static String DESERIALIZE_REGEX = "__deserialize";
    final static String CLONE_BODY_REGEX = "__cloneBody";
    final static String CLASS_COMMENT_REGEX = "__classComment";
    final static String SIZE_REGEX = "__size";
    final static String CS_DIR = "CSharp";
    private String projectDirectory;
    private static String BASE_CLASS_NAME_REGEX = "__baseClassName";
    private static String CREATE_OBJECT_BODY_REGEX = "__createObjectBody";
    private static String CREATE_STRING_BODY_REGEX = "__createStringBody";

    String createdFiles = "";
    private Vector<JarDependency> dllDependencies;

    public CSharpCompiler(String projname) {
        super(projname);
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        createdFiles = "";
        this._idlClasses = idlClasses;
        this.projectDirectory = projectDirectory;
        try
        {
            if (!_onlyGenTypeSupport) {
              for (IDLClass iDLClass : idlClasses)
              {
                if (iDLClass.getType() == IDLClass.ENUM_TYPE)
                {
                    //System.out.println("Compile enum");
                    compileEnum(iDLClass);
                }
                else
                {
                    compileDataClass(iDLClass);
                    // We put these in the same file as the data class
                    //compileSubscriber(iDLClass);
                    //compilePublisher(iDLClass);
                }
              }
            }

            compileTypeSupport(idlClasses, _projectName);
        } catch (IOException iOException)  {
            //JOptionPane.showMessageDialog(null, "Generating C# failed with the following exception: " + iOException.getMessage());
            System.out.println( "Error: Generating C# failed with the following exception: " + iOException.getMessage());
        }
    }

    protected void compilePublisher(IDLClass cl) {}
    protected void compileSubscriber(IDLClass cl) {}

    protected void compileEnum(IDLClass idlClass) throws IOException
    {
        String className = idlClass.getClassName();

        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        //setOutputFileName(projectDirectory + CS_DIR + "/" + packageFilePart + "/" + className + ".cs");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".cs");

        java.io.InputStream stream = findTemplateFile("csenumtemplate.tpl");
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
                    ret += tab(1) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "").replace("\r", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(1) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
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
            // Fix for different casing in C# and Java/C++ on ops namespace
            int idx = baseClassName.indexOf("ops.");
            if (idx == 0) {
                baseClassName = "Ops." + baseClassName.substring(4);
            }
        }
        String packageName = idlClass.getPackageName();
        String packageFilePart = packageName.replace(".", "/");
        //setOutputFileName(projectDirectory + CS_DIR + "/" + packageFilePart + "/" + className + ".cs");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".cs");

        java.io.InputStream stream;
        if (isOnlyDefinition(idlClass)) {
            stream = findTemplateFile("cstemplatebare.tpl");
        } else {
            stream = findTemplateFile("cstemplate.tpl");
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
        return "CsFactoryIDLCompiler";
    }

    protected String getFieldName(IDLField field)
    {
        String name = field.getName();
        if (isReservedName(name)) return name + "_";
        return name;
    }

//    private void compilePublisher(IDLClass idlClass) throws IOException
//    {
//        String className = idlClass.getClassName();
//        String packageName = idlClass.getPackageName();
//
//        String packageFilePart = packageName.replace(".", "/");
//        setOutputFileName(projectDirectory + CS_DIR + "/" + packageFilePart + "/" + className + "Publisher.cs");
//        String resource = "/ops/netbeansmodules/idlsupport/templates/cspublishertemplate.tpl";
//        setTemplateTextFromResource(resource);
//        //setTemplateFileName("templates/javapublishertemplate.tpl");
//        setTabString("    ");//Default is "\t"
//        setEndlString("\n");//Default is "\r\n"
//
//        //Get the template file as a String
//        String templateText = getTemplateText();
//
//        //Replace regular expressions in the template file.
//        templateText = templateText.replace(CLASS_NAME_REGEX, className);
//        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
//
//        //Save the modified text to the output file.
//        saveOutputText(templateText);
//
//        createdFiles += "\"" + getOutputFileName() + "\"\n";
//    }

//    private void compileSubscriber(IDLClass idlClass) throws IOException
//    {
//        String className = idlClass.getClassName();
//        String packageName = idlClass.getPackageName();
//
//        String packageFilePart = packageName.replace(".", "/");
//        setOutputFileName(projectDirectory + CS_DIR + "/" + packageFilePart + "/" + className + "Subscriber.cs");
//        String resource = "/ops/netbeansmodules/idlsupport/templates/cssubscribertemplate.tpl";
//        setTemplateTextFromResource(resource);
//        //setTemplateFileName("templates/javasubscribertemplate.tpl");
//        setTabString("    ");//Default is "\t"
//        setEndlString("\n");//Default is "\r\n"
//
//        //Get the template file as a String
//        String templateText = getTemplateText();
//
//        //Replace regular expressions in the template file.
//        templateText = templateText.replace(CLASS_NAME_REGEX, className);
//        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
//
//        //Save the modified text to the output file.
//        saveOutputText(templateText);
//
//        createdFiles += "\"" + getOutputFileName() + "\"\n";
//    }

    protected void compileTypeSupport(Vector<IDLClass> idlClasses, String projectName)
    {
      try {
        String className = projectName + "TypeFactory";
        String packageName = projectName;

        String packageFilePart = packageName.replace(".", "/");
        //setOutputFileName(projectDirectory + CS_DIR + "/" + projectName + "/" + className + ".cs");
        setOutputFileName(_outputDir + File.separator + packageFilePart + File.separator + className + ".cs");

        java.io.InputStream stream = findTemplateFile("cstypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();

        //Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);

        //Save the modified text to the output file.
        String createObjectBodyText = "";
        String createStringBodyText = "";

        for (IDLClass iDLClass : idlClasses)
        {
            if (isOnlyDefinition(iDLClass) || isNoFactory(iDLClass)) continue;

            createObjectBodyText += tab(3) + "if (type.Equals(\"" + iDLClass.getPackageName() + "." + iDLClass.getClassName() + "\"))" + endl();
            createObjectBodyText += tab(3) + "{" + endl();
            createObjectBodyText += tab(4) +      "return new " + iDLClass.getPackageName() + "." + iDLClass.getClassName() + "();" + endl();
            createObjectBodyText += tab(3) + "}" + endl();

            createStringBodyText += tab(3) + "if (obj is " + iDLClass.getPackageName() + "." + iDLClass.getClassName() + ")" + endl();
            createStringBodyText += tab(3) + "{" + endl();
            createStringBodyText += tab(4) +    "return \"" + iDLClass.getPackageName() + "." + iDLClass.getClassName() + "\";" + endl();
            createStringBodyText += tab(3) + "}" + endl();
        }
        createObjectBodyText += tab(3) + "return null;" + endl();
        createStringBodyText += tab(3) + "return null;" + endl();

        templateText = templateText.replace(CREATE_OBJECT_BODY_REGEX, createObjectBodyText);
        templateText = templateText.replace(CREATE_STRING_BODY_REGEX, createStringBodyText);

        saveOutputText(templateText);

        createdFiles += "\"" + getOutputFileName() + "\"\n";
      } catch (IOException iOException)  {
          //JOptionPane.showMessageDialog(null, "Generating C# failed with the following exception: " + iOException.getMessage());
          System.out.println( "Error: Generating C# Factory failed with the following exception: " + iOException.getMessage());
      }
    }

    protected String getConstructorBody(IDLClass idlClass)
    {
        String ret = "";
        if (idlClass.getVersion() > 0) {
            ret += tab(3) + "IdlVersionMask = IdlVersionMask | 1;" + endl();
        }
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            if (field.isArray() && (field.getArraySize() > 0)) {
                String fieldName = getFieldName(field);
                String eType = elementType(field.getType());
                // for (int i = 0; i < 5; i++) _stringFixArr.Add("");
                ret += tab(3) + "for (int i = 0; i < " + field.getArraySize() + "; i++) _" + fieldName + ".Add(" + initValue(eType) + ");" + endl();

            }
        }
        return ret;
    }

    private String getCloneBody(IDLClass idlClass)
    {
        String ret = "";
        ret += tab(3) + "cloneResult." + idlClass.getClassName() + "_version = this." + idlClass.getClassName() + "_version;" + endl();
        for (IDLField field : idlClass.getFields())
        {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            if (field.isIdlType())
            {
                if (!field.isArray())
                {
                    ret += tab(3) + "cloneResult." +  fieldName + " = (" + field.getType() + ")this." + fieldName + ".Clone();" + endl();
                }
                else
                {
                    String s = field.getType();
                    s = s.substring(0, s.indexOf('['));
                    ret += tab(3) + "cloneResult." +  fieldName + " = new " + languageType(field.getType()) + "(this." + fieldName + ".Count);" + endl();
                    ret += tab(3) + "this." +  fieldName + ".ForEach((item) => { cloneResult." + fieldName + ".Add((" + s + ")item.Clone()); });" + endl();
                }
            }

            //"Arrays.copyOf(original, newLength)";
            else if (field.isArray())
            {

                ret += tab(3) + "cloneResult." +  fieldName + " = new " + languageType(field.getType()) + "(this." + fieldName + ");" + endl();

            }
            else
            {
                ret += tab(3) + "cloneResult." +  fieldName + " = this." + fieldName + ";" + endl();
            }

        }
        return ret;
    }



    private String getEnumDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (int i = 0; i < idlClass.getEnumNames().size(); i++)
        {
            ret += idlClass.getEnumNames().get(i);
            ret += ",";
        }
        return ret;
    }

    protected String getDeclareVector(IDLField field) {
        return "";
    }

    protected String getEnumTypeDeclarations(IDLClass idlClass)
    {
        String ret = "";
        for (IDLEnumType et : idlClass.getEnumTypes()) {
            if (!et.getComment().equals("")) {
                String comment = et.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                    ret += tab(2) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(2) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
            }
            ret += tab(2) + "public enum " + et.getName() + " : short {" + endl();
            String values = "";
            for (String eName : et.getEnumNames()) {
                if (values != "") values += ", ";
                values += eName;
            }
            ret += tab(3) + values + endl();
            ret += tab(2) + "};" + endl();
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
            // Need an implicit version field that should be [de]serialized
            ret += tab(2) + "public const byte " + idlClass.getClassName() + "_idlVersion = " + version + ";" + endl();
        }

        for (IDLField field : idlClass.getFields())
        {
            String fieldName = getFieldName(field);
            if(!field.getComment().equals(""))
            {
                String comment = field.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0) {
                  ret += tab(2) + "///" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                  comment = comment.substring(idx+1);
                }
                ret += tab(2) + "///" + comment.replace("/*", "").replace("*/", "") + endl();
///                ret += tab(2) + "///" + field.getComment().replace("/*", "").replace("*/", "") + endl();
///                ret += tab(2) + "///" + field.getComment() + endl();
            }
            if (field.isArray())
            {
                ret += tab(2) + "private " + languageType(field.getType()) + " _" + fieldName +
                        " = new " + languageType(field.getType()) + "();" + endl();
                if(field.getType().equals("string[]")) {
                    ret += tab(2) + "[Editor(@\"System.Windows.Forms.Design.StringCollectionEditor,\" +" + endl() +
                           tab(3) + "\"System.Design, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a\"," + endl() +
                           tab(3) + "typeof(System.Drawing.Design.UITypeEditor))]" + endl();
                }
                ret += tab(2) + "public " + languageType(field.getType()) + " " + fieldName +
                        " { get { return " + " _" + fieldName + "; } set { _" + fieldName + " = value; } } " + endl() + endl();
            }
            else if(field.getType().equals("string"))
            {
                if (field.isStatic()) {
                    ret += tab(2) + "public const " + languageType(field.getType()) + " " + fieldName + " = " + field.getValue() + ";" + endl() + endl();
                } else {
                    ///TEST gives a description and category in a propertygrid
                    /// ret += tab(2) + "[Description(\"TBD\"), Category(\"" + idlClass.getClassName() + "\")]" + endl();
                    ///TEST
                    ret += tab(2) + "public " + languageType(field.getType()) + " " + fieldName + " { get; set; }" + endl() + endl();
                }
            }
            else if(field.isIdlType())
            {
                ret += tab(2) + "private " + languageType(field.getType()) + " _" + fieldName +
                        " = new " + languageType(field.getType()) + "();" + endl();
                ret += tab(2) + "[System.ComponentModel.TypeConverter(typeof(System.ComponentModel.ExpandableObjectConverter))]" + endl();
                ret += tab(2) + "public " + languageType(field.getType()) + " " + fieldName +
                        " { get { return " + " _" + fieldName + "; } set { _" + fieldName + " = value; } } " + endl() + endl();
            }
            else //Simple primitive type
            {
                if (field.isStatic()) {
                    String suffix = "";
                    if (languageType(field.getType()).equals("float")) suffix = "F";
                    ret += tab(2) + "public const " + languageType(field.getType()) + " " + fieldName + " = " + field.getValue() + suffix + ";" + endl() + endl();
                } else {
                    ret += tab(2) + "public " + languageType(field.getType()) + " " + fieldName + " { get; set; }" + endl() + endl();
                }
            }

        }
        return ret;
    }

    protected String languageType(String s)
    {
        if (s.equals("string")) return "string";
        if (s.equals("boolean")) return "bool";
        if (s.equals("short")) return "short";
        if (s.equals("int")) return "int";
        if (s.equals("long")) return "long";
        if (s.equals("double")) return "double";
        if (s.equals("float")) return "float";
        if (s.equals("byte")) return "byte";
        if (s.equals("string[]")) return "List<string>";
        if (s.equals("short[]")) return "List<short>";
        if (s.equals("int[]")) return "List<int>";
        if (s.equals("long[]")) return "List<long>";
        if (s.equals("double[]")) return "List<double>";
        if (s.equals("float[]")) return "List<float>";
        if (s.equals("byte[]")) return "List<byte>";
        if (s.equals("boolean[]")) return "List<bool>";
        if (s.endsWith("[]")) return "List<" + s.substring(0, s.indexOf('[')) + ">";
        if (s.equals("static string")) return "static const string";
        return s;
    }

    protected String elementType(String s)
    {
        if (s.endsWith("[]")) return s.substring(0, s.indexOf('['));
        return s;
    }

    protected String initValue(String s)
    {
        s = elementType(s);
        if (s.equals("string")) return "\"\"";
        if (s.equals("boolean")) return "false";
        if (s.equals("short")) return "0";
        if (s.equals("int")) return "0";
        if (s.equals("long")) return "0";
        if (s.equals("double")) return "0.0";
        if (s.equals("float")) return "0.0f";
        if (s.equals("byte")) return "0";
        return "new " + s + "()";
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
        // Need an implicit version field that may be [de]serialized
        ret += tab(3) + "if (IdlVersionMask != 0) {" + endl();
        ret += tab(4) + versionName + " = archive.Inout(\"" + versionName + "\", " + versionName + ");" + endl();
        ret += tab(3) + "} else {" + endl();
        ret += tab(4) + versionName + " = 0;" + endl();
        ret += tab(3) + "}" + endl();
        for (IDLField field : idlClass.getFields()) {
            if (field.isStatic()) continue;
            String fieldName = getFieldName(field);
            String fieldGuard = getFieldGuard(versionName, field);
            int tabCnt = 3;
            if (fieldGuard.length() > 0) {
                ret += tab(tabCnt) + "if (" + fieldGuard + ") {" + endl();
                tabCnt += 1;
            }
            if (field.isIdlType()) {
                if (!field.isArray()) {
                  if (field.isAbstract()) {
                    ret += tab(tabCnt) + "_" + fieldName + " = (" + field.getType() + ") archive.Inout(\"" + field.getName() + "\", _" + fieldName + ");" + endl();
                  } else {
                    ret += tab(tabCnt) + "_" + fieldName + " = (" + field.getType() + ") archive.Inout<" + field.getType() + ">(\"" + field.getName() + "\", _" + fieldName + ");" + endl();
                  }
                } else {
                  if (field.isAbstract()) {
                    ret += tab(tabCnt) + "_" + fieldName + " = (" + languageType(field.getType()) + ") archive.InoutSerializableList(\"" + field.getName() + "\", _" + fieldName + ");" + endl();
                  } else {
                    ret += tab(tabCnt) + "_" + fieldName + " = (" + languageType(field.getType()) + ") archive.InoutSerializableList<" + languageType(elementType(field.getType())) + ">(\"" + field.getName() + "\", _" + fieldName + ");" + endl();
                  }
                }
            } else if (field.isEnumType()) {
                if (!field.isArray()) {
                    ret += tab(tabCnt) + fieldName + " = archive.InoutEnum(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                } else {
                    ret += tab(tabCnt) + fieldName + " = archive.InoutEnumList(\"" + field.getName() + "\", " + fieldName + ");" + endl();
                }
            } else if (field.isArray()) {
                ret += tab(tabCnt) + "_" + fieldName + " = (" + languageType(field.getType()) + ") archive.Inout";
                if (field.getType().equals("int[]")) {
                    ret += "Integer";
                } else if(field.getType().equals("short[]")) {
                    ret += "Short";
                } else if(field.getType().equals("byte[]"))  {
                    ret += "Byte";
                } else if(field.getType().equals("long[]"))  {
                    ret += "Long";
                } else if(field.getType().equals("boolean[]")) {
                    ret += "Boolean";
                } else if(field.getType().equals("float[]")) {
                    ret += "Float";
                } else if(field.getType().equals("double[]")) {
                    ret += "Double";
                } else if(field.getType().equals("string[]")) {
                    ret += "String";
                }
                ret += "List(\"" + field.getName() + "\", _" + fieldName + ");" + endl();
            } else {
                ret += tab(tabCnt) + fieldName + " = archive.Inout(\"" + field.getName() + "\", " + fieldName + ");" + endl();
            }
            if (fieldGuard.length() > 0) {
                tabCnt -= 1;
                ret += tab(tabCnt) + "}" + endl();
            }
        }
        return ret;
    }

    public void setDllDependencies(Vector<JarDependency> dllDeps)
    {
        this.dllDependencies = dllDeps;
    }

    public void buildDll(String projectDir) throws IOException, InterruptedException
    {
      // don't want to build bat scripts on linux...
      final boolean isLinux = System.getProperty("os.name").equals("Linux");

      if (isLinux) {
        System.out.println("Info: Building C# on Linux is not yet supported");
        return;
      }

      //System.out.println(">>>>: projectDir = " +  projectDir);
      //System.out.println(">>>>: _outputDir = " +  _outputDir);

//        String projDirUp = projectDirectory.substring(0, projectDirectory.lastIndexOf("/Generated"));
//        String projectName = projectDirectory.substring(projDirUp.lastIndexOf("/")+1, projDirUp.length());

      String dllDepString = "";

      if (dllDependencies != null) {
        for (JarDependency dllDep : dllDependencies) {
          File dllToBeCopied = new File(dllDep.path + "");
          if (!dllToBeCopied.isAbsolute()) {
            dllToBeCopied = new File(projectDir + File.separator + dllDep.path + "");
          }
          // Backward compatibility with HMI-based IDL compiler (i.e. old project files).
          if (!dllToBeCopied.exists()) {
            // try changing C# (in old project files) to CSharp
            dllToBeCopied = new File(dllToBeCopied.getAbsolutePath().replaceAll("C#","CSharp"));
          }

          dllDepString += "/r:\"" + dllToBeCopied.getAbsolutePath() + "\" ";
        }
      }

      String cscPath = System.getenv("OPS_CSC_PATH");
      if (cscPath == null) {
        System.out.println("Info: Path to C# compiler \"csc.exe\" can be set using env. symbol OPS_CSC_PATH");
        cscPath = "csc.exe";
      } else {
        cscPath += File.separator + "csc.exe";
        System.out.println("Info: C# compiler \"" + cscPath + "\" used (from env. symbol OPS_CSC_PATH)");
      }

      String execString = "\"" + cscPath + "\" /target:library " +
                "/out:\"" + _outputDir + File.separator + _projectName + ".dll\" " +
                dllDepString + " /recurse:\"" + _outputDir + File.separator + "*.cs\"";
      String  batFileText = "";
      batFileText += "@pushd %~dp0" + endl();   // cd to bat-file directory
      batFileText += "@echo Compiling C#..."  + endl();
      batFileText += execString + endl();
      batFileText += "@popd" + endl();
      batFileText += "@echo Compiling C# done."  + endl();

      String script = _outputDir + File.separator + "cs_build_script.bat";
      createAndWriteFile(script, batFileText);

      // --------------------------------------------------------------------
      // Run the batch / shell script and redirct output to standard out
      try {
        System.out.println("Info: running \"" + script + "\"");
        ProcessBuilder pb = new ProcessBuilder(script, "");
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

    public boolean isReservedName(String name)
    {
        return Arrays.binarySearch(reservedNames, name.toLowerCase()) >= 0;
    }

    // Array of all reserved keywords in ascending order (for binarySearch() to work)
    private static final String[] reservedNames = {
      "abstract", "as",
      "base", "bool", "break", "byte",
      "case", "catch", "char", "checked", "class", "const", "continue",
      "decimal", "default", "delegate", "do", "double",
      "else", "enum", "event", "explicit", "extern",
      "false", "finally", "fixed", "float", "for", "foreach",
      "goto",
      "if", "implicit", "in", "int", "interface", "internal", "is",
      "lock", "long",
      "namespace", "new", "null",
      "object", "operator", "out", "override",
      "params", "private", "protected", "public",
      "readonly", "ref", "return",
      "sbyte", "sealed", "short", "sizeof", "stackalloc", "static", "string",
      "struct", "switch",
      "this", "throw", "true", "try", "typeof",
      "uint", "ulong", "unchecked", "unsafe", "ushort", "using",
      "virtual", "void", "volatile",
      "while"
    };

}
