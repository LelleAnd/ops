/** -*-java-*- Copyright (C) 2014 Saab Dynamics AB
***************************************************************************
*  @file   opsc/PythonCompiler.java
*  @author Jakob Lindgren <jakob.lindgren@saabgroup.com>
*
* This file is based on:
*   Tools/NBOPSIDLSupport/src/ops/netbeansmodules/idlsupport/PythonCompiler.java
* It is a complete copy of the said file but with dependencies to NB
* completely removed. I have also added support for runtime configuration
* of where output should ge and where templates can be found
*
***************************************************************************
*/

package opsc;

import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Vector;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.TopicInfo;




public class PythonCompiler extends opsc.CompilerSupport
{
//class template
    private final static String BASE_CLASS_NAME_REGEX = "__baseClassName";
    private final static String DECLARATIONS_REGEX = "__declarations";
    private final static String SERIALIZE_REGEX = "__serialize";
    private final static String CREATE_BODY_REGEX = "__createBody";
    private final static String CLASS_COMMENT_REGEX = "__classComment";
    private final static String VALIDATION_REGEX = "__validation";
//module template
    private final static String IMPORTS_REGEX = "__imports";
    private final static String CLASSES_REGEX = "__classes";

/*

            for (IDLField field : idlClass.getFields())
            {
                if (field.isIdlType())
                {
                    String type = field.getType();
                    if (field.isArray())
                    {
                        type = type.substring(0, type.length() - 2);
                    }

                    int splitIndex = type.lastIndexOf(".");
                    String packageStr = type.substring(0,splitIndex);
                    String classStr   = type.substring(splitIndex+1);

                    importString.add("*from " + packageStr.replace(".","_") + " import " + classStr + endl());
                    System.out.println("Adding import");
                    System.out.println(" for: " + idlClass.getClassName());
                    System.out.println(" packageStr: " + packageStr);
                    System.out.println(" classStr: " + classStr);


                }
            }
*/

    private static String StringJoin(String a,ArrayList<String> list)
    {
        String str = list.get(0);

        for (int i=1;i<list.size();i++)
        {
            str += a + list.get(i);
        }
        return str;
    }

    protected class PythonHelper
    {

        private String packageName;
        private String className;
        //private String baseClassName = "opsTypes.OPS_Object";
        //private PythonHelper baseClass = null;
        private ArrayList<String>       dependencyName;
        private ArrayList<PythonHelper> dependencyHelper;

        private String classDeclaration;
        private HashSet<String> imports;

        private boolean inherited = false;
        private boolean saved = false;

        /*
        public String toString()
        {
            String str = "package : " + packageName + "\n";
            str += "  " + className + " extends " + baseClassName + "\n";
            //str += "  hasBaseClass " + (baseClass == null ? "false":"true") + "\n";
            //str += "  inherited " + (inherited ? "true":"false") + "\n";


            return str;
        }
        */

        public PythonHelper(String packageName,String className)
        {
            this.packageName = packageName;
            this.className = className;
            imports = new HashSet<String>();

            dependencyName = new ArrayList<String>();
            dependencyHelper = new ArrayList<PythonHelper>();

            dependencyName.add("opsTypes.OPS_Object");
            dependencyHelper.add(null);
        }
        public void addDependency(String name)
        {
            System.out.println("Adding " + name + " to " + className);
            dependencyName.add(name);
            dependencyHelper.add(null);
        }
        public String getPackageName()
        {
            return packageName;
        }
        public void setClassDeclaration(String classDeclaration)
        {
            this.classDeclaration = classDeclaration;
        }
        public String getClassDeclaration()
        {
            return classDeclaration;
        }
        public void setBaseClassName(String baseClassName)
        {
            //this.baseClassName=baseClassName;
            dependencyName.set(0,baseClassName);
            inherited = true;

            System.out.println(this.className +" extends "+ baseClassName);
        }

        public boolean connect(PythonHelper other)
        {
            //if (inherited == false) return true;
            //System.out.println("connect : " + this.baseClassName);
            if (this.packageName.equals(other.packageName))
            {
                //System.out.println("  comparing " + this.baseClassName +" to " + other.className);
                for (int i=0;i<dependencyName.size();i++)
                {
                    if (dependencyHelper.get(i) != null) continue;
                    System.out.print("For " + className + "  comparing " + dependencyName.get(i) +" to " + other.className + ": ");
                    if (dependencyName.get(i).equals(other.className))
                    {
                        dependencyHelper.set(i,other);
                        System.out.println("success");
                        return true;
                    }
                    else
                    {
                        System.out.println("failed");
                    }
                }
/*
                if (this.baseClassName.equals(other.className))
                {
                    this.baseClass = other;
                    dependencies.add(other);
//                    System.out.println(other.className + " is base for " + this.className);
                    return true;
                }
                */
            }
            else
            {
                System.out.println("For " + className + "  comparing " + "XXX" +" to " + other.className);

            }
            return false;
        }
        public void setSaved(boolean value)
        {
            saved = value;
        }
        public boolean isSaved()
        {
            return saved;
        }
        public boolean canBeSaved()
        {
            //if ((baseClass!=null) && (baseClass.saved == false)) return false;
            //return true;

            for (PythonHelper h : dependencyHelper)
            {
                if (h==null) continue;
                if (h.saved == false) return false;
            }
            return true;

        }
        /*
        public boolean isParentInPackage()
        {
            if (baseClass==null)
                return false;

            return packageName.equals(baseClass.packageName);
        }
        */
        private void createImports()
        {
            if (inherited)
            {
                for (int i=0;i<dependencyHelper.size();i++)
                {
                    PythonHelper h = dependencyHelper.get(i);
                    if (h==null)
                    {
                        int splitIndex = dependencyName.get(i).lastIndexOf(".");
                        String packageStr = dependencyName.get(i).substring(0,splitIndex);
                        if (packageName.equals(packageStr)) continue;
                        String classStr   = dependencyName.get(i).substring(splitIndex+1);
                        packageStr.replace(".","_");
                        addImport(packageStr,classStr);
                    }
                    else
                    {
                        if (packageName.equals(h.packageName)) continue;

                        int splitIndex = dependencyName.get(i).lastIndexOf(".");
                        if (splitIndex <0)
                            System.out.println("incorrect split of " + dependencyName.get(i) + " for " + className);
                        String packageStr = dependencyName.get(i).substring(0,splitIndex);
                        String classStr   = dependencyName.get(i).substring(splitIndex+1);
                        packageStr.replace(".","_");
                        addImport(packageStr,classStr);
                    }
                }

/*

                if (isParentInPackage()) return;
                int splitIndex = baseClassName.lastIndexOf(".");
                if (splitIndex <0)
                    System.out.println("incorrect split of " + baseClassName + " for " + className);
                String packageStr = baseClassName.substring(0,splitIndex);
                String classStr   = baseClassName.substring(splitIndex+1);

                if (inherited)
                    packageStr.replace(".","_");

                addImport(packageStr,classStr);
                */
            }
            else
            {
                addImport("ops.opsTypes","OPS_Object");
            }
        }
        public void addImport(String packageStr,String classStr)
        {
            imports.add("from " + packageStr + " import " + classStr + endl());
        }
        public void getImports(HashSet<String> imports)
        {
            createImports();
            imports.addAll(this.imports);
        }

    }
    private class ClassesAndImports
    {
        public String classes="";
        public HashSet<String> imports;
        public ClassesAndImports()
        {
            imports = new HashSet<String>();
        }
        public void append(String str)
        {
            classes = classes+str;
        }
        public String createImport()
        {
            ArrayList<String> temp = new ArrayList<String>(imports);
            Collections.sort(temp);
            return StringJoin("", temp); }
    }





    //private HashMap<String,Vector<PythonHelper>> packagesMap;
    private ArrayList<PythonHelper> helpers;

    public String getName()
    {
        return "PythonCompiler";
    }

    public PythonCompiler(String projectName)
    {
        super(projectName);
        setTabString("\t");
        helpers = new ArrayList<PythonHelper>();
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        System.out.println("PYTHON START");
        System.out.println("");
        try
        {
            for (IDLClass iDLClass : idlClasses)
            {
                compileIDLClass(iDLClass);
            }
            connectHelpers();
            saveAll();

            compileTypeSupport(_idlClasses, _projectName);

        } catch (IOException iOException)
        {
            System.out.println("Generating Python failed with the following exception: " + iOException.getMessage());
        }
        System.out.println("");
        System.out.println("PYTHON END");
    }


    public void compileIDLClass(IDLClass idlClass)
    {
        try
        {
            if (idlClass.getType() == IDLClass.ENUM_TYPE)
            {
                compileEnum(idlClass);
            }
            else
            {
                compileDataClass(idlClass);
            }

            _idlClasses.add(idlClass);

        } catch (IOException ioe)
        {
            System.out.println("Failed to generate code for " + idlClass.getClassName());
            System.out.println("  Generating failed with the following exception: " + ioe.getMessage());
        }
    }

    public void compileDataClass(IDLClass idlClass) throws IOException
    {

        String packageName = idlClass.getPackageName();
        String className = idlClass.getClassName();

        PythonHelper helper = new PythonHelper(packageName,className);
        helpers.add(helper);

        String baseClassName = "OPS_Object";
        if (idlClass.getBaseClassName() != null)
        {
            baseClassName = idlClass.getBaseClassName();
            helper.setBaseClassName(baseClassName);

            int splitIndex = baseClassName.lastIndexOf(".");
            baseClassName  = baseClassName.substring(splitIndex+1);
        }

        for (IDLField field : idlClass.getFields())
        {
            if (field.isIdlType())
            {
                String typeName = field.getType();
                if (field.isArray())
                {
                    typeName = typeName.substring(0,typeName.length() - 2);
                }
                helper.addDependency(typeName);
            }
        }

        java.io.InputStream stream = findTemplateFile("pythonclasstemplate.tpl");
        setTemplateTextFromResource(stream);
        String templateText = getTemplateText();

//Replace regular expressions in the template file.
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, baseClassName);
        templateText = templateText.replace(DECLARATIONS_REGEX, getDeclarations(idlClass));
        templateText = templateText.replace(SERIALIZE_REGEX, getSerialize(idlClass));
        templateText = templateText.replace(VALIDATION_REGEX, getValidation(idlClass));



        helper.setClassDeclaration(templateText);
        checkForImports(helper,idlClass);
        //System.out.println(helper);
    }

    protected void compileEnum(IDLClass idlClass) throws IOException
    {
        String packageName = idlClass.getPackageName();
        String className = idlClass.getClassName();
        String baseClassName = "OPS_Object";

        PythonHelper helper = new PythonHelper(packageName,className);
        helpers.add(helper);

        java.io.InputStream stream = findTemplateFile("pythonenumtemplate.tpl");
        setTemplateTextFromResource(stream);
        String templateText = getTemplateText();


        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(PACKAGE_NAME_REGEX, packageName);
        templateText = templateText.replace(BASE_CLASS_NAME_REGEX, baseClassName);
        templateText = templateText.replace(DECLARATIONS_REGEX, getEnumDeclarations(idlClass));
        templateText = templateText.replace(VALIDATION_REGEX, "" + idlClass.getEnumNames().size());

        helper.setClassDeclaration(templateText);
        checkForImports(helper,idlClass);
    }

    public void connectHelpers()
    {
        for (int i=0;i<helpers.size();i++)
        {
            for (int j=0;j<helpers.size();j++)
            {
                helpers.get(i).connect(helpers.get(j));
            }
        }
    }

    public void saveAll() throws IOException
    {
        HashMap<String,ClassesAndImports> packages = new HashMap<String,ClassesAndImports>();

        int lastNumSaved = 0;
        int numSaved = 0;
        do
        {
            Iterator<PythonHelper> i = helpers.iterator();
            lastNumSaved = numSaved;
            numSaved = 0;
            while (i.hasNext())
            {
                PythonHelper helper = i.next();
                if (helper.isSaved()==false)
                {
                    if (helper.canBeSaved())
                    {
                        //helper.createBaseImport();

                        String packageName = helper.getPackageName();

                        if (packages.containsKey(packageName)==false)
                        {
                            ClassesAndImports cai = new ClassesAndImports();
                            packages.put(packageName,cai);
                        }

                        packages.get(packageName).append(helper.getClassDeclaration());
                        helper.getImports(packages.get(packageName).imports);
                        helper.setSaved(true);
                        numSaved++;
                    }
                }
                else
                {
                    numSaved++;
                }
            }
        } while (lastNumSaved != numSaved);

        java.io.InputStream stream = findTemplateFile("pythonpackagetemplate.tpl");
        setTemplateTextFromResource(stream);


        for(String key: packages.keySet())
        {
            setOutputFileName(_outputDir + File.separator + key.replace(".","_") + ".py");
            String templateText = getTemplateText();

            templateText = templateText.replace(CLASSES_REGEX, packages.get(key).classes);
            templateText = templateText.replace(IMPORTS_REGEX, packages.get(key).createImport());

            saveOutputText(templateText);
        }
    }

    private void checkForImports(PythonHelper helper,IDLClass idlClass)
    {
        String packageName = idlClass.getPackageName();
        for (IDLField field : idlClass.getFields())
        {
            if (field.isIdlType())
            {
                String typeName = field.getType();
                if (field.isArray())
                {
                    typeName = typeName.substring(0,typeName.length() - 2);
                }

                int splitIndex = typeName.lastIndexOf(".");
                if (splitIndex == -1) continue;

                String packageStr = typeName.substring(0,splitIndex);
                String classStr   = typeName.substring(splitIndex+1);

                if (packageStr.equals(packageName)) continue;


/*
                if (typeName.startsWith(packageName))
                {
                    typeName = typeName.substring(packageName.length());
                }
*/
                helper.addImport(packageStr,classStr);
            }
        }
    }


/*
    protected void compilePublisher(IDLClass idlClass) throws IOException
    {
        System.out.println("PythonCompiler::compilePublisher");
    }


    protected void compileSubscriber(IDLClass idlClass) throws IOException
    {
        System.out.println("PythonCompiler::compileSubscriber");
    }
*/

    protected void compileTypeSupport(Vector<IDLClass> idlClasses, String projectName) throws IOException
    {

        String className = projectName + "TypeFactory";
        setOutputFileName(_outputDir + File.separator + projectName + File.separator + className + ".py");

        java.io.InputStream stream = findTemplateFile("pythontypefactorytemplate.tpl");
        setTemplateTextFromResource(stream);

        //Get the template file as a String
        String templateText = getTemplateText();


        ArrayList<String> importString = new ArrayList<String>();
        ArrayList<String> createBodyText = new ArrayList<String>();

        for (IDLClass idlClass : idlClasses)
        {
            importString.add("from " + idlClass.getPackageName().replace(".","_") + " import " + idlClass.getClassName() + endl());
            createBodyText.add(tab(2)+"self.addType(\"" + idlClass.getPackageName() + "." + idlClass.getClassName() + "\"," + idlClass.getClassName() + ")" + endl());
        }

        Collections.sort(importString);
        Collections.sort(createBodyText);

        templateText = templateText.replace(IMPORTS_REGEX, StringJoin("", importString));
        templateText = templateText.replace(CLASS_NAME_REGEX, className);
        templateText = templateText.replace(CREATE_BODY_REGEX, StringJoin("", createBodyText));



        saveOutputText(templateText);
    }

    public void compileTopicConfig(Vector<TopicInfo> topics, String name, String packageString, String projectDirectory)
    {
        System.out.println("PythonCompiler::compileTopicConfig NOT IMPLEMENTED");
    }

    private String getEnumDeclarations(IDLClass idlClass) {
        String ret = "";
        for (int i = 0; i < idlClass.getEnumNames().size(); i++)
        {
            ret += tab(1) + idlClass.getEnumNames().get(i) + " = " + i + endl();

        }
        return ret;
    }

    protected String getDeclarations(IDLClass idlClass)
    {
        String packageName = idlClass.getPackageName() + ".";
        String ret = "";
        for (IDLField field : idlClass.getFields())
        {

            if (!field.getComment().equals(""))
            {
                String comment = field.getComment();
                int idx;
                while ((idx = comment.indexOf('\n')) >= 0)
                {
                    ret += tab(2) + "#" + comment.substring(0,idx).replace("/*", "").replace("*/", "") + endl();
                    comment = comment.substring(idx+1);
                }
                ret += tab(2) + "#" + comment.replace("/*", "").replace("*/", "") + endl();
            }
            if (field.isArray())
            {
                //ret += tab(1) + "" + getDeclareVector(field);
                //ret += "#### VECTOR ####" + endl();
                ret += tab(2) + "self." + field.getName() + " = []"+endl();
            }
            else
            {
                if (field.isIdlType())
                {
                    String typeName = field.getType();
                    if (typeName.startsWith(packageName))
                    {
                        typeName = typeName.substring(packageName.length());
                    }
                    ret += tab(2) + "self." + field.getName() + " = " + typeName + "()" + endl();
                }
                else
                {
                    ret += tab(2) + "self." + field.getName() + " = " +getTypeInitialization(field.getType()) +endl();
                }
            }

        }
        return ret;
    }
/*
    protected String getDeclareVector(IDLField field)
    {
        System.out.println("PythonCompiler::getDeclareVector");
        return "PythonCompiler::getDeclareVector";
    }

    protected String languageType(String s)
    {
        System.out.println("PythonCompiler::getSerialize");
        return "PythonCompiler::languageType";
    }
*/
    protected String getSerialize(IDLClass idlClass)
    {
        String packageName = idlClass.getPackageName() + ".";
        String ret = "";
        for (IDLField field : idlClass.getFields())
        {
            String seralizerString = "archiver.";
            if (field.isArray()==false)
            {
                seralizerString = "self." + field.getName() + " = " + seralizerString;
            }

            if (field.isIdlType())
            {
                String typeName = field.getType();
                seralizerString += "Ops";
                if (field.isArray())
                {
                    seralizerString +="Vector";
                    typeName = typeName.substring(0,typeName.length() - 2);
                }
                seralizerString += "(\"" + field.getName() + "\", self." + field.getName();

                {
                    int splitIndex = typeName.lastIndexOf(".");
                    typeName  = typeName.substring(splitIndex+1);
                }

                if (field.isAbstract()==false)
                {
                    seralizerString += ", " + typeName;
                }

                seralizerString += ")";
            }
            else
            {
                seralizerString += getArchiverCall(field) + "(\"" + field.getName() + "\", self." + field.getName() + ")";
            }
            ret += tab(2) + seralizerString + endl();
        }
        return ret;
    }

    protected String getValidation(IDLClass idlClass)
    {
        String ret = "";
        for (IDLField field : idlClass.getFields())
        {
                String fieldName = "self." + field.getName();
                String typeName = field.getType();

                //ret+=tab(2) + "print \"Checking " + fieldName + " for " + typeName + "\"" + endl();

                int tabs = 2;


                if (field.isIdlType())
                {
                    int splitIndex = typeName.lastIndexOf(".");
                    typeName   = typeName.substring(splitIndex+1);
                }
                if (field.isArray())
                {
                    typeName = typeName.substring(0,typeName.length() - 2);
                    ret += tab(tabs++) + "for x in self." + field.getName() + ":" + endl();
                    fieldName = "x";
                }
                if (field.isIdlType()==false)
                {
                    typeName = getValidationString(typeName);
                }
                ret += tab(tabs++) + "if not isinstance(" + fieldName + "," + typeName +"):"+endl();
                ret += tab(tabs--) + "raise ValueError()" + endl();
                if (field.isIdlType())
                {
                    ret += tab(tabs++) + fieldName + ".validate()"+endl();
                }
        }
        return ret;
    }
    protected String getValidationString(String type)
    {
        if (type.equals("byte") || type.equals("short") || type.equals("int"))
            return "int";
        if (type.equals("long"))
            return "(int,long)";
        if (type.equals("float") || type.equals("double"))
            return "(float,int,long)";
        if (type.equals("string"))
            return "str";
        if (type.equals("boolean"))
            return "bool";

        return "##### ERROR getValidationString("+type+")";
    }

    protected String getTypeInitialization(String s)
    {
        if (s.equals("string"))       return "\"\"";
        else if (s.equals("boolean")) return "False";
        else if (s.equals("short"))   return "0";
        else if (s.equals("int"))     return "0";
        else if (s.equals("long"))    return "0";
        else if (s.equals("double"))  return "0.0";
        else if (s.equals("float"))   return "0.0";
        else if (s.equals("byte"))    return "0";
        else                          return "##### ERROR (" + s + ")";
    }

    protected String getArchiverCall(IDLField field)
    {
        String s = field.getType();
        if (field.isArray())
        {
            return getArchiverCall(s.substring(0, s.length() - 2)) + "Vector";
        }
        else
        {
            return getArchiverCall(s);
        }
    }

    protected String getArchiverCall(String s)
    {
        if (s.equals("string"))       return "String";
        else if (s.equals("boolean")) return "Bool";
        else if (s.equals("short"))   return "Int16";
        else if (s.equals("int"))     return "Int32";
        else if (s.equals("long"))    return "Int64";
        else if (s.equals("double"))  return "Float64";
        else if (s.equals("float"))   return "Float32";
        else if (s.equals("byte"))    return "Int8";
        return "### ERROR";
    }

}
